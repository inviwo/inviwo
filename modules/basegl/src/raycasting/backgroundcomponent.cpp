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

std::string BackgroundComponent::getName() const { return "background"; }

void BackgroundComponent::setUniforms(Shader& shader, TextureUnitContainer& cont) const {
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

    static constexpr std::string_view main{
        "color = texture(bgColor, texCoords);\n"
        "gl_FragDepth = backgroundDepth = texture({0}Depth, texCoords).x;\n"
        "PickingData = texture({0}Picking, texCoords);\n"};

    static constexpr std::string_view pre{
        "// convert to raycasting depth\n"
        "float bgTDepth = tEnd * calculateTValueFromDepthValue(camera, backgroundDepth, "
        "entryPointDepth, exitPointDepth);\n"
        "if (bgTDepth < 0) {\n"
        "    result = backgroundColor;\n"
        "}\n"};

    static constexpr std::string_view loop{
        "result = drawBackground(result, t, tIncr, backgroundColor, bgTDepth, tDepth);"};

    static constexpr std::string_view post{
        "// composite background if lying beyond the last volume sample\n"
        "if (bgTDepth > tEnd - tIncr * 0.5) {\n"
        "    result = drawBackground(result, bgTDepth, tIncr * 0.5, backgroundColor, bgTDepth, "
        "tDepth);\n"
        "}\n"};

    if (background_.isConnected()) {
        return {Segment{"#include \"utils/raycastgeometry.glsl\"", Segment::include, 900},
                Segment{fmt::format(uniforms, background_.getIdentifier()), Segment::uniform, 900},
                Segment{fmt::format(main, background_.getIdentifier()), Segment::main, 900},
                Segment{std::string(pre), Segment::pre, 900},
                Segment{std::string(loop), Segment::loop, 900},
                Segment{std::string(post), Segment::post, 900}};
    } else {
        return {};
    }
}

}  // namespace inviwo
