/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2022 Inviwo Foundation
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

#include <modules/base/algorithm/volume/volumevoronoi.h>

#include <inviwo/core/datastructures/datamapper.h>        // for DataMapper
#include <inviwo/core/datastructures/image/imagetypes.h>  // for Wrapping, Wrapping3D, Wrapping:...
#include <inviwo/core/datastructures/volume/volume.h>     // for Volume
#include <inviwo/core/datastructures/volume/volumeram.h>  // for VolumeRAMPrecision
#include <inviwo/core/util/exception.h>                   // for Exception
#include <inviwo/core/util/glmmat.h>                      // for mat4
#include <inviwo/core/util/glmvec.h>                      // for vec3, size3_t, vec4, dvec2
#include <inviwo/core/util/indexmapper.h>                 // for IndexMapper, IndexMapper3D
#include <inviwo/core/util/sourcecontext.h>               // for IVW_CONTEXT_CUSTOM
#include <inviwo/core/util/volumeramutils.h>              // for forEachVoxelParallel
#include <inviwo/core/util/zip.h>                         // for zip, zipper

#include <algorithm>    // for max_element, min_element
#include <array>        // for array<>::value_type, array
#include <cstddef>      // for size_t
#include <functional>   // for __base
#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t, integral_constant

#include <glm/geometric.hpp>  // for dot
#include <glm/gtx/norm.hpp>   // for length2
#include <glm/mat4x4.hpp>     // for operator*
#include <glm/vec3.hpp>       // for operator-, operator*
#include <glm/vec4.hpp>       // for operator*, operator+

namespace inviwo {
namespace util {

namespace detail {
template <Wrapping X, Wrapping Y, Wrapping Z>
auto distance2(const vec3& a, const vec3& b, const mat3& dataToModelMatrix) {
    auto delta = b - a;

    if constexpr (X == Wrapping::Repeat) {
        if (delta.x > 0.5f) delta.x -= 1.0f;
        if (delta.x < -0.5f) delta.x += 1.0f;
    }

    if constexpr (Y == Wrapping::Repeat) {
        if (delta.y > 0.5f) delta.y -= 1.0f;
        if (delta.y < -0.5f) delta.y += 1.0f;
    }

    if constexpr (Z == Wrapping::Repeat) {
        if (delta.z > 0.5f) delta.z -= 1.0f;
        if (delta.z < -0.5f) delta.z += 1.0f;
    }

    return glm::length2(dataToModelMatrix * delta);
}

template <typename Index, typename Functor, Index... Is>
constexpr auto build_array_impl(Functor&& func, std::integer_sequence<Index, Is...>) noexcept {
    return std::array{func(std::integral_constant<Index, Is>{})...};
}

template <std::size_t N, typename Index = size_t, typename Functor>
constexpr auto build_array(Functor&& func) noexcept {
    return build_array_impl<Index>(std::forward<Functor>(func),
                                   std::make_integer_sequence<Index, N>());
}

}  // namespace detail

template <Wrapping X, Wrapping Y, Wrapping Z>
void voronoiSegmentationImpl(
    const size3_t volumeDimensions, const mat4& indexToDataMatrix, const mat4& dataToModelMatrix,
    const std::vector<std::pair<unsigned short, vec3>>& seedPointsWithIndices,
    VolumeRAMPrecision<unsigned short>& voronoiVolumeRep) {

    auto volumeIndices = voronoiVolumeRep.getDataTyped();
    util::IndexMapper3D index(volumeDimensions);

    // We can ignore any translations
    const auto d2m = mat3{dataToModelMatrix};

    util::forEachVoxelParallel(volumeDimensions, [&](const size3_t& voxelPos) {
        const auto dataVoxelPos = vec3{indexToDataMatrix * vec4{voxelPos, 1.0f}};
        const auto it =
            std::min_element(seedPointsWithIndices.cbegin(), seedPointsWithIndices.cend(),
                             [&](const auto& p1, const auto& p2) {
                                 return detail::distance2<X, Y, Z>(p1.second, dataVoxelPos, d2m) <
                                        detail::distance2<X, Y, Z>(p2.second, dataVoxelPos, d2m);
                             });
        volumeIndices[index(voxelPos)] = it->first;
    });
}

template <Wrapping X, Wrapping Y, Wrapping Z>
void weightedVoronoiSegmentationImpl(
    const size3_t volumeDimensions, const mat4& indexToDataMatrix, const mat4& dataToModelMatrix,
    const std::vector<std::pair<unsigned short, vec3>>& seedPointsWithIndices,
    const std::vector<float>& weights, VolumeRAMPrecision<unsigned short>& voronoiVolumeRep) {

    auto volumeIndices = voronoiVolumeRep.getDataTyped();
    util::IndexMapper3D index(volumeDimensions);

    // We can ignore any translations
    const auto d2m = mat3{dataToModelMatrix};

    util::forEachVoxelParallel(volumeDimensions, [&](const size3_t& voxelPos) {
        const auto dataVoxelPos = vec3{indexToDataMatrix * vec4{voxelPos, 1.0f}};
        auto zipped = util::zip(seedPointsWithIndices, weights);

        const auto&& [posWithIndex, weight] =
            *std::min_element(zipped.begin(), zipped.end(), [&](auto&& i1, auto&& i2) {
                auto&& [p1, w1] = i1;
                auto&& [p2, w2] = i2;
                return detail::distance2<X, Y, Z>(p1.second, dataVoxelPos, d2m) - w1 * w1 <
                       detail::distance2<X, Y, Z>(p2.second, dataVoxelPos, d2m) - w2 * w2;
            });
        volumeIndices[index(voxelPos)] = posWithIndex.first;
    });
}

std::shared_ptr<Volume> voronoiSegmentation(
    const size3_t volumeDimensions, const mat4& indexToDataMatrix, const mat4& dataToModelMatrix,
    const std::vector<std::pair<uint32_t, vec3>>& seedPointsWithIndices, const Wrapping3D& wrapping,
    const std::optional<std::vector<float>>& weights) {

    if (seedPointsWithIndices.size() == 0) {
        throw Exception("No seed points, cannot create volume voronoi segmentation",
                        IVW_CONTEXT_CUSTOM("VoronoiSegmentation"));
    }

    if (weights.has_value() && weights.value().size() != seedPointsWithIndices.size()) {
        throw Exception(
            "Cannot use weighted voronoi when dimensions do not match (weights and seed "
            "positions)",
            IVW_CONTEXT_CUSTOM("VoronoiSegmentation"));
    }

    if (wrapping[0] == Wrapping::Mirror || wrapping[1] == Wrapping::Mirror ||
        wrapping[2] == Wrapping::Mirror) {
        throw Exception("Mirror Wrapping is not supported",
                        IVW_CONTEXT_CUSTOM("VoronoiSegmentation"));
    }

    auto voronoiVolumeRep = std::make_shared<VolumeRAMPrecision<unsigned short>>(volumeDimensions);
    auto voronoiVolume = std::make_shared<Volume>(voronoiVolumeRep);
    voronoiVolume->setInterpolation(InterpolationType::Nearest);
    voronoiVolume->setWrapping(wrapping);

    const auto [itMin, itMax] =
        std::minmax_element(seedPointsWithIndices.begin(), seedPointsWithIndices.end(),
                            [](const auto& a, const auto& b) { return a.first < b.first; });

    voronoiVolume->dataMap_.dataRange =
        dvec2{static_cast<double>(itMin->first), static_cast<double>(itMax->first)};
    voronoiVolume->dataMap_.valueRange = voronoiVolume->dataMap_.dataRange;

    if (itMax->first > std::numeric_limits<unsigned short>::max()) {
        throw Exception(IVW_CONTEXT_CUSTOM("VoronoiSegmentation"),
                        "Seed point index greater than {} it not supported",
                        std::numeric_limits<unsigned short>::max());
    }

    std::vector<std::pair<unsigned short, vec3>> dataSeedPointsWithIndices{
        seedPointsWithIndices.size()};

    const auto modelToDataMatrix = glm::inverse(dataToModelMatrix);
    std::transform(seedPointsWithIndices.begin(), seedPointsWithIndices.end(),
                   dataSeedPointsWithIndices.begin(),
                   [&modelToDataMatrix](const std::pair<uint32_t, vec3>& pair) {
                       return std::pair<uint32_t, vec3>{
                           static_cast<unsigned short>(pair.first),
                           vec3{modelToDataMatrix * vec4{pair.second, 1.0f}}};
                   });

    if (weights.has_value()) {
        using Functor = void (*)(const size3_t, const mat4&, const mat4&,
                                 const std::vector<std::pair<unsigned short, vec3>>&,
                                 const std::vector<float>&, VolumeRAMPrecision<unsigned short>&);

        constexpr auto table = detail::build_array<3>([&](auto x) constexpr {
            using XT = decltype(x);
            return detail::build_array<3>([&](auto y) constexpr {
                using YT = decltype(y);
                return detail::build_array<3>([&](auto z) constexpr -> Functor {
                    using ZT = decltype(z);
                    return [](const size3_t dim, const mat4& i2d, const mat4& d2m,
                              const std::vector<std::pair<unsigned short, vec3>>& sp,
                              const std::vector<float>& w,
                              VolumeRAMPrecision<unsigned short>& volRep) {
                        constexpr auto X = static_cast<Wrapping>(XT::value);
                        constexpr auto Y = static_cast<Wrapping>(YT::value);
                        constexpr auto Z = static_cast<Wrapping>(ZT::value);
                        weightedVoronoiSegmentationImpl<X, Y, Z>(dim, i2d, d2m, sp, w, volRep);
                    };
                });
            });
        });

        table[static_cast<size_t>(wrapping[0])][static_cast<size_t>(wrapping[1])]
             [static_cast<size_t>(wrapping[2])](volumeDimensions, indexToDataMatrix,
                                                dataToModelMatrix, dataSeedPointsWithIndices,
                                                *weights, *voronoiVolumeRep);

    } else {
        using Functor = void (*)(const size3_t, const mat4&, const mat4&,
                                 const std::vector<std::pair<unsigned short, vec3>>&,
                                 VolumeRAMPrecision<unsigned short>&);

        constexpr auto table = detail::build_array<3>([&](auto x) constexpr {
            using XT = decltype(x);
            return detail::build_array<3>([&](auto y) constexpr {
                using YT = decltype(y);
                return detail::build_array<3>([&](auto z) constexpr -> Functor {
                    using ZT = decltype(z);
                    return [](const size3_t dim, const mat4& i2d, const mat4& d2m,
                              const std::vector<std::pair<unsigned short, vec3>>& sp,
                              VolumeRAMPrecision<unsigned short>& volRep) {
                        constexpr auto X = static_cast<Wrapping>(XT::value);
                        constexpr auto Y = static_cast<Wrapping>(YT::value);
                        constexpr auto Z = static_cast<Wrapping>(ZT::value);
                        voronoiSegmentationImpl<X, Y, Z>(dim, i2d, d2m, sp, volRep);
                    };
                });
            });
        });

        table[static_cast<size_t>(wrapping[0])][static_cast<size_t>(wrapping[1])]
             [static_cast<size_t>(wrapping[2])](volumeDimensions, indexToDataMatrix,
                                                dataToModelMatrix, dataSeedPointsWithIndices,
                                                *voronoiVolumeRep);
    }

    return voronoiVolume;
}

}  // namespace util
}  // namespace inviwo
