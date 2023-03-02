/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <modules/basegl/shadercomponents/isocomponent.h>

#include <algorithm>  // for max

#include <fmt/core.h>  // for format

namespace inviwo {

IsoComponent::IsoComponent(std::string_view identifier, std::string_view name, Document help,
                           VolumeInport& volume)
    : ShaderComponent()
    , iso{identifier, name, std::move(help),
          IsoValueCollection{std::vector<TFPrimitiveData>{TFPrimitiveData{0.5, vec4{1}}}},
          &volume} {}

std::string_view IsoComponent::getName() const { return iso.getIdentifier(); }

void IsoComponent::process(Shader& shader, TextureUnitContainer&) {
    const auto positions = iso.get().getPositionsf();
    const auto colors = iso.get().getColors();
    const auto name = iso.getIdentifier();
    StrBuffer buff;
    shader.setUniform(buff.replace("{}.values", name), positions);
    shader.setUniform(buff.replace("{}.colors", name), colors);
    shader.setUniform(buff.replace("{}.size", name), static_cast<int>(colors.size()));
}

void IsoComponent::initializeResources(Shader& shader) {
    // need to ensure there is always at least one isovalue due to the use of the macro
    // as array size in IsovalueParameters
    size_t isoCount = std::max(size_t{1}, iso.get().size());
    shader.getFragmentShaderObject()->addShaderDefine("MAX_ISOVALUE_COUNT",
                                                      fmt::format("{}", isoCount));
}

std::vector<Property*> IsoComponent::getProperties() { return {&iso}; }

constexpr std::string_view isoStruct = util::trim(R"(
#if !defined INC_ISOVALUEPARAMETERS
#define INC_ISOVALUEPARAMETERS
struct IsovalueParameters {
    float values[MAX_ISOVALUE_COUNT];
    vec4 colors[MAX_ISOVALUE_COUNT];
    int size;
};
#endif
)");

auto IsoComponent::getSegments() -> std::vector<Segment> {
    return {Segment{std::string{isoStruct}, placeholder::include, 1000 + 1},
            Segment{fmt::format("uniform IsovalueParameters {};", iso.getIdentifier()),
                    placeholder::uniform, 1000 + 1}};
}

}  // namespace inviwo
