/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <modules/basegl/processors/raycasting/atlasisosurfacerenderer.h>

#include <inviwo/core/io/serialization/serialization.h>
#include <inviwo/core/io/serialization/versionconverter.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumeutils.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/algorithm/boundingbox.h>

#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/datastructures/volume/volume.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo AtlasIsosurfaceRenderer::processorInfo_{
    "org.inviwo.AtlasIsosurfaceRenderer",  // Class identifier
    "Atlas Isosurface Renderer",           // Display name
    "Undefined",                           // Category
    CodeState::Experimental,               // Code state
    Tags::None,                            // Tags
};
const ProcessorInfo AtlasIsosurfaceRenderer::getProcessorInfo() const { return processorInfo_; }

AtlasIsosurfaceRenderer::AtlasIsosurfaceRenderer()
    : Processor()
    , shader_("raycasting/atlasisosurfacerenderer.frag", Shader::Build::Yes)
    , volumeInport_("volume")
    , entryPort_("entry")
    , exitPort_("exit")
    , outport_("outport")
    , showHighlighted_("showHighlightedIndices", "Show Highlighted", true,
                       vec3(1.0f, 0.906f, 0.612f))
    , showSelected_("showSelectedIndices", "Show Selected", true, vec3(1.0f, 0.769f, 0.247f))
    , showFiltered_("showFilteredIndices", "Show Filtered", true, vec3(0.5f, 0.5f, 0.5f))
    , sampleRate_{"sampleRate", "Sampling Rate", 2.0f,
                  std::pair{1.0f, ConstraintBehavior::Immutable},
                  std::pair{25.0f, ConstraintBehavior::Editable}}
    , volumeSwitch_{"switch", "Switch volumes", false} {

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    addPort(volumeInport_);
    addPort(entryPort_);
    addPort(exitPort_);
    addPort(outport_);
    addProperties(sampleRate_);
    addProperties(showFiltered_);
    addProperties(showSelected_);
    addProperties(showHighlighted_);
    addProperties(volumeSwitch_);

    auto updateSampleRate = [this]() { sampleRateValue_ = sampleRate_.get(); };
}

void AtlasIsosurfaceRenderer::process() {
    if (volumeSwitch_.get()) {
        std::array<uint16_t, 8> sampledata({2, 32767, 1, 2, 3, 1, 2, 3});
        auto volumeram = std::make_shared<VolumeRAMPrecision<uint16_t>>(size3_t{2, 2, 2});
        std::copy(sampledata.begin(), sampledata.end(), volumeram->getDataTyped());
        auto volume = Volume(volumeram);
        volume.setSwizzleMask(swizzlemasks::luminance);
        volume.setInterpolation(InterpolationType::Nearest);
        raycast(volume);
    } else {
        raycast(*volumeInport_.getData());
    }
}

void AtlasIsosurfaceRenderer::raycast(const Volume& volume) {
    if (!volume.getRep<kind::GL>()) {
        throw Exception("Couldn't find VolumeGL representation", IVW_CONTEXT);
    }
    utilgl::activateAndClearTarget(outport_);
    shader_.activate();

    TextureUnitContainer units;

    utilgl::bindAndSetUniforms(shader_, units, volume, "volume");
    GLuint v;
    glGetTexParameterIuiv(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, &v);
    LogInfo(v << " " << GL_NEAREST);
    utilgl::bindAndSetUniforms(shader_, units, entryPort_, ImageType::ColorDepthPicking);
    utilgl::bindAndSetUniforms(shader_, units, exitPort_, ImageType::ColorDepth);

    float scaling = std::powf(2, static_cast<float>(volume.getDataFormat()->getPrecision()))-1.0f;
    shader_.setUniform("scaling", scaling);
    shader_.setUniform("sampleRate", sampleRate_.get());
    shader_.setUniform("selectedColor", showSelected_.getColor());
    shader_.setUniform("showSelected", showSelected_.getBoolProperty()->get());
    shader_.setUniform("highlightedColor", showHighlighted_.getColor());
    shader_.setUniform("showHighlighted", showHighlighted_.getBoolProperty()->get());
    shader_.setUniform("filteredColor", showFiltered_.getColor());
    shader_.setUniform("showFiltered", showFiltered_.getBoolProperty()->get());
    utilgl::setUniforms(shader_, outport_);
    utilgl::singleDrawImagePlaneRect();
    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
