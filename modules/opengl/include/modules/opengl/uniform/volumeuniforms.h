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

#include <tuple>
#include <utility>
#include <string_view>

namespace inviwo::uniform {

template <>
struct UniformTraits<Volume> {
    using CT = StructuredCoordinateTransformer<3>;
    static constexpr std::string_view structFormat = UniformTraits<CT>::structFormat;

    static constexpr auto getCT = [](const Volume& v) -> const CT& {
        return v.getCoordinateTransformer();
    };

    static constexpr auto scalingfactor = [](const Volume& volume) {
        const dvec2 dataRange = volume.dataMap_.dataRange;
        const double invRange = 1.0 / (dataRange.y - dataRange.x);
        switch (GLFormats::get(volume.getDataFormat()->getId()).normalization) {
            default:
                [[fallthrough]];
            case utilgl::Normalization::None: {
                return invRange;
            }
            case utilgl::Normalization::Normalized: {
                const dvec2 defaultRange = DataMapper{volume.getDataFormat()}.dataRange;
                return invRange * (defaultRange.y - defaultRange.x);
            }
            case utilgl::Normalization::SignNormalized: {
                const dvec2 defaultRange = DataMapper{volume.getDataFormat()}.dataRange;
                return invRange * 0.5 * (defaultRange.y - defaultRange.x);
            }
        }
    };

    static constexpr auto offset = [](const Volume& volume) {
        const dvec2 dataRange = volume.dataMap_.dataRange;
        switch (GLFormats::get(volume.getDataFormat()->getId()).normalization) {
            default:
                [[fallthrough]];
            case utilgl::Normalization::None: {
                return -dataRange.x;
            }
            case utilgl::Normalization::Normalized: {
                const dvec2 defaultRange = DataMapper{volume.getDataFormat()}.dataRange;
                return -(dataRange.x - defaultRange.x) / (defaultRange.y - defaultRange.x);
            }
            case utilgl::Normalization::SignNormalized: {
                const dvec2 defaultRange = DataMapper{volume.getDataFormat()}.dataRange;
                return 1.0 - 2 * (dataRange.x - defaultRange.x) / (defaultRange.y - defaultRange.x);
            }
        }
    };
    static constexpr auto signedScalingFactor = [](const Volume& volume) {
        const dvec2 dataRange = volume.dataMap_.dataRange;
        const double invRange = 1.0 / (dataRange.y - dataRange.x);
        switch (GLFormats::get(volume.getDataFormat()->getId()).normalization) {
            default:
                [[fallthrough]];
            case utilgl::Normalization::None: {
                return invRange;
            }
            case utilgl::Normalization::Normalized: {
                const dvec2 defaultRange = DataMapper{volume.getDataFormat()}.dataRange;
                return invRange * (defaultRange.y - defaultRange.x);
            }
            case utilgl::Normalization::SignNormalized: {
                const dvec2 defaultRange = DataMapper{volume.getDataFormat()}.dataRange;
                return invRange * (defaultRange.y - defaultRange.x);
            }
        }
    };

    static constexpr auto signedOffset = [](const Volume& volume) {
        const dvec2 dataRange = volume.dataMap_.dataRange;
        switch (GLFormats::get(volume.getDataFormat()->getId()).normalization) {
            default:
                [[fallthrough]];
            case utilgl::Normalization::None: {
                return -dataRange.x;
            }
            case utilgl::Normalization::Normalized: {
                const dvec2 defaultRange = DataMapper{volume.getDataFormat()}.dataRange;
                return -(dataRange.x - defaultRange.x) / (defaultRange.y - defaultRange.x);
            }
            case utilgl::Normalization::SignNormalized: {
                const dvec2 defaultRange = DataMapper{volume.getDataFormat()}.dataRange;
                return -(dataRange.x - defaultRange.x) / (defaultRange.y - defaultRange.x);
            }
        }
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
        std::pair{"worldSpaceGradientSpacing", &Volume::getWorldSpaceGradientSpacing},
        // offset scaling because of reversed scaling in the shader, i.e.
        // (1 - formatScaling_)
        std::pair{"formatScaling", asOneMinus<Volume, float>(scalingfactor)},
        std::pair{"formatOffset", asIdentity<Volume, float>(offset)},
        std::pair{"signedFormatScaling", asOneMinus<Volume, float>(signedScalingFactor)},
        std::pair{"signedFormatOffset", asIdentity<Volume, float>(signedOffset)}};

    inline static const auto uniforms =
        std::tuple_cat(tupleMap(
                           [](auto&& item) {
                               return std::pair{item.first, composition(item.second, getCT)};
                           },
                           UniformTraits<CT>::uniforms),
                       volumeUniforms);
};

}  // namespace inviwo::uniform
