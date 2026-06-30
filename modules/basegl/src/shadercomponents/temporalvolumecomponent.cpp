/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/basegl/shadercomponents/temporalvolumecomponent.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/util/stringconversion.h>
#include <modules/basegl/shadercomponents/isocomponent.h>
#include <modules/basegl/shadercomponents/tfcomponent.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderobject.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/volume/volumeutils.h>

#include <algorithm>

#include <fmt/format.h>

namespace inviwo {

TemporalVolumeComponent::TemporalVolumeComponent(std::string_view name, Gradients gradients,
                                                 Document help)
    : ShaderComponent()
    , volumePort{name, std::move(help)}
    , gradients{gradients}
    , time{"time", "Time",
           util::ordinalCount(0.0, 1.0).set("The time to sample the temporal volume at"_help)}
    , interpolation{"interpolation",
                    "Interpolation",
                    "How to sample between adjacent frames"_help,
                    {{"nearest", "Nearest", Interpolation::Nearest},
                     {"linear", "Linear", Interpolation::Linear}},
                    1,
                    InvalidationLevel::InvalidResources}
    , prefetch{"prefetch", "Prefetch", "Schedule background loading of upcoming frames"_help, true}
    , prefetchAhead{"prefetchAhead", "Prefetch Ahead",
                    util::ordinalCount<size_t>(2u, 16u)
                        .set("Number of upcoming frames to prefetch"_help)}
    // No port is passed to the IsoTFProperty: the TF operates in normalized space and the data
    // map is supplied manually in process() from the temporal volume prototype.
    , isoTF_{"isotf", "TF & Iso Values"} {}

std::string_view TemporalVolumeComponent::getName() const { return volumePort.getIdentifier(); }

IsoTFProperty& TemporalVolumeComponent::isoTF() { return isoTF_; }

void TemporalVolumeComponent::initializeResources(Shader& shader) {
    // need to ensure there is always at least one isovalue due to the use of the macro
    // as array size in IsovalueParameters
    const size_t isoCount = std::max<size_t>(1, isoTF_.isovalues_.get().size());
    shader.getFragmentShaderObject()->addShaderDefine("MAX_ISOVALUE_COUNT",
                                                      fmt::format("{}", isoCount));
}

void TemporalVolumeComponent::process(Shader& shader, TextureUnitContainer& cont) {
    auto temporal = volumePort.getData();
    if (!temporal || temporal->empty()) return;

    const double t = time.get();

    std::shared_ptr<const Volume> a;
    std::shared_ptr<const Volume> b;
    float blend = 0.0f;

    if (interpolation.get() == Interpolation::Linear) {
        auto frame = temporal->interpolate(t);
        a = frame.a;
        b = frame.b ? frame.b : frame.a;
        blend = static_cast<float>(frame.t);
    } else {
        a = temporal->get(t);
        b = a;
    }
    if (!a) return;
    if (!b) b = a;

    utilgl::bindAndSetUniforms(shader, cont, *a, getName());
    utilgl::bindAndSetUniforms(shader, cont, *b, fmt::format("{}B", getName()));
    shader.setUniform(fmt::format("{}Blend", getName()), blend);

    // Transfer function and isovalues
    utilgl::bindAndSetUniforms(shader, cont, isoTF_.tf_);
    detail::setUniforms(shader, isoTF_.tf_);
    detail::setUniforms(shader, isoTF_.isovalues_, &temporal->prototype().dataMap);

    if (prefetch.get()) {
        const size_t current = temporal->nearestIndex(t);
        temporal->prefetch(current + 1, prefetchAhead.get());
    }
}

std::vector<std::tuple<Inport*, std::string>> TemporalVolumeComponent::getInports() {
    return {{&volumePort, std::string{"volumes"}}};
}

std::vector<Property*> TemporalVolumeComponent::getProperties() {
    return {&time, &interpolation, &prefetch, &prefetchAhead, &isoTF_};
}

namespace {

constexpr std::string_view uniforms = util::trim(R"(
uniform VolumeParameters {0}Parameters;
uniform sampler3D {0};
uniform VolumeParameters {0}BParameters;
uniform sampler3D {0}B;
uniform float {0}Blend = 0.0;
)");

constexpr std::string_view sampleFirst = util::trim(R"(
vec3 {0}SamplePosition = samplePosition;
)");

// Initialize the VoxelPrev value to the same as the first voxel value, this prevents isosurfaces
// being rendered at the volume boundaries.
constexpr std::string_view voxelFirst = util::trim(R"(
vec4 {0}Voxel = mix(getNormalizedVoxel({0}, {0}Parameters, {0}SamplePosition),
                    getNormalizedVoxel({0}B, {0}BParameters, {0}SamplePosition), {0}Blend);
vec4 {0}VoxelPrev = {0}Voxel;
)");

constexpr std::string_view sample = util::trim(R"(
{0}SamplePosition = samplePosition;
)");

constexpr std::string_view voxel = util::trim(R"(
{0}VoxelPrev = {0}Voxel;
{0}Voxel = mix(getNormalizedVoxel({0}, {0}Parameters, {0}SamplePosition),
               getNormalizedVoxel({0}B, {0}BParameters, {0}SamplePosition), {0}Blend);
)");

constexpr std::string_view gradientFirst = util::trim(R"(
vec3 {0}GradientPrev = vec3(0);
vec3 {0}Gradient = vec3(0);
#if defined(GRADIENTS_ENABLED)
{0}Gradient = useSurfaceNormals ? -texture(surfaceNormal, texCoords).xyz :
    normalize(mix(COMPUTE_GRADIENT_FOR_CHANNEL({0}Voxel, {0}, {0}Parameters,
                                               {0}SamplePosition, channel),
                  COMPUTE_GRADIENT_FOR_CHANNEL({0}Voxel, {0}B, {0}BParameters,
                                               {0}SamplePosition, channel), {0}Blend));
if (!useSurfaceNormals) {{
    {0}Gradient *= sign({0}Voxel[channel] / {0}Parameters.texToNormalized.scale + {0}Parameters.texToNormalized.offset);
}}
#endif
)");

constexpr std::string_view gradient = util::trim(R"(
#if defined(GRADIENTS_ENABLED)
{0}GradientPrev = {0}Gradient;
{0}Gradient = normalize(mix(COMPUTE_GRADIENT_FOR_CHANNEL({0}Voxel, {0}, {0}Parameters,
                                                         {0}SamplePosition, channel),
                            COMPUTE_GRADIENT_FOR_CHANNEL({0}Voxel, {0}B, {0}BParameters,
                                                         {0}SamplePosition, channel), {0}Blend));
{0}Gradient *= sign({0}Voxel[channel] / {0}Parameters.texToNormalized.scale + {0}Parameters.texToNormalized.offset);
#endif
)");

}  // namespace

auto TemporalVolumeComponent::getSegments() -> std::vector<Segment> {
    std::vector<Segment> segments{{.snippet = fmt::format(uniforms, getName()),
                                   .placeholder = placeholder::uniform,
                                   .priority = 400},
                                  {.snippet = fmt::format(sampleFirst, getName()),
                                   .placeholder = placeholder::first,
                                   .priority = 400},
                                  {.snippet = fmt::format(voxelFirst, getName()),
                                   .placeholder = placeholder::first,
                                   .priority = 420},
                                  {.snippet = fmt::format(sample, getName()),
                                   .placeholder = placeholder::loop,
                                   .priority = 400},
                                  {.snippet = fmt::format(voxel, getName()),
                                   .placeholder = placeholder::loop,
                                   .priority = 420}};

    if (gradients != Gradients::None) {
        segments.emplace_back(std::string{R"(#include "utils/gradients.glsl")"},
                              placeholder::include, 400);
        segments.emplace_back(fmt::format(gradientFirst, getName()), placeholder::first, 440);
        segments.emplace_back(fmt::format(gradient, getName()), placeholder::loop, 440);
    }

    detail::addSegmentsFor(segments, isoTF_.tf_);
    detail::addSegmentsFor(segments, isoTF_.isovalues_);

    return segments;
}

std::string TemporalVolumeComponent::getGradientString() const {
    return fmt::format("{0}Gradient", getName());
}

std::optional<size_t> TemporalVolumeComponent::channelsForVolume() const {
    if (auto data = volumePort.getData(); data && !data->empty()) {
        return data->prototype().getDataFormat()->getComponents();
    }
    return std::nullopt;
}

}  // namespace inviwo
