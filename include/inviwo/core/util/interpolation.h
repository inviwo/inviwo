/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2026 Inviwo Foundation
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
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/detected.h>

#include <span>

namespace inviwo {

namespace interpolate {

template <typename T, typename P = double>
constexpr T linear(std::span<const T, 2> samples, const P& interpolant) {
    return glm::mix(samples[0], samples[1], interpolant);
}

template <typename T, typename P = double>
constexpr T bilinear(std::span<const T, 4> samples, const glm::vec<2, P>& interpolants) {
    const std::array<T, 2> b{linear<T, P>(samples.template subspan<0, 2>(), interpolants.x),
                             linear<T, P>(samples.template subspan<2, 2>(), interpolants.x)};
    return linear<T, P>(b, interpolants.y);
}

template <typename T, typename P = double>
constexpr T trilinear(std::span<const T, 8> samples, const glm::vec<3, P>& interpolants) {
    const std::array<T, 2> b{
        bilinear<T, P>(samples.template subspan<0, 4>(), glm::vec<2, P>(interpolants)),
        bilinear<T, P>(samples.template subspan<4, 4>(), glm::vec<2, P>(interpolants))};
    return linear<T, P>(b, interpolants.z);
}

template <typename T, typename P = double>
constexpr T quadlinear(std::span<const T, 16> samples, const glm::vec<4, P>& interpolants) {
    const std::array<T, 2> b{
        trilinear<T, P>(samples.template subspan<0, 8>(), glm::vec<3, P>(interpolants)),
        trilinear<T, P>(samples.template subspan<4, 8>(), glm::vec<3, P>(interpolants))};
    return linear<T, P>(b, interpolants.w);
}

}  // namespace interpolate

template <typename T, typename P = double>
class Interpolation {
public:
    Interpolation() = delete;

    static inline T linear(const T& a, const T& b, P x);

    static inline T linearVector(const T& a, const T& b, P x);

    static inline T linear(std::span<const T, 2> samples, const Vector<1, P>& interpolants);

    static inline T bilinear(std::span<const T, 4> samples, const Vector<2, P>& interpolants);

    static inline T trilinear(std::span<const T, 8> samples, const Vector<3, P>& interpolants);

    static inline T quadlinear(std::span<const T, 16> samples, const Vector<4, P>& interpolants);
};

namespace detail {

template <typename T, typename P>
inline T linearVectorInterpolation(const T& a, const T& b, P x) {
    if constexpr (util::rank_v<T> == 0) {
        return Interpolation<T, P>::linear(a, b, x);
    } else if constexpr (util::rank_v<T> == 1) {
        auto la = glm::length(a);
        auto lb = glm::length(b);
        auto l = Interpolation<P, P>::linear(std::array{la, lb}, x);
        auto v = Interpolation<T, P>::linear(std::array{a, b}, x);
        auto lOut = glm::length(v);
        if (lOut == 0) {
            return v;
        }
        return v * (l / lOut);
    } else {
        static_assert(util::alwaysFalse<T>(), "Mat types not supported");
    }
}

}  // namespace detail

template <typename T, typename P>
inline T Interpolation<T, P>::linear(const T& a, const T& b, P x) {
    return glm::mix(a, b, x);
}

template <typename T, typename P>
inline T Interpolation<T, P>::linearVector(const T& a, const T& b, P x) {
    return detail::linearVectorInterpolation<T, P>(a, b, x);
}

template <typename T, typename P>
inline T Interpolation<T, P>::linear(std::span<const T, 2> samples,
                                     const Vector<1, P>& interpolants) {
    return glm::mix(samples[0], samples[1], interpolants);
}

template <typename T, typename P>
inline T Interpolation<T, P>::bilinear(std::span<const T, 4> samples,
                                       const Vector<2, P>& interpolants) {
    const std::array b{linear(samples.template subspan<0, 2>(), interpolants.x),
                       linear(samples.template subspan<2, 2>(), interpolants.x)};
    return linear(b, interpolants.y);
}

template <typename T, typename P>
inline T Interpolation<T, P>::trilinear(std::span<const T, 8> samples,
                                        const Vector<3, P>& interpolants) {
    const std::array b{bilinear(samples.template subspan<0, 4>(), Vector<2, P>(interpolants)),
                       bilinear(samples.template subspan<4, 4>(), Vector<2, P>(interpolants))};
    return linear(b, interpolants.z);
}

template <typename T, typename P>
inline T Interpolation<T, P>::quadlinear(std::span<const T, 16> samples,
                                         const Vector<4, P>& interpolants) {
    const std::array b{trilinear(samples.template subspan<0, 8>(), Vector<3, P>(interpolants)),
                       trilinear(samples.template subspan<4, 8>(), Vector<3, P>(interpolants))};
    return linear(b, interpolants.w);
}

}  // namespace inviwo
