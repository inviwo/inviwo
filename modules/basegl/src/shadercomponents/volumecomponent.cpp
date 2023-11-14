/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2023 Inviwo Foundation
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

#include <modules/basegl/shadercomponents/volumecomponent.h>

#include <inviwo/core/ports/volumeport.h>                     // for VolumeInport
#include <inviwo/core/util/stringconversion.h>                // for trim
#include <modules/basegl/shadercomponents/shadercomponent.h>  // for ShaderComponent::Segment
#include <modules/opengl/volume/volumeutils.h>                // for bindAndSetUniforms
#include <modules/opengl/shader/shader.h>

#include <fmt/core.h>    // for format
#include <fmt/format.h>  // for compile_string_to_view, FMT...

namespace inviwo {
class Inport;
class Shader;
class TextureUnitContainer;

VolumeComponent::VolumeComponent(std::string_view name, Gradients gradients, Document help)
    : ShaderComponent(), volumePort{name, std::move(help)}, gradients{gradients} {}

std::string_view VolumeComponent::getName() const { return volumePort.getIdentifier(); }

void VolumeComponent::process(Shader& shader, TextureUnitContainer& cont) {
    utilgl::bindAndSetUniforms(shader, cont, volumePort);
}

std::vector<std::tuple<Inport*, std::string>> VolumeComponent::getInports() {
    return {{&volumePort, std::string{"volumes"}}};
}

namespace {

constexpr std::string_view uniforms = util::trim(R"(
uniform VolumeParameters {0}Parameters;
uniform sampler3D {0};
)");

// Initialize the VoxelPrev value to the same as the first voxel value. This value is important
// mainly for the isosurface rendering. Setting it to the same voxel value prevents isosurfaces
// being rendered at the volume boundaries.
constexpr std::string_view voxelFirst = util::trim(R"(
vec4 {0}Voxel = getNormalizedVoxel({0}, {0}Parameters, samplePosition);
vec4 {0}VoxelPrev = {0}Voxel;
)");

constexpr std::string_view voxel = util::trim(R"(
{0}VoxelPrev = {0}Voxel;
{0}Voxel = getNormalizedVoxel({0}, {0}Parameters, samplePosition);
)");

constexpr std::string_view gradientFirst = util::trim(R"(
#if defined(GRADIENTS_ENABLED)
vec3 {0}GradientPrev = vec3(0);
vec3 {0}Gradient = useSurfaceNormals ? -texture(surfaceNormal, texCoords).xyz :
    normalize(COMPUTE_GRADIENT_FOR_CHANNEL({0}Voxel, {0}, {0}Parameters,
                                           samplePosition, channel));
if (!useSurfaceNormals) {{
    {0}Gradient *= sign({0}Voxel[channel] / (1.0 - {0}Parameters.formatScaling) - {0}Parameters.formatOffset);
}}
#endif
)");

constexpr std::string_view gradient = util::trim(R"(
#if defined(GRADIENTS_ENABLED)
{0}GradientPrev = {0}Gradient;
{0}Gradient = normalize(COMPUTE_GRADIENT_FOR_CHANNEL({0}Voxel, {0}, {0}Parameters,
                                                     samplePosition, channel));
{0}Gradient *= sign({0}Voxel[channel] / (1.0 - {0}Parameters.formatScaling) - {0}Parameters.formatOffset);
#endif
)");

constexpr std::string_view allGradientsFirst = util::trim(R"(
#if defined(GRADIENTS_ENABLED)
mat4x3 {0}AllGradientsPrev = mat4x3(0);
vec3 surfaceNormal = useSurfaceNormals ? -texture(surfaceNormal, texCoords).xyz : vec3(0);
mat4x3 {0}AllGradients = useSurfaceNormals ?
    mat4x3(surfaceNormal, surfaceNormal, surfaceNormal, surfaceNormal) :
    COMPUTE_ALL_GRADIENTS({0}Voxel, {0}, {0}Parameters, samplePosition);
{0}AllGradients[0] = normalize({0}AllGradients[0]);
{0}AllGradients[1] = normalize({0}AllGradients[1]);
{0}AllGradients[2] = normalize({0}AllGradients[2]);
{0}AllGradients[3] = normalize({0}AllGradients[3]);
#endif
)");

constexpr std::string_view allGradients = util::trim(R"(
#if defined(GRADIENTS_ENABLED)
{0}AllGradientsPrev = {0}AllGradients;
{0}AllGradients = COMPUTE_ALL_GRADIENTS({0}Voxel, {0}, {0}Parameters, samplePosition);
{0}AllGradients[0] = normalize({0}AllGradients[0]);
{0}AllGradients[1] = normalize({0}AllGradients[1]);
{0}AllGradients[2] = normalize({0}AllGradients[2]);
{0}AllGradients[3] = normalize({0}AllGradients[3]);
#endif
)");

}  // namespace

auto VolumeComponent::getSegments() -> std::vector<Segment> {

    std::vector<Segment> segments{
        {fmt::format(FMT_STRING(uniforms), getName()), placeholder::uniform, 400},
        {fmt::format(FMT_STRING(voxelFirst), getName()), placeholder::first, 400},
        {fmt::format(FMT_STRING(voxel), getName()), placeholder::loop, 400}};

    if (gradients != Gradients::None) {
        segments.push_back(
            Segment{std::string{R"(#include "utils/gradients.glsl")"}, placeholder::include, 400});
    }
    if (gradients == Gradients::Single) {
        segments.push_back(
            Segment{fmt::format(FMT_STRING(gradientFirst), getName()), placeholder::first, 410});
        segments.push_back(
            Segment{fmt::format(FMT_STRING(gradient), getName()), placeholder::loop, 410});
    }

    if (gradients == Gradients::All) {
        segments.push_back(Segment{fmt::format(FMT_STRING(allGradientsFirst), getName()),
                                   placeholder::first, 410});
        segments.push_back(
            Segment{fmt::format(FMT_STRING(allGradients), getName()), placeholder::loop, 410});
    }

    return segments;
}

}  // namespace inviwo
