/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/basegl/shadercomponents/highlightcomponent.h>

#include <inviwo/core/util/stringconversion.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureutils.h>

#include <string_view>
#include <fmt/core.h>
#include <fmt/format.h>

namespace inviwo {

namespace {

const std::vector<OptionPropertyIntOption> channelsList = util::enumeratedOptions("Channel", 8);

}

HighlightComponent::HighlightComponent()
    : ShaderComponent{}
    , compositeProperty_{"highlight", "Highlight", false}
    , position_{"highlightPosition", "Position",
                OrdinalPropertyState<vec2>{vec2{-1.0f}, vec2{0.0f}, ConstraintBehavior::Ignore,
                                           vec2{1.0f}, ConstraintBehavior::Ignore}}
    , radius_{"highlightRadius", "Radius (Pixel)", util::ordinalLength(50.0f, 500.0f)}
    , tf_{"highlightTF", "Transfer Function"}
    , highlightedChannel_{"highlightedChannel", "Highlighted Channel",
                          "Highlighted channel of the input volume"_help,
                          util::enumeratedOptions("Channel", 8), 0} {

    compositeProperty_.addProperties(position_, radius_, tf_, highlightedChannel_);
}

std::string_view HighlightComponent::getName() const { return tf_.getIdentifier(); }

void HighlightComponent::initializeResources(Shader&) {}

std::vector<std::tuple<Inport*, std::string>> HighlightComponent::getInports() { return {}; }

void HighlightComponent::process(Shader& shader, TextureUnitContainer& cont) {
    shader.setUniform("highlightEnabled", compositeProperty_.isChecked());
    utilgl::setUniforms(shader, position_, radius_, highlightedChannel_);

    utilgl::bindAndSetUniforms(shader, cont, tf_);
}

std::vector<Property*> HighlightComponent::getProperties() { return {&compositeProperty_}; }

namespace {

constexpr std::string_view uniforms = util::trim(R"(
uniform sampler2D {tf};
uniform bool highlightEnabled = false;
uniform vec2 {pos};
uniform float {radius};
uniform int highlightedChannel = 0;
)");

constexpr std::string_view setup = util::trim(R"(
bool isHighlighted = highlightEnabled
    && length({pos} * outportParameters.dimensions - gl_FragCoord.xy) < {radius};
)");

constexpr std::string_view post = util::trim(R"(
// proof of concept
//if (isHighlighted) {{
//    result.r = 1.0;
//}}
)");

}  // namespace

auto HighlightComponent::getSegments() -> std::vector<Segment> {
    using namespace fmt::literals;

    return {
        {fmt::format(uniforms, "pos"_a = position_.getIdentifier(),
                     "radius"_a = radius_.getIdentifier(), "tf"_a = tf_.getIdentifier()),
         placeholder::uniform, 450},
        {fmt::format(setup, "pos"_a = position_.getIdentifier(),
                     "radius"_a = radius_.getIdentifier()),
         placeholder::setup, 200},
        {fmt::format(post, "pos"_a = position_.getIdentifier(),
                     "radius"_a = radius_.getIdentifier()),
         placeholder::post, 200},
    };
}

}  // namespace inviwo
