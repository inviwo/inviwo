/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <modules/basegl/raycasting/sampletransformcomponent.h>
#include <modules/opengl/shader/shaderutils.h>

#include <fmt/format.h>

namespace inviwo {

SampleTransformComponent::SampleTransformComponent()
    : RaycasterComponent()
    , offset_{"sampleOffset", "Sample Shift", vec3{0.0f}, vec3{-1.0f}, vec3{1.0f}} {}

std::string SampleTransformComponent::getName() const { return offset_.getIdentifier(); }

void SampleTransformComponent::setUniforms(Shader &shader, TextureUnitContainer &) const {
    utilgl::setUniforms(shader, offset_);
}

std::vector<Property *> SampleTransformComponent::getProperties() { return {&offset_}; }

auto SampleTransformComponent::getSegments() const -> std::vector<Segment> {
    return {
        Segment{"entryPoint += sampleOffset;", Segment::pre, 400},
        Segment{fmt::format("uniform vec3 {};", offset_.getIdentifier()), Segment::uniform, 400}};
}

}  // namespace inviwo
