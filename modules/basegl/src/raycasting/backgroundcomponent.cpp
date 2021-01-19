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

#include <modules/basegl/raycasting/backgroundcomponent.h>
#include <modules/opengl/texture/textureutils.h>

#include <string_view>
#include <fmt/format.h>

namespace inviwo {

BackgroundComponent::BackgroundComponent(Processor& processor)
    : RaycasterComponent(), background_("bg") {
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
    return {{&background_, std::string{"images"}}};
}

auto BackgroundComponent::getSegments() const -> std::vector<Segment> {
    static constexpr std::string_view uniforms{
        "uniform ImageParameters {0}Parameters;\n"
        "uniform sampler2D {0}Color;\n"
        "uniform sampler2D {0}Picking;\n"
        "uniform sampler2D {0}Depth;\n"};

    static constexpr std::string_view setup{
        "vec4 {0} = texture({0}Color, texCoords);\n"
        "depth = texture({0}Depth, texCoords).x;\n"
        "picking = texture({0}Picking, texCoords);\n"
        "// convert to raycasting depth\n"
        "float {0}RayDepth = rayLength * calculateTValueFromDepthValue(camera, depth, "
        "entryPointDepth, exitPointDepth);\n"
        "if ({0}RayDepth < 0) {{\n"
        "    result = {0};\n"
        "}}\n"};

    static constexpr std::string_view loop{
        "result = drawBackground(result, rayPosition, rayStep, {0}, {0}RayDepth, rayDepth);"};

    static constexpr std::string_view post{
        "// composite background if lying beyond the last volume sample\n"
        "if ({0}RayDepth > rayLength - rayStep * 0.5) {{\n"
        "    result = drawBackground(result, {0}RayDepth, rayStep * 0.5, {0}, {0}RayDepth, "
        "rayDepth);\n"
        "}}\n"};

    if (background_.isConnected()) {
        return {Segment{"#include \"utils/raycastgeometry.glsl\"", Segment::include, 900},
                Segment{fmt::format(uniforms, background_.getIdentifier()), Segment::uniform, 900},
                Segment{fmt::format(setup, background_.getIdentifier()), Segment::setup, 900},
                Segment{fmt::format(loop, background_.getIdentifier()), Segment::first, 900},
                Segment{fmt::format(post, background_.getIdentifier()), Segment::post, 900}};
    } else {
        return {};
    }
}

}  // namespace inviwo
