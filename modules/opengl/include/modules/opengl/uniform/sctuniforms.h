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
#include <modules/opengl/glformats.h>

#include <inviwo/core/datastructures/coordinatetransformer.h>

#include <tuple>
#include <utility>
#include <string_view>

namespace inviwo::uniform {
using namespace std::literals;

template <size_t N>
struct UniformTraits<StructuredCoordinateTransformer<N>> {
    using CT = StructuredCoordinateTransformer<N>;
    static constexpr std::string_view structFormat{"{}Parameters"};
    // Using constexpr here will crash MSVC or result in runtime errors,
    // https://godbolt.org/z/WGa9o3
    inline static const auto uniforms =
        std::tuple{std::pair{"dataToModel"sv, &CT::getDataToModelMatrix},
                   std::pair{"modelToData"sv, &CT::getModelToDataMatrix},
                   std::pair{"dataToWorld"sv, &CT::getDataToWorldMatrix},
                   std::pair{"worldToData"sv, &CT::getWorldToDataMatrix},
                   std::pair{"modelToWorld"sv, &CT::getModelToWorldMatrix},
                   std::pair{"worldToModel"sv, &CT::getWorldToModelMatrix},
                   std::pair{"worldToTexture"sv, &CT::getWorldToTextureMatrix},
                   std::pair{"textureToWorld"sv, &CT::getTextureToWorldMatrix},
                   std::pair{"textureToIndex"sv, &CT::getTextureToIndexMatrix},
                   std::pair{"indexToTexture"sv, &CT::getIndexToTextureMatrix},
                   std::pair{"textureToWorldNormalMatrix"sv, [](const CT& ct) {
                                 return glm::inverseTranspose(ct.getTextureToWorldMatrix());
                             }}};
};

}  // namespace inviwo::uniform
