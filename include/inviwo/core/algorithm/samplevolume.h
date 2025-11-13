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

struct IVW_CORE_API SampleState {
    ptrdiff3_t dims;
    util::IndexMapper3D im;
    dmat4 posToTexture;
    dmat4 textureToIndex;
};

struct IVW_CORE_API State {
    const VolumeRAM& volumeRAM;
    dmat4 posToData;
    dmat4 dataToIndex;
    DataMapper dataMap;
    DataMapper::Space space;
};

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
size3_t wrap(const ptrdiff3_t& p, const ptrdiff3_t& dims) {
    return {wrap<X>(p.x, dims.x), wrap<Y>(p.y, dims.y), wrap<Z>(p.z, dims.z)};
}

template <typename T, size_t N, InterpolationType I, Wrapping X, Wrapping Y, Wrapping Z>
void sample(std::span<const T> data, const SampleState& state, std::span<const dvec3> positions,
            std::span<Vector<N, double>> output) {

    using R = util::same_extent_t<T, double>;

    const auto get = [&](const size3_t& index) {
        const auto i = state.im(index);
        IVW_ASSERT(i < data.size(), "out of bounds access");
        return static_cast<R>(data[i]);
    };

    if constexpr (I == InterpolationType::Linear) {
        std::ranges::transform(positions, output.begin(), [&](dvec3 position) {
            const auto texPos = wrap<X, Y, Z>(dvec3(state.posToTexture * dvec4{position, 1.0}));
            const auto index = dvec3(state.textureToIndex * dvec4{texPos, 1.0});

            const auto fIndex = static_cast<ptrdiff3_t>(glm::floor(index));
            const std::array<const R, 8> samples{
                get(wrap<X, Y, Z>(fIndex + ptrdiff3_t{0, 0, 0}, state.dims)),
                get(wrap<X, Y, Z>(fIndex + ptrdiff3_t{1, 0, 0}, state.dims)),
                get(wrap<X, Y, Z>(fIndex + ptrdiff3_t{0, 1, 0}, state.dims)),
                get(wrap<X, Y, Z>(fIndex + ptrdiff3_t{1, 1, 0}, state.dims)),
                get(wrap<X, Y, Z>(fIndex + ptrdiff3_t{0, 0, 1}, state.dims)),
                get(wrap<X, Y, Z>(fIndex + ptrdiff3_t{1, 0, 1}, state.dims)),
                get(wrap<X, Y, Z>(fIndex + ptrdiff3_t{0, 1, 1}, state.dims)),
                get(wrap<X, Y, Z>(fIndex + ptrdiff3_t{1, 1, 1}, state.dims)),
            };

            const auto value = interpolate::trilinear<R>(samples, index - glm::floor(index));
            return util::glm_convert<Vector<N, double>>(value);
        });
    } else {
        std::ranges::transform(positions, output.begin(), [&](dvec3 position) {
            position = wrap<X, Y, Z>(dvec3(state.posToTexture * dvec4{position, 1.0}));
            const auto index = dvec3(state.textureToIndex * dvec4{position, 1.0});
            const auto value = get(static_cast<size3_t>(glm::round(index)));
            return util::glm_convert<Vector<N, double>>(value);
        });
    }
}

}  // namespace detail

template <size_t N, typename T>
void sample(const State& state, const VolumeRAMPrecision<T>& volume,
            std::span<const dvec3> positions, std::span<Vector<N, double>> output) {

    using Functor = void (*)(std::span<const T>, const SampleState&, std::span<const dvec3>,
                             std::span<Vector<N, double>>);

    constexpr auto table = detail::build_array<2>([&](auto i) constexpr {
        return detail::build_array<3>([&](auto x) constexpr {
            return detail::build_array<3>([&](auto y) constexpr {
                return detail::build_array<3>([&](auto z) constexpr -> Functor {
                    return [](std::span<const T> data, const SampleState& state,
                              std::span<const dvec3> pos, std::span<Vector<N, double>> dst) {
                        using IT = decltype(i);
                        using XT = decltype(x);
                        using YT = decltype(y);
                        using ZT = decltype(z);
                        constexpr auto I = static_cast<InterpolationType>(IT::value);
                        constexpr auto X = static_cast<Wrapping>(XT::value);
                        constexpr auto Y = static_cast<Wrapping>(YT::value);
                        constexpr auto Z = static_cast<Wrapping>(ZT::value);
                        detail::sample<T, N, I, X, Y, Z>(data, state, pos, dst);
                    };
                });
            });
        });
    });

    const auto wrapping = volume.getWrapping();
    const auto interpolation = volume.getInterpolation();

    SampleState sstate{.dims = volume.getDimensions(),
                       .im = util::IndexMapper3D{volume.getDimensions()},
                       .posToTexture = state.posToData,
                       .textureToIndex = state.dataToIndex};

    table[std::to_underlying(interpolation)][std::to_underlying(wrapping[0])][std::to_underlying(
        wrapping[1])][std::to_underlying(wrapping[2])](volume.getView(), sstate, positions, output);

    if (state.space == DataMapper::Space::Normalized) {
        std::ranges::transform(output, output.begin(), [&](auto value) {
            return state.dataMap.mapFromDataTo<DataMapper::Space::Normalized>(value);
        });
    } else if (state.space == DataMapper::Space::SignNormalized) {
        std::ranges::transform(output, output.begin(), [&](auto value) {
            return state.dataMap.mapFromDataTo<DataMapper::Space::SignNormalized>(value);
        });
    } else if (state.space == DataMapper::Space::Value) {
        std::ranges::transform(output, output.begin(), [&](auto value) {
            return state.dataMap.mapFromDataTo<DataMapper::Space::Value>(value);
        });
    }
}

template <size_t N, typename T>
auto createFunctor(const State& state, const VolumeRAMPrecision<T>& volume)
    -> std::function<void(std::span<const dvec3>, std::span<Vector<N, double>>)> {

    using Functor = void (*)(std::span<const T>, const SampleState&, std::span<const dvec3>,
                             std::span<Vector<N, double>>);

    constexpr auto table = detail::build_array<2>([&](auto i) constexpr {
        return detail::build_array<3>([&](auto x) constexpr {
            return detail::build_array<3>([&](auto y) constexpr {
                return detail::build_array<3>([&](auto z) constexpr -> Functor {
                    return [](std::span<const T> data, const SampleState& state,
                              std::span<const dvec3> pos, std::span<Vector<N, double>> dst) {
                        using IT = decltype(i);
                        using XT = decltype(x);
                        using YT = decltype(y);
                        using ZT = decltype(z);
                        constexpr auto I = static_cast<InterpolationType>(IT::value);
                        constexpr auto X = static_cast<Wrapping>(XT::value);
                        constexpr auto Y = static_cast<Wrapping>(YT::value);
                        constexpr auto Z = static_cast<Wrapping>(ZT::value);
                        detail::sample<T, N, I, X, Y, Z>(data, state, pos, dst);
                    };
                });
            });
        });
    });

    const auto wrapping = volume.getWrapping();
    const auto interpolation = volume.getInterpolation();

    SampleState sstate{.dims = volume.getDimensions(),
                       .im = util::IndexMapper3D{volume.getDimensions()},
                       .posToTexture = state.posToData,
                       .textureToIndex = state.dataToIndex};

    const auto func = table[std::to_underlying(interpolation)][std::to_underlying(wrapping[0])]
                           [std::to_underlying(wrapping[1])][std::to_underlying(wrapping[2])];

    if (state.space == DataMapper::Space::Normalized) {
        return [func, data = volume.getView(), sstate, dm = state.dataMap](
                   std::span<const dvec3> positions, std::span<Vector<N, double>> output) {
            func(data, sstate, positions, output);
            std::ranges::transform(output, output.begin(), [&](auto value) {
                return dm.mapFromDataTo<DataMapper::Space::Normalized>(value);
            });
        };
    } else if (state.space == DataMapper::Space::SignNormalized) {
        return [func, data = volume.getView(), sstate, dm = state.dataMap](
                   std::span<const dvec3> positions, std::span<Vector<N, double>> output) {
            func(data, sstate, positions, output);
            std::ranges::transform(output, output.begin(), [&](auto value) {
                return dm.mapFromDataTo<DataMapper::Space::SignNormalized>(value);
            });
        };
    } else if (state.space == DataMapper::Space::Value) {
        return [func, data = volume.getView(), sstate, dm = state.dataMap](
                   std::span<const dvec3> positions, std::span<Vector<N, double>> output) {
            func(data, sstate, positions, output);
            std::ranges::transform(output, output.begin(), [&](auto value) {
                return dm.mapFromDataTo<DataMapper::Space::Value>(value);
            });
        };
    } else {
        return [func, data = volume.getView(), sstate](std::span<const dvec3> positions,
                                                       std::span<Vector<N, double>> output) {
            func(data, sstate, positions, output);
        };
    }
}

inline State createState(const Volume& volume,
                         CoordinateSpace positionSpace = CoordinateSpace::World,
                         DataMapper::Space space = DataMapper::Space::Value) {
    const auto& cm = volume.getCoordinateTransformer();
    return {.volumeRAM = *volume.getRepresentation<VolumeRAM>(),
            .posToData = cm.getMatrix(positionSpace, CoordinateSpace::Data),
            .dataToIndex = cm.getDataToIndexMatrix(),
            .dataMap = volume.dataMap,
            .space = space};
}

/*
IVW_CORE_API void sample(const State& state, std::span<const dvec3> positions,
                         std::span<double> outpu);
IVW_CORE_API void sample(const State& state, std::span<const dvec3> positions,
                         std::span<dvec2> output);
IVW_CORE_API void sample(const State& state, std::span<const dvec3> positions,
                         std::span<dvec3> output);
IVW_CORE_API void sample(const State& state, std::span<const dvec3> positions,
                         std::span<dvec4> output);

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
*/

template <size_t N>
using SampleFunctor = std::function<void(std::span<const dvec3>, std::span<Vector<N, double>>)>;

IVW_CORE_API auto createVec1Functor(const Volume& volume,
                                    CoordinateSpace positionSpace = CoordinateSpace::World,
                                    DataMapper::Space space = DataMapper::Space::Value)
    -> SampleFunctor<1>;
IVW_CORE_API auto createVec2Functor(const Volume& volume,
                                    CoordinateSpace positionSpace = CoordinateSpace::World,
                                    DataMapper::Space space = DataMapper::Space::Value)
    -> SampleFunctor<2>;
IVW_CORE_API auto createVec3Functor(const Volume& volume,
                                    CoordinateSpace positionSpace = CoordinateSpace::World,
                                    DataMapper::Space space = DataMapper::Space::Value)
    -> SampleFunctor<3>;
IVW_CORE_API auto createVec4Functor(const Volume& volume,
                                    CoordinateSpace positionSpace = CoordinateSpace::World,
                                    DataMapper::Space space = DataMapper::Space::Value)
    -> SampleFunctor<4>;

}  // namespace inviwo::sample
