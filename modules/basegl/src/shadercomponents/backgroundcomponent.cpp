/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2022 Inviwo Foundation
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

#include <modules/basegl/shadercomponents/backgroundcomponent.h>
#include <modules/opengl/texture/textureutils.h>
#include <inviwo/core/util/stringconversion.h>

#include <string_view>
#include <fmt/format.h>

namespace inviwo {

BackgroundComponent::BackgroundComponent(Processor& processor)
    : ShaderComponent(), background_("bg") {
    background_.setOptional(true);

    background_.onConnect([&]() { processor.invalidate(InvalidationLevel::InvalidResources); });
    background_.onDisconnect([&]() { processor.invalidate(InvalidationLevel::InvalidResources); });
}

std::string_view BackgroundComponent::getName() const { return background_.getIdentifier(); }

void BackgroundComponent::process(Shader& shader, TextureUnitContainer& cont) {
    if (background_.isReady()) {
        utilgl::bindAndSetUniforms(shader, cont, background_, ImageType::ColorDepthPicking);
    }
}

std::vector<std::tuple<Inport*, std::string>> BackgroundComponent::getInports() {
    // Should be in the same PortGroup ('images') as the ImageOutport of the processor using this
    // component (@see ShaderComponentProcessorBase)
    return {{&background_, std::string{"images"}}};
}

namespace {

constexpr std::string_view uniforms = util::trim(R"(
uniform ImageParameters {bg}Parameters;
uniform sampler2D {bg}Color;
uniform sampler2D {bg}Picking;
uniform sampler2D {bg}Depth;
)");

constexpr std::string_view setup = util::trim(R"(
vec4 {bg}ColorVal = texture({bg}Color, texCoords);
vec4 {bg}PickingVal = texture({bg}Picking, texCoords);
depth = texture({bg}Depth, texCoords).x;
// convert to raycasting depth
float {bg}RayDepth =
    rayLength * calculateTValueFromDepthValue(camera, depth, entryPointDepth, exitPointDepth);
if ({bg}RayDepth <= 0) {comp}
)");

constexpr std::string_view first = util::trim(R"(
if ({bg}RayDepth > 0 && {bg}RayDepth <= rayPosition) {comp}
)");

constexpr std::string_view loop = util::trim(R"(
if ({bg}RayDepth > rayPosition - rayStep  && {bg}RayDepth <= rayPosition) {comp}
)");

constexpr std::string_view post = util::trim(R"(
// composite background if lying beyond the last volume sample
if ({bg}RayDepth > rayLength - rayStep * 0.5) {comp}
)");

constexpr std::string_view composite = util::trim(R"(
{{
    if (rayDepth == -1.0 && {bg}ColorVal.a > 0.0) rayDepth = {bg}RayDepth;
    if (picking.a == 0.0 && {bg}PickingVal.a > 0.0) picking = {bg}PickingVal;
    result.rgb = result.rgb + (1.0 - result.a) * {bg}ColorVal.a * {bg}ColorVal.rgb;
    result.a = result.a + (1.0 - result.a) * {bg}ColorVal.a;
}}
)");

}  // namespace

auto BackgroundComponent::getSegments() -> std::vector<Segment> {
    using namespace fmt::literals;
    const auto comp = fmt::format(composite, "bg"_a = getName());
    if (background_.isConnected()) {
        return {{fmt::format(uniforms, "bg"_a = getName()), placeholder::uniform, 900},
                {fmt::format(setup, "bg"_a = getName(), "comp"_a = comp), placeholder::setup, 900},
                {fmt::format(first, "bg"_a = getName(), "comp"_a = comp), placeholder::first, 900},
                {fmt::format(loop, "bg"_a = getName(), "comp"_a = comp), placeholder::loop, 900},
                {fmt::format(post, "bg"_a = getName(), "comp"_a = comp), placeholder::post, 900}};
    } else {
        return {};
    }
}

}  // namespace inviwo
