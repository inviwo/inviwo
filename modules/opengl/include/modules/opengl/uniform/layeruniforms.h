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
#pragma once

#include <modules/opengl/openglmoduledefine.h>
#include <modules/opengl/uniform/uniform.h>
#include <modules/opengl/uniform/sctuniforms.h>
#include <inviwo/core/datastructures/image/layer.h>

#include <tuple>
#include <utility>
#include <string_view>

namespace inviwo::uniform {
using namespace std::literals;

template <>
struct UniformTraits<Layer> {
    using CT = StructuredCoordinateTransformer<2>;
    static constexpr std::string_view structFormat = UniformTraits<CT>::structFormat;

    static constexpr auto getCT = [](const Layer& l) -> const CT& {
        return l.getCoordinateTransformer();
    };

    inline static const auto uniforms = std::tuple_cat(
        tupleMap(
            [](auto&& item) {
                return std::pair{item.first, composition(item.second, getCT)};
            },
            UniformTraits<CT>::uniforms),
        std::tuple{
            std::pair{"dimensions"sv, asIdentity<Layer, vec2>(&Layer::getDimensions)},
            std::pair{"reciprocalDimensions"sv, asReciprocal<Layer, vec2>(&Layer::getDimensions)}});
};

}  // namespace inviwo::uniform
