/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 *********************************************************************************/

#include "heightfieldprocessor.h"
#include <inviwo/core/datastructures/geometry/simplemeshcreator.h>
#include <inviwo/core/datastructures/light/directionallight.h>
#include <inviwo/core/util/glmstreamoperators.h>
#include <modules/opengl/glwrap/textureunit.h>
#include <modules/opengl/textureutils.h>

namespace inviwo {

ProcessorClassIdentifier(HeightFieldProcessor, "org.inviwo.HeightFieldRenderGL");
ProcessorDisplayName(HeightFieldProcessor,  "Height Field Renderer");
ProcessorTags(HeightFieldProcessor, Tags::GL); 
ProcessorCategory(HeightFieldProcessor, "Heightfield");
ProcessorCodeState(HeightFieldProcessor, CODE_STATE_EXPERIMENTAL); 

HeightFieldProcessor::HeightFieldProcessor()
    : GeometryRenderProcessorGL()
    , inportHeightfield_("heightfield.inport", true)
    , inportTexture_("texture.inport", true)
    , inportNormalMap_("normalmap.inport", true)
    , heightScale_("heightScale", "Height Scale", 1.0f, 0.0f, 10.0f)
    , terrainShadingMode_("terrainShadingMode", "Terrain Shading")
{
    inportHeightfield_.onChange(this, &HeightFieldProcessor::heightfieldChanged);
    addPort(inportHeightfield_);
    addPort(inportTexture_);
    addPort(inportNormalMap_);
    
    addProperty(heightScale_);

    terrainShadingMode_.addOption("shadingConstant", "Constant Color", HeightFieldShading::ConstantColor);
    terrainShadingMode_.addOption("shadingColorTex", "Color Texture", HeightFieldShading::ColorTexture);
    terrainShadingMode_.addOption("shadingHeightField", "Heightfield Texture", HeightFieldShading::HeightField);
    terrainShadingMode_.set(HeightFieldShading::ConstantColor);
    terrainShadingMode_.setCurrentStateAsDefault();
    addProperty(terrainShadingMode_);
}

HeightFieldProcessor::~HeightFieldProcessor() {
}

void HeightFieldProcessor::initialize() {
    Processor::initialize();

    // initialize shader to offset vertices in the vertex shader
    shader_ = new Shader("heightfield.vert", "heightfield.frag", false);

    GeometryRenderProcessorGL::initializeResources();
}

void HeightFieldProcessor::process() {
    int terrainShadingMode = terrainShadingMode_.get();

    shader_->activate();

    // bind input textures
    TextureUnit heightFieldUnit, colorTexUnit, normalTexUnit;

    if (inportHeightfield_.isReady()) {
        utilgl::bindColorTexture(inportHeightfield_, heightFieldUnit.getEnum());
    } else if (terrainShadingMode == HeightFieldShading::HeightField) {
        // switch to flat shading since color texture is not available
        terrainShadingMode = HeightFieldShading::ConstantColor;
    }

    if (inportTexture_.isReady()) {
        utilgl::bindColorTexture(inportTexture_, colorTexUnit.getEnum());
    } else if (terrainShadingMode == HeightFieldShading::ColorTexture) {
        // switch to flat shading since heightfield texture is not available
        terrainShadingMode = HeightFieldShading::ConstantColor;
    }

    bool normalMapping = inportNormalMap_.isReady();
    if (normalMapping) {
        utilgl::bindColorTexture(inportNormalMap_, normalTexUnit.getEnum());
    }

    shader_->setUniform("inportHeightfield_", heightFieldUnit.getUnitNumber());
    shader_->setUniform("inportTexture_", colorTexUnit.getUnitNumber());
    shader_->setUniform("inportNormalMap_", normalTexUnit.getUnitNumber());
    shader_->setUniform("terrainShadingMode_", terrainShadingMode);
    shader_->setUniform("normalMapping_", (normalMapping ? 1 : 0));
    shader_->setUniform("heightScale_", heightScale_.get());

    // render mesh
    GeometryRenderProcessorGL::process();
    TextureUnit::setZeroUnit();
}

void HeightFieldProcessor::heightfieldChanged() {
    if (!inportHeightfield_.isConnected())
        return;

    //std::ostringstream str;
    //const Image *img = inportHeightfield_.getData();
    //const DataFormatBase* format = img->getDataFormat();

    //str << "Heightfield Port Properties:"
    //    << "\ndim: " << glm::to_string(img->getDimensions())
    //    << "\nType: " << img->getImageType()
    //    << "\nNum Color Layers: " << img->getNumberOfColorLayers()
    //    << std::endl << std::endl
    //    << "Format:"
    //    << "\nName: " << format->getString()
    //    << "\nComponents: " << format->getComponents()
    //    ;

    //LogInfo(str.str());
}

} // namespace
