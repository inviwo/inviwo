/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/postprocessing/processors/fog.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/sharedopenglresources.h>
#include <modules/opengl/geometry/meshgl.h>

namespace inviwo {

const ProcessorInfo Fog::processorInfo_{
    "org.inviwo.Fog",       // Class identifier
    "Fog",                  // Display name
    "Postprocessing",       // Category
    CodeState::Stable,      // Code state
    "GL, Image Operation",  // Tags
};
const ProcessorInfo Fog::getProcessorInfo() const { return processorInfo_; }

Fog::Fog()
    : input_("inport")
    , output_("output")
    , color_("color", "Color", vec3(1.f))
    , density_("density", "Density", 1.f, 0.f, 10.f)
    , range_("range", "Range", 0.0f, 1.0f, 0.0f, 1.0f)
    , camera_("camera", "Camera")
    , shader_("fullscreenquad.vert", "fog.frag") {
    addPort(input_);
    addPort(output_);
    addProperty(color_);
    addProperty(density_);
    addProperty(range_);
    addProperty(camera_);

    color_.setSemantics(PropertySemantics::Color);

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidOutput); });
}

void Fog::process() {
    utilgl::activateTargetAndCopySource(output_, input_, ImageType::ColorOnly);

    shader_.activate();
    shader_.setUniform("fogColor", color_);
    shader_.setUniform("fogDensity", density_);
    utilgl::setUniforms(shader_, camera_, range_);
    TextureUnitContainer cont;
    utilgl::bindAndSetUniforms(shader_, cont, input_, ImageType::ColorDepth);

    utilgl::singleDrawImagePlaneRect();

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
