/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <modules/basegl/raycasting/volumecomponent.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

VolumeComponent::VolumeComponent(std::string_view name)
    : RaycasterComponent(), volumePort(std::string(name)) {}

std::string_view VolumeComponent::getName() const { return volumePort.getIdentifier(); }

void VolumeComponent::process(Shader& shader, TextureUnitContainer& cont) {
    utilgl::bindAndSetUniforms(shader, cont, volumePort);
}

std::vector<std::tuple<Inport*, std::string>> VolumeComponent::getInports() {
    return {{&volumePort, std::string{"volumes"}}};
}

auto VolumeComponent::getSegments() const -> std::vector<Segment> {
    constexpr std::string_view uniforms{
        "uniform VolumeParameters {0}Parameters;\n"
        "uniform sampler3D {0};"};

    constexpr std::string_view voxelFirst{
        "vec4 {0}VoxelPrev = vec4(0);\n"
        "vec4 {0}Voxel = getNormalizedVoxel({0}, {0}Parameters, samplePosition);"};

    constexpr std::string_view voxel{
        "{0}VoxelPrev = {0}Voxel;\n"
        "{0}Voxel = getNormalizedVoxel({0}, {0}Parameters, samplePosition);"};

    constexpr std::string_view gradientFirst{
        "vec3 {0}GradientPrev = vec3(0);\n"
        "vec3 {0}Gradient = normalize(\n"
        "    COMPUTE_GRADIENT_FOR_CHANNEL({0}Voxel, {0}, {0}Parameters, samplePosition, "
        "channel));"};

    constexpr std::string_view gradient{
        "{0}GradientPrev = {0}Gradient;\n"
        "{0}Gradient = normalize(\n"
        "    COMPUTE_GRADIENT_FOR_CHANNEL({0}Voxel, {0}, {0}Parameters, samplePosition, channel));"};

    std::vector<Segment> segments{
        Segment{fmt::format(uniforms, volumePort.getIdentifier()), Segment::uniform, 400},
        Segment{fmt::format(voxelFirst, volumePort.getIdentifier()), Segment::first, 400},
        Segment{fmt::format(voxel, volumePort.getIdentifier()), Segment::loop, 400}};

    if (calculateGradient) {
        segments.push_back(
            Segment{std::string("#include \"utils/gradients.glsl\""), Segment::include, 400});
        segments.push_back(
            Segment{fmt::format(gradientFirst, volumePort.getIdentifier()), Segment::first, 410});
        segments.push_back(
            Segment{fmt::format(gradient, volumePort.getIdentifier()), Segment::loop, 410});
    }

    return segments;
}

}  // namespace inviwo
