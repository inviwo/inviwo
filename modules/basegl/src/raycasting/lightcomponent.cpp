/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2020 Inviwo Foundation
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

#include <modules/basegl/raycasting/lightcomponent.h>
#include <modules/opengl/shader/shaderutils.h>

#include <string_view>

namespace inviwo {

LightComponent::LightComponent(CameraProperty* camera)
    : RaycasterComponent(), lighting_("lighting", "Lighting", camera) {}
std::string LightComponent::getName() const { return lighting_.getIdentifier(); }

void LightComponent::setDefines(Shader& shader) const { utilgl::addDefines(shader, lighting_); }

std::vector<Property*> LightComponent::getProperties() { return {&lighting_}; }

auto LightComponent::getSegments() const -> std::vector<Segment> {
    static constexpr std::string_view pre{
        "gradient = normalize(COMPUTE_GRADIENT_FOR_CHANNEL(\n"
        "    voxel, volume, volumeParameters, entryPoint + 0.5 * tIncr * rayDirection, channel));"};

    static constexpr std::string_view loop{
        "gradient = normalize(\n"
        "    COMPUTE_GRADIENT_FOR_CHANNEL(voxel, volume, volumeParameters, samplePos, channel));"};

    std::vector<Segment> segments;

    segments.push_back(
        Segment{std::string("uniform LightParameters lighting;"), Segment::uniform, 500});

    if (lighting_.shadingMode_ != ShadingMode::None) {
        segments.push_back(
            Segment{std::string("#include \"utils/gradients.glsl\""), Segment::include, 500});
        segments.push_back(
            Segment{std::string("#include \"utils/shading.glsl\""), Segment::include, 500});
        segments.push_back(Segment{std::string(pre), Segment::pre, 500});
        segments.push_back(Segment{std::string(loop), Segment::loop, 500});
    }
    return segments;
}

void LightComponent::setUniforms(Shader& shader, TextureUnitContainer&) const {
    if (lighting_.shadingMode_ != ShadingMode::None) {
        utilgl::setUniforms(shader, lighting_);
    }
}
}  // namespace inviwo
