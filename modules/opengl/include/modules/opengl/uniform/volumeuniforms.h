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

#include <inviwo/core/datastructures/volume/volume.h>
#include <modules/opengl/uniform/sctuniforms.h>
#include <modules/opengl/util/glformatutils.h>

#include <tuple>
#include <utility>
#include <string_view>

namespace inviwo::uniform {

template <>
struct UniformTraits<Volume> {
    using CT = StructuredCoordinateTransformer;
    static constexpr std::string_view structFormat = UniformTraits<CT>::structFormat;

    static constexpr auto getCT = [](const Volume& v) -> const CT& {
        return v.getCoordinateTransformer();
    };
    static constexpr auto getConv = [](const Volume& v) -> utilgl::FormatConversion {
        return utilgl::createGLFormatRenormalization(v.dataMap, v.getDataFormat());
    };

    // Transform the world space gradient spacing to texture space.
    // Wold space gradient spacing is given by:
    // mat3{ gradientSpacing.x         0                     0
    //             0             gradientSpacing.y           0
    //             0                   0               gradientSpacing.z }
    // which means that the transformation is equal to scaling the world to texture matrix.
    inline static const auto volumeUniforms = std::tuple{
        std::pair{"dimensions", asIdentity<Volume, vec3>(&Volume::getDimensions)},
        std::pair{"reciprocalDimensions", asReciprocal<Volume, vec3>(&Volume::getDimensions)},

        std::pair{"textureSpaceGradientSpacing",
                  [](const Volume& volume) {
                      const auto gradientSpacing = volume.getWorldSpaceGradientSpacing();
                      return mat3(
                          glm::scale(volume.getCoordinateTransformer().getWorldToTextureMatrix(),
                                     gradientSpacing));
                  }},
        std::pair{"worldSpaceGradientSpacing", &Volume::getWorldSpaceGradientSpacing}};

    inline static const auto convUniforms =
        std::tuple{std::pair{"texToNormalized.scale",
                             [](const utilgl::FormatConversion& conv) {
                                 return static_cast<float>(conv.texToNormalized.scale);
                             }},
                   std::pair{"texToNormalized.offset",
                             [](const utilgl::FormatConversion& conv) {
                                 return static_cast<float>(conv.texToNormalized.offset);
                             }},
                   std::pair{"texToSignNormalized.scale",
                             [](const utilgl::FormatConversion& conv) {
                                 return static_cast<float>(conv.texToSignNormalized.scale);
                             }},
                   std::pair{"texToSignNormalized.offset",
                             [](const utilgl::FormatConversion& conv) {
                                 return static_cast<float>(conv.texToSignNormalized.offset);
                             }},

                   std::pair{"texToValue.scale",
                             [](const utilgl::FormatConversion& conv) {
                                 return static_cast<float>(conv.texToValue.scale);
                             }},
                   std::pair{"texToValue.inputOffset",
                             [](const utilgl::FormatConversion& conv) {
                                 return static_cast<float>(conv.texToValue.inputOffset);
                             }},
                   std::pair{"texToValue.outputOffset", [](const utilgl::FormatConversion& conv) {
                                 return static_cast<float>(conv.texToValue.outputOffset);
                             }}};

    inline static const auto uniforms =
        std::tuple_cat(tupleMap(
                           [](auto&& item) {
                               return std::pair{item.first, composition(item.second, getCT)};
                           },
                           UniformTraits<CT>::uniforms),

                       tupleMap(
                           [](auto&& item) {
                               return std::pair{item.first, composition(item.second, getConv)};
                           },
                           convUniforms),

                       volumeUniforms);
};

}  // namespace inviwo::uniform
