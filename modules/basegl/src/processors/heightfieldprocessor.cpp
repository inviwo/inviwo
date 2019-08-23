/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/datastructures/geometry/simplemeshcreator.h>
#include <inviwo/core/datastructures/light/directionallight.h>
#include <inviwo/core/interaction/trackball.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/rendering/meshdrawerfactory.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <inviwo/core/algorithm/boundingbox.h>

namespace inviwo {

const ProcessorInfo HeightFieldProcessor::processorInfo_{
    "org.inviwo.HeightFieldRenderGL",  // Class identifier
    "Height Field Renderer",           // Display name
    "Heightfield",                     // Category
    CodeState::Experimental,           // Code state
    Tags::GL,                          // Tags
};
const ProcessorInfo HeightFieldProcessor::getProcessorInfo() const { return processorInfo_; }

HeightFieldProcessor::HeightFieldProcessor()
    : Processor()
    , inport_("geometry")
    , inportHeightfield_("heightfield", true)
    , inportTexture_("texture", true)
    , inportNormalMap_("normalmap", true)
    , imageInport_("imageInport")
    , outport_("image")
    , heightScale_("heightScale", "Height Scale", 1.0f, 0.0f, 10.0f)
    , terrainShadingMode_(
          "terrainShadingMode", "Terrain Shading",
          {{"shadingConstant", "Constant Color", HeightFieldShading::ConstantColor},
           {"shadingColorTex", "Color Texture", HeightFieldShading::ColorTexture},
           {"shadingHeightField", "Heightfield Texture", HeightFieldShading::HeightField}},
          0)
    , camera_("camera", "Camera", util::boundingBox(inport_))
    , resetViewParams_("resetView", "Reset Camera")
    , trackball_(&camera_)
    , lightingProperty_("lighting", "Lighting", &camera_)
    , shader_("heightfield.vert", "heightfield.frag", false) {

    addPort(inport_);
    addPort(inportHeightfield_);
    addPort(inportTexture_);
    addPort(inportNormalMap_);
    addPort(imageInport_);
    addPort(outport_);

    inportHeightfield_.setOptional(true);
    inportTexture_.setOptional(true);
    inportNormalMap_.setOptional(true);
    imageInport_.setOptional(true);

    addProperty(heightScale_);
    addProperty(terrainShadingMode_);

    addProperty(camera_);
    resetViewParams_.onChange([this]() { camera_.resetCamera(); });
    addProperty(resetViewParams_);
    inport_.onChange([this]() { updateDrawers(); });

    addProperty(lightingProperty_);
    addProperty(trackball_);

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

HeightFieldProcessor::~HeightFieldProcessor() = default;

void HeightFieldProcessor::initializeResources() {
    // shading defines
    utilgl::addShaderDefines(shader_, lightingProperty_);
    shader_.build();
}

void HeightFieldProcessor::process() {
    if (imageInport_.isReady()) {
        utilgl::activateTargetAndCopySource(outport_, imageInport_);
    } else {
        utilgl::activateAndClearTarget(outport_);
    }

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

    bool normalMapping = inportNormalMap_.isReady();
    if (normalMapping) {
        utilgl::bindColorTexture(inportNormalMap_, normalTexUnit.getEnum());
    }

    shader_.setUniform("inportHeightfield", heightFieldUnit.getUnitNumber());
    shader_.setUniform("inportTexture", colorTexUnit.getUnitNumber());
    shader_.setUniform("inportNormalMap", normalTexUnit.getUnitNumber());
    shader_.setUniform("terrainShadingMode", terrainShadingMode);
    shader_.setUniform("normalMapping", (normalMapping ? 1 : 0));

    utilgl::setUniforms(shader_, camera_, lightingProperty_, heightScale_);
    for (auto& drawer : drawers_) {
        utilgl::setShaderUniforms(shader_, *(drawer.second->getMesh()), "geometry");
        drawer.second->draw();
    }

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
    TextureUnit::setZeroUnit();
}

void HeightFieldProcessor::updateDrawers() {
    auto changed = inport_.getChangedOutports();
    DrawerMap temp;
    std::swap(temp, drawers_);

    std::map<const Outport*, std::vector<std::shared_ptr<const Mesh>>> data;
    for (auto& elem : inport_.getSourceVectorData()) {
        data[elem.first].push_back(elem.second);
    }

    for (auto elem : data) {
        auto ibegin = temp.lower_bound(elem.first);
        auto iend = temp.upper_bound(elem.first);

        if (util::contains(changed, elem.first) || ibegin == temp.end() ||
            static_cast<long>(elem.second.size()) !=
                std::distance(ibegin, iend)) {  // data is changed or new.

            for (auto geo : elem.second) {
                auto factory = getNetwork()->getApplication()->getMeshDrawerFactory();
                if (auto renderer = factory->create(geo.get())) {
                    drawers_.emplace(std::make_pair(elem.first, std::move(renderer)));
                }
            }
        } else {  // reuse the old data.
            drawers_.insert(std::make_move_iterator(ibegin), std::make_move_iterator(iend));
        }
    }
}

}  // namespace inviwo
