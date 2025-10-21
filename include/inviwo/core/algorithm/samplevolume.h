/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/interpolation.h>

namespace inviwo::sample {

using ptrdiff3_t = glm::vec<3, std::ptrdiff_t>;
namespace detail {

template <typename Index, typename Functor, Index... Is>
constexpr auto build_array_impl(Functor&& func, std::integer_sequence<Index, Is...>) noexcept {
    return std::array{func(std::integral_constant<Index, Is>{})...};
}

template <std::size_t N, typename Index = size_t, typename Functor>
constexpr auto build_array(Functor&& func) noexcept {
    return build_array_impl<Index>(std::forward<Functor>(func),
                                   std::make_integer_sequence<Index, N>());
}

inline double glClamp(double x) { return std::clamp(x, 0.0, 1.0); }
inline double glRepeat(double x) { return x - std::floor(x); }
inline double glMirrorRepeat(double x) {
    const double t = x - std::floor(x / 2.0) * 2.0;
    return 1.0 - std::fabs(t - 1.0);
}

template <Wrapping W>
double wrap(double x) {
    if constexpr (W == Wrapping::Clamp) {
        return glClamp(x);
    } else if constexpr (W == Wrapping::Repeat) {
        return glRepeat(x);
    } else {  // Wrapping::Mirror
        return glMirrorRepeat(x);
    }
}

template <Wrapping W>
size_t wrap(ptrdiff_t i, ptrdiff_t dim) {
    if constexpr (W == Wrapping::Clamp) {
        return std::clamp(i, ptrdiff_t{0}, dim - 1);
    } else if constexpr (W == Wrapping::Repeat) {
        if (i < ptrdiff_t{0}) return i + dim;
        return i < dim ? i : i - dim;
    } else {  // Wrapping::Mirror
        if (i < ptrdiff_t{0}) return -i;
        return i < dim ? i : dim - (i - dim + 1);
    }
}

template <Wrapping X, Wrapping Y, Wrapping Z>
dvec3 wrap(const dvec3& p) {
    return {wrap<X>(p.x), wrap<Y>(p.y), wrap<Z>(p.z)};
}

template <Wrapping X, Wrapping Y, Wrapping Z>
size3_t wrap(const size3_t& p, const size3_t& dims) {
    return {wrap<X>(p.x, dims.x), wrap<Y>(p.y, dims.y), wrap<Z>(p.z, dims.z)};
}

template <typename T>
struct SampleState {
    std::span<const T> data;
    size_t dims;
    util::IndexMapper3D im;
    dmat4 posToTexture;
    dmat4 textureToIndex;
};

template <typename T, size_t N, InterpolationType I, Wrapping X, Wrapping Y, Wrapping Z>
void sample(const VolumeRAMPrecision<T>& volume, std::span<const dvec3> positions,
            std::span<Vector<N, double>> output, const dmat4& posToTexture,
            const dmat4& textureToIndex) {

    using R = util::same_extent_t<T, double>;

    const auto dims = volume.getDimensions();
    const auto im = util::IndexMapper3D{dims};
    const auto data = volume.getView();

    const auto get = [&](const size3_t& index) {
        const auto i = im(index);
        IVW_ASSERT(i < data.size(), "out of bounds access");
        return static_cast<R>(data[i]);
    };

    if constexpr (I == InterpolationType::Linear) {
        std::ranges::transform(positions, output.begin(), [&](dvec3 position) {
            const auto texPos = wrap<X, Y, Z>(dvec3(posToTexture * dvec4{position, 1.0}));
            const auto index = dvec3(textureToIndex * dvec4{texPos, 1.0});

            const auto fIndex = static_cast<ptrdiff3_t>(glm::floor(index));
            const std::array<const R, 8> samples{
                get(wrap<X, Y, Z>(fIndex + ptrdiff3_t{0, 0, 0}, dims)),
                get(wrap<X, Y, Z>(fIndex + ptrdiff3_t{1, 0, 0}, dims)),
                get(wrap<X, Y, Z>(fIndex + ptrdiff3_t{0, 1, 0}, dims)),
                get(wrap<X, Y, Z>(fIndex + ptrdiff3_t{1, 1, 0}, dims)),
                get(wrap<X, Y, Z>(fIndex + ptrdiff3_t{0, 0, 1}, dims)),
                get(wrap<X, Y, Z>(fIndex + ptrdiff3_t{1, 0, 1}, dims)),
                get(wrap<X, Y, Z>(fIndex + ptrdiff3_t{0, 1, 1}, dims)),
                get(wrap<X, Y, Z>(fIndex + ptrdiff3_t{1, 1, 1}, dims)),
            };

            const auto value = interpolate::trilinear<R>(samples, index - glm::floor(index));
            return util::glm_convert<Vector<N, double>>(value);
        });
    } else {
        std::ranges::transform(positions, output.begin(), [&](dvec3 position) {
            position = wrap<X, Y, Z>(dvec3(posToTexture * dvec4{position, 1.0}));
            const auto index = dvec3(textureToIndex * dvec4{position, 1.0});
            const auto value = get(static_cast<size3_t>(glm::round(index)));
            return util::glm_convert<Vector<N, double>>(value);
        });
    }
}

}  // namespace detail

template <size_t N, typename T>
void sample(const VolumeRAMPrecision<T>& volume, std::span<const dvec3> positions,
            std::span<Vector<N, double>> output, const dmat4& posToTexture,
            const dmat4& textureToIndex, const DataMapper& dataMap, DataMapper::Space space) {

    using Functor = void (*)(const VolumeRAMPrecision<T>&, std::span<const dvec3>,
                             std::span<Vector<N, double>>, const dmat4&, const dmat4&);

    constexpr auto table = detail::build_array<2>([&](auto i) constexpr {
        return detail::build_array<3>([&](auto x) constexpr {
            return detail::build_array<3>([&](auto y) constexpr {
                return detail::build_array<3>([&](auto z) constexpr -> Functor {
                    return
                        [](const VolumeRAMPrecision<T>& vol, std::span<const dvec3> pos,
                           std::span<Vector<N, double>> dst, const dmat4& p2t, const dmat4& t2i) {
                            using IT = decltype(i);
                            using XT = decltype(x);
                            using YT = decltype(y);
                            using ZT = decltype(z);
                            constexpr auto I = static_cast<InterpolationType>(IT::value);
                            constexpr auto X = static_cast<Wrapping>(XT::value);
                            constexpr auto Y = static_cast<Wrapping>(YT::value);
                            constexpr auto Z = static_cast<Wrapping>(ZT::value);
                            detail::sample<T, N, I, X, Y, Z>(vol, pos, dst, p2t, t2i);
                        };
                });
            });
        });
    });

    const auto wrapping = volume.getWrapping();
    const auto interpolation = volume.getInterpolation();

    table[std::to_underlying(interpolation)][std::to_underlying(wrapping[0])]
         [std::to_underlying(wrapping[1])][std::to_underlying(wrapping[2])](
             volume, positions, output, posToTexture, textureToIndex);

    if (space == DataMapper::Space::Normalized) {
        std::ranges::transform(output, output.begin(), [&](auto value) {
            return dataMap.mapFromDataTo<DataMapper::Space::Normalized>(value);
        });
    } else if (space == DataMapper::Space::SignNormalized) {
        std::ranges::transform(output, output.begin(), [&](auto value) {
            return dataMap.mapFromDataTo<DataMapper::Space::SignNormalized>(value);
        });
    } else if (space == DataMapper::Space::Value) {
        std::ranges::transform(output, output.begin(), [&](auto value) {
            return dataMap.mapFromDataTo<DataMapper::Space::Value>(value);
        });
    }
}

IVW_CORE_API void sample(const VolumeRAM& volume, std::span<const dvec3> positions,
                         std::span<double> output, const dmat4& posToTexture,
                         const dmat4& textureToIndex, const DataMapper& dataMap,
                         DataMapper::Space space);
IVW_CORE_API void sample(const VolumeRAM& volume, std::span<const dvec3> positions,
                         std::span<dvec2> output, const dmat4& posToTexture,
                         const dmat4& textureToIndex, const DataMapper& dataMap,
                         DataMapper::Space space);
IVW_CORE_API void sample(const VolumeRAM& volume, std::span<const dvec3> positions,
                         std::span<dvec3> output, const dmat4& posToTexture,
                         const dmat4& textureToIndex, const DataMapper& dataMap,
                         DataMapper::Space space);
IVW_CORE_API void sample(const VolumeRAM& volume, std::span<const dvec3> positions,
                         std::span<dvec4> output, const dmat4& posToTexture,
                         const dmat4& textureToIndex, const DataMapper& dataMap,
                         DataMapper::Space space);

IVW_CORE_API void sample(const Volume& volume, std::span<const dvec3> positions,
                         std::span<double> output, CoordinateSpace positionSpace,
                         DataMapper::Space outputSpace);
IVW_CORE_API void sample(const Volume& volume, std::span<const dvec3> positions,
                         std::span<dvec2> output, CoordinateSpace positionSpace,
                         DataMapper::Space outputSpace);
IVW_CORE_API void sample(const Volume& volume, std::span<const dvec3> positions,
                         std::span<dvec3> output, CoordinateSpace positionSpace,
                         DataMapper::Space outputSpace);
IVW_CORE_API void sample(const Volume& volume, std::span<const dvec3> positions,
                         std::span<dvec4> output, CoordinateSpace positionSpace,
                         DataMapper::Space outputSpace);

}  // namespace inviwo::sample
