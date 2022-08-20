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

#include <array>

namespace inviwo {
namespace util {

namespace detail {
template <Wrapping X, Wrapping Y, Wrapping Z, typename T>
auto distance2(const glm::vec<3, T>& a, const glm::vec<3, T>& b,
               [[maybe_unused]] const glm::vec<3, T>& size) {
    auto delta = b - a;

    if constexpr (X == Wrapping::Repeat) {
        if (delta.x > 0.5 * size.x) delta.x -= size.x;
        if (delta.x < -0.5 * size.x) delta.x += size.x;
    }

    if constexpr (Y == Wrapping::Repeat) {
        if (delta.y > 0.5 * size.y) delta.y -= size.y;
        if (delta.y < -0.5 * size.y) delta.y += size.y;
    }

    if constexpr (Z == Wrapping::Repeat) {
        if (delta.z > 0.5 * size.z) delta.z -= size.z;
        if (delta.z < -0.5 * size.z) delta.z += size.z;
    }

    return glm::length2(delta);
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
void voronoiSegmentationImpl(const size3_t volumeDimensions, const mat4& indexToModelMatrix,
                             const std::vector<std::pair<uint32_t, vec3>>& seedPointsWithIndices,
                             VolumeRAMPrecision<unsigned short>& voronoiVolumeRep) {

    auto volumeIndices = voronoiVolumeRep.getDataTyped();
    util::IndexMapper3D index(volumeDimensions);

    const auto size = vec3{indexToModelMatrix * vec4{volumeDimensions, 1.0f}} -
                      vec3{indexToModelMatrix * vec4{0.0f, 0.0f, 0.0f, 1.0f}};

    util::forEachVoxelParallel(volumeDimensions, [&](const size3_t& voxelPos) {
        const auto transformedVoxelPos = vec3{indexToModelMatrix * vec4{voxelPos, 1.0f}};
        auto it = std::min_element(
            seedPointsWithIndices.cbegin(), seedPointsWithIndices.cend(),
            [&](const auto& p1, const auto& p2) {
                return detail::distance2<X, Y, Z>(p1.second, transformedVoxelPos, size) <
                       detail::distance2<X, Y, Z>(p2.second, transformedVoxelPos, size);
            });
        volumeIndices[index(voxelPos)] = static_cast<unsigned short>(it->first);
    });
}

template <Wrapping X, Wrapping Y, Wrapping Z>
void weightedVoronoiSegmentationImpl(
    const size3_t volumeDimensions, const mat4& indexToModelMatrix,
    const std::vector<std::pair<uint32_t, vec3>>& seedPointsWithIndices,
    const std::vector<float>& weights, VolumeRAMPrecision<unsigned short>& voronoiVolumeRep) {

    auto volumeIndices = voronoiVolumeRep.getDataTyped();
    util::IndexMapper3D index(volumeDimensions);

    const auto size = vec3{indexToModelMatrix * vec4{volumeDimensions, 1.0f}} -
                      vec3{indexToModelMatrix * vec4{0.0f, 0.0f, 0.0f, 1.0f}};

    util::forEachVoxelParallel(volumeDimensions, [&](const size3_t& voxelPos) {
        const auto transformedVoxelPos = vec3{indexToModelMatrix * vec4{voxelPos, 1.0f}};
        auto zipped = util::zip(seedPointsWithIndices, weights);

        auto&& [posWithIndex, weight] =
            *std::min_element(zipped.begin(), zipped.end(), [&](auto&& i1, auto&& i2) {
                auto&& [p1, w1] = i1;
                auto&& [p2, w2] = i2;
                return detail::distance2<X, Y, Z>(p1.second, transformedVoxelPos, size) - w1 * w1 <
                       detail::distance2<X, Y, Z>(p2.second, transformedVoxelPos, size) - w2 * w2;
            });
        volumeIndices[index(voxelPos)] = static_cast<unsigned short>(posWithIndex.first);
    });
}

std::shared_ptr<Volume> voronoiSegmentation(
    const size3_t volumeDimensions, const mat4& indexToModelMatrix,
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

    auto voronoiVolumeRep = std::make_shared<VolumeRAMPrecision<unsigned short>>(volumeDimensions);
    auto voronoiVolume = std::make_shared<Volume>(voronoiVolumeRep);
    voronoiVolume->setInterpolation(InterpolationType::Nearest);
    voronoiVolume->setWrapping(wrapping);

    const auto imax =
        std::max_element(seedPointsWithIndices.begin(), seedPointsWithIndices.end(),
                         [](const auto& a, const auto& b) { return a.first < b.first; });

    voronoiVolume->dataMap_.dataRange = dvec2{0.0, static_cast<double>(imax->first)};
    voronoiVolume->dataMap_.valueRange = voronoiVolume->dataMap_.dataRange;

    if (weights.has_value()) {
        using Functor =
            void (*)(const size3_t, const mat4&, const std::vector<std::pair<uint32_t, vec3>>&,
                     const std::vector<float>&, VolumeRAMPrecision<unsigned short>&);

        constexpr auto table = detail::build_array<3>([&](auto x) constexpr {
            using XT = decltype(x);
            return detail::build_array<3>([&](auto y) constexpr {
                using YT = decltype(y);
                return detail::build_array<3>([&](auto z) constexpr->Functor {
                    using ZT = decltype(z);
                    return [](const size3_t dim, const mat4& matrix,
                              const std::vector<std::pair<uint32_t, vec3>>& sp,
                              const std::vector<float>& w,
                              VolumeRAMPrecision<unsigned short>& volRep) {
                        constexpr auto X = static_cast<Wrapping>(XT::value);
                        constexpr auto Y = static_cast<Wrapping>(YT::value);
                        constexpr auto Z = static_cast<Wrapping>(ZT::value);
                        weightedVoronoiSegmentationImpl<X, Y, Z>(dim, matrix, sp, w, volRep);
                    };
                });
            });
        });

        table[static_cast<size_t>(wrapping[0])][static_cast<size_t>(wrapping[1])]
             [static_cast<size_t>(wrapping[2])](volumeDimensions, indexToModelMatrix,
                                                seedPointsWithIndices, *weights, *voronoiVolumeRep);

    } else {
        using Functor =
            void (*)(const size3_t, const mat4&, const std::vector<std::pair<uint32_t, vec3>>&,
                     VolumeRAMPrecision<unsigned short>&);

        constexpr auto table = detail::build_array<3>([&](auto x) constexpr {
            using XT = decltype(x);
            return detail::build_array<3>([&](auto y) constexpr {
                using YT = decltype(y);
                return detail::build_array<3>([&](auto z) constexpr->Functor {
                    using ZT = decltype(z);
                    return [](const size3_t dim, const mat4& matrix,
                              const std::vector<std::pair<uint32_t, vec3>>& sp,
                              VolumeRAMPrecision<unsigned short>& volRep) {
                        constexpr auto X = static_cast<Wrapping>(XT::value);
                        constexpr auto Y = static_cast<Wrapping>(YT::value);
                        constexpr auto Z = static_cast<Wrapping>(ZT::value);
                        voronoiSegmentationImpl<X, Y, Z>(dim, matrix, sp, volRep);
                    };
                });
            });
        });

        table[static_cast<size_t>(wrapping[0])][static_cast<size_t>(wrapping[1])]
             [static_cast<size_t>(wrapping[2])](volumeDimensions, indexToModelMatrix,
                                                seedPointsWithIndices, *voronoiVolumeRep);
    }

    return voronoiVolume;
}

}  // namespace util
}  // namespace inviwo
