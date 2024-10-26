/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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

#include <modules/basegl/processors/heightfieldprocessor.h>

#include <inviwo/core/algorithm/boundingbox.h>         // for boundingBox
#include <inviwo/core/algorithm/markdown.h>            // for operator""_help, operator""_uninde...
#include <inviwo/core/datastructures/geometry/mesh.h>  // for Mesh
#include <inviwo/core/ports/imageport.h>               // for ImageInport, BaseImageInport, Outp...
#include <inviwo/core/ports/inportiterable.h>          // for InportIterable<>::const_iterator
#include <inviwo/core/ports/meshport.h>                // for MeshFlatMultiInport
#include <inviwo/core/processors/processor.h>          // for Processor
#include <inviwo/core/processors/processorinfo.h>      // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>     // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>      // for Tags, Tags::GL
#include <inviwo/core/properties/cameraproperty.h>     // for CameraProperty
#include <inviwo/core/properties/invalidationlevel.h>  // for InvalidationLevel, InvalidationLev...
#include <inviwo/core/properties/optionproperty.h>     // for OptionPropertyOption, OptionProper...
#include <inviwo/core/properties/ordinalproperty.h>    // for ordinalLength, OrdinalPropertyState
#include <modules/opengl/geometry/meshgl.h>            // for MeshGL
#include <modules/opengl/rendering/meshdrawergl.h>     // for MeshDrawerGL::DrawObject, MeshDraw...
#include <modules/opengl/shader/shader.h>              // for Shader, Shader::Build
#include <modules/opengl/shader/shaderutils.h>         // for addShaderDefines, setShaderUniforms
#include <modules/opengl/texture/textureunit.h>        // for TextureUnit
#include <modules/opengl/texture/textureutils.h>       // for bindColorTexture, activateTargetAn...

#include <functional>   // for __base
#include <memory>       // for shared_ptr, shared_ptr<>::element_...
#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t

namespace inviwo {

const ProcessorInfo HeightFieldProcessor::processorInfo_{
    "org.inviwo.HeightFieldRenderGL",  // Class identifier
    "Height Field Renderer",           // Display name
    "Heightfield",                     // Category
    CodeState::Stable,                 // Code state
    Tags::GL,                          // Tags
    R"(
        Maps a height field onto a geometry and renders it to an image.
        
        ![](file:~modulePath~/docs/images/heightfield-network.png)
        
        Example Network:
        [core/heightfield.inv](file:~basePath~/data/workspaces/heightfield.inv)
    )"_unindentHelp};

const ProcessorInfo& HeightFieldProcessor::getProcessorInfo() const { return processorInfo_; }

HeightFieldProcessor::HeightFieldProcessor()
    : Processor()
    , inport_{"geometry", "Input geometry which is modified by the height field"_help}
    , inportHeightfield_{"heightfield", R"(
        The height field input (single-channel image).
        If the image has multiple channels only the red channel is used.)"_unindentHelp,
                         OutportDeterminesSize::Yes}
    , inportTexture_{"texture", "Color texture for color mapping (optional)."_help,
                     OutportDeterminesSize::Yes}
    , inportNormalMap_{"normalmap", "Normal map input (optional)"_help, OutportDeterminesSize::Yes}
    , imageInport_{"imageInport", "Background image (optional)"_help}
    , outport_{"image", "The rendered height field."_help}
    , heightScale_{"heightScale", "Height Scale",
                   util::ordinalLength(1.0f, 10.0f).set("Scaling factor for the height field"_help)}
    , terrainShadingMode_(
          "terrainShadingMode", "Terrain Shading",
          "Defines the color mapped onto the height field using either constant color, color input texture, or the height field texture"_help,
          {{"shadingConstant", "Constant Color", HeightFieldShading::ConstantColor},
           {"shadingColorTex", "Color Texture", HeightFieldShading::ColorTexture},
           {"shadingHeightField", "Heightfield Texture", HeightFieldShading::HeightField}},
          0)
    , camera_("camera", "Camera", util::boundingBox(inport_))
    , trackball_(&camera_)
    , lightingProperty_("lighting", "Lighting", &camera_)
    , shader_("heightfield.vert", "heightfield.frag", Shader::Build::No) {
    addPort(inport_);
    addPort(inportHeightfield_).setOptional(true);
    addPort(inportTexture_).setOptional(true);
    addPort(inportNormalMap_).setOptional(true);
    addPort(imageInport_).setOptional(true);
    addPort(outport_);

    addProperties(heightScale_, terrainShadingMode_, camera_, lightingProperty_, trackball_);

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

HeightFieldProcessor::~HeightFieldProcessor() = default;

void HeightFieldProcessor::initializeResources() {
    utilgl::addShaderDefines(shader_, lightingProperty_);
    shader_.build();
}

void HeightFieldProcessor::process() {
    utilgl::activateTargetAndClearOrCopySource(outport_, imageInport_);

    shader_.activate();

    // bind input textures
    TextureUnit heightFieldUnit, colorTexUnit, normalTexUnit;

    int terrainShadingMode = terrainShadingMode_.get();
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

    const bool normalMapping = inportNormalMap_.isReady();
    if (normalMapping) {
        utilgl::bindColorTexture(inportNormalMap_, normalTexUnit.getEnum());
    }

    shader_.setUniform("inportHeightfield", heightFieldUnit.getUnitNumber());
    shader_.setUniform("inportTexture", colorTexUnit.getUnitNumber());
    shader_.setUniform("inportNormalMap", normalTexUnit.getUnitNumber());
    shader_.setUniform("terrainShadingMode", terrainShadingMode);
    shader_.setUniform("normalMapping", (normalMapping ? 1 : 0));

    utilgl::setUniforms(shader_, camera_, lightingProperty_, heightScale_);

    for (auto mesh : inport_) {
        utilgl::setShaderUniforms(shader_, *mesh, "geometry");
        MeshDrawerGL::DrawObject drawer{mesh->getRepresentation<MeshGL>(),
                                        mesh->getDefaultMeshInfo()};
        drawer.draw();
    }

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
    TextureUnit::setZeroUnit();
}

}  // namespace inviwo
