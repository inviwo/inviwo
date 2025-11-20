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

#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/glmvec.h>

#include <functional>

namespace inviwo::grid {

namespace detail {

template <Wrapping Wx, Wrapping Wy, typename Callable, typename... Args>
inline constexpr void wrapper2(const std::array<Wrapping, 3>& w, const Callable& obj,
                               Args&&... args) {
    switch (w[2]) {
        default:
            [[fallthrough]];
        case Wrapping::Clamp:
            return obj.template operator()<Wx, Wy, Wrapping::Clamp>(std::forward<Args>(args)...);
        case Wrapping::Repeat:
            return obj.template operator()<Wx, Wy, Wrapping::Repeat>(std::forward<Args>(args)...);
        case Wrapping::Mirror:
            return obj.template operator()<Wx, Wy, Wrapping::Mirror>(std::forward<Args>(args)...);
    }
}

template <Wrapping Wx, typename Callable, typename... Args>
inline constexpr void wrapper1(const std::array<Wrapping, 3>& w, const Callable& obj,
                               Args&&... args) {
    switch (w[1]) {
        default:
            [[fallthrough]];
        case Wrapping::Clamp:
            return wrapper2<Wx, Wrapping::Clamp>(w, obj, std::forward<Args>(args)...);
        case Wrapping::Repeat:
            return wrapper2<Wx, Wrapping::Repeat>(w, obj, std::forward<Args>(args)...);
        case Wrapping::Mirror:
            return wrapper2<Wx, Wrapping::Mirror>(w, obj, std::forward<Args>(args)...);
    }
}
}  // namespace detail

template <typename Callable, typename... Args>
inline constexpr void wrapper(const std::array<Wrapping, 3>& w, const Callable& obj,
                              Args&&... args) {
    switch (w[0]) {
        default:
            [[fallthrough]];
        case Wrapping::Clamp:
            return detail::wrapper1<Wrapping::Clamp>(w, obj, std::forward<Args>(args)...);
        case Wrapping::Repeat:
            return detail::wrapper1<Wrapping::Repeat>(w, obj, std::forward<Args>(args)...);
        case Wrapping::Mirror:
            return detail::wrapper1<Wrapping::Mirror>(w, obj, std::forward<Args>(args)...);
    }
}

enum class Part : std::uint8_t { First, Mid, Last };

template <typename Func>
void loop(size_t N, Func func) {
    func.template operator()<Part::First>(size_t{0});
    for (size_t i = 1; i < N - 1; ++i) {
        func.template operator()<Part::Mid>(i);
    }
    func.template operator()<Part::Last>(N - 1);
}

template <typename Func>
void loop(size3_t dims, Func func, const std::function<void(double)>& progress = nullptr,
          const std::function<bool()>& stop = nullptr) {

    loop(dims.z, [&]<Part Pz>(size_t z) {
        if (stop && stop()) return;
        if (progress) progress(static_cast<double>(z) / static_cast<double>(dims.z));
        loop(dims.y, [&]<Part Py>(size_t y) {
            loop(dims.x, [&]<Part Px>(size_t x) { func.template operator()<Px, Py, Pz>(x, y, z); });
        });
    });
}

template <Part part, Wrapping wrap>
size_t prev(size_t i, size_t N) {
    if constexpr (part == Part::First) {
        if constexpr (wrap == Wrapping::Clamp) {
            return size_t{0};
        } else if constexpr (wrap == Wrapping::Repeat) {
            return N - 1;
        } else {  // Wrapping::Mirror
            return size_t{1};
        }
    } else if constexpr (part == Part::Mid) {
        return i - 1;
    } else {
        return N - 2;
    }
}
template <Part part, Wrapping wrap>
size_t next(size_t i, size_t N) {
    if constexpr (part == Part::First) {
        return size_t{1};
    } else if constexpr (part == Part::Mid) {
        return i + 1;
    } else {
        if constexpr (wrap == Wrapping::Clamp) {
            return N - 1;
        } else if constexpr (wrap == Wrapping::Repeat) {
            return size_t{1};
        } else {
            return N - 2;
        }
    }
}

template <Part P, Wrapping W>
double invStep() {
    if constexpr (W == Wrapping::Clamp && P != Part::Mid) {
        return 1.0;
    } else {
        return 0.5;
    }
}

template <typename Func>
void centralDifferences(size3_t dims, const std::array<Wrapping, 3>& w, Func func,
                        const std::function<void(double)>& progress = nullptr,
                        const std::function<bool()>& stop = nullptr) {

    std::array<size3_t, 6> positions{};
    dvec3 reciprocalSampleDist{};

    loop(dims.z, [&]<Part Pz>(size_t z) {
        if (stop && stop()) return;
        if (progress) progress(static_cast<double>(z) / static_cast<double>(dims.z));
        loop(dims.y, [&]<Part Py>(size_t y) {
            loop(dims.x, [&]<Part Px>(size_t x) {
                wrapper(w, [&]<Wrapping Wx, Wrapping Wy, Wrapping Wz>() {
                    positions = std::array<size3_t, 6>{{{next<Px, Wx>(x, dims.x), y, z},
                                                        {prev<Px, Wx>(x, dims.x), y, z},
                                                        {x, next<Py, Wy>(y, dims.y), z},
                                                        {x, prev<Py, Wy>(y, dims.y), z},
                                                        {x, y, next<Pz, Wz>(z, dims.z)},
                                                        {x, y, prev<Pz, Wz>(z, dims.z)}}};

                    reciprocalSampleDist = {invStep<Px, Wx>(), invStep<Py, Wy>(),
                                            invStep<Pz, Wz>()};
                });

                func(size3_t{x, y, z}, positions, reciprocalSampleDist);
            });
        });
    });
}

namespace detail {
template <template <class> class Predicate, size_t I>
constexpr bool pred(DataFormatId format) {
    return static_cast<size_t>(format) == I + 1 &&
           Predicate<DataFormat<std::tuple_element_t<I, DefaultDataTypes>>>::value;
};

template <size_t I, template <class> class Predicate, typename Callable, typename... Args>
void call(const Callable& obj, Args&&... args) {
    if constexpr (Predicate<DataFormat<std::tuple_element_t<I, DefaultDataTypes>>>::value) {
        using T = std::tuple_element_t<I, DefaultDataTypes>;
        obj.template operator()<T>(std::forward<Args>(args)...);
    }
}
}  // namespace detail

template <template <class> class Predicate, typename Callable, typename... Args>
inline void dispatch(DataFormatId format, const Callable& obj, Args&&... args) {
    return [&]<size_t... Is>(std::integer_sequence<size_t, Is...>) {
        const bool found =
            ((detail::pred<Predicate, Is>(format)
                  ? (detail::call<Is, Predicate>(obj, std::forward<Args>(args)...), true)
                  : false) ||
             ...);
        if (!found) throw std::runtime_error("Format not specialized");
    }
    (std::make_integer_sequence<size_t, std::tuple_size_v<DefaultDataTypes>>{});
}

}  // namespace inviwo::grid
