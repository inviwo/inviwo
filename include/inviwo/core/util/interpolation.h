/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#ifndef IVW_INTERPOLATION_H
#define IVW_INTERPOLATION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

template <typename T, typename P = double>
class Interpolation {
     static_assert(std::is_floating_point_v<P>, "P needs to be a floating point type");
public:
    Interpolation() = delete;

    static inline T linear(const T &a, const T &b, P x);

    static inline T linearVector(const T &a, const T &b, P x);

    static inline T bilinear(const T &a, const T &b, const T &c, const T &d,
                             const Vector<2, P> &interpolants);

    static inline T bilinear(const T samples[4], const Vector<2, P> &interpolants);

    static inline T trilinear(const T &a, const T &b, const T &c, const T &d, const T &e,
                              const T &f, const T &g, const T &h, const Vector<3, P> &interpolants);

    static inline T trilinear(const T samples[8], const Vector<3, P> &interpolants);

    static inline T quadlinear(const T &a, const T &b, const T &c, const T &d, const T &e,
                               const T &f, const T &g, const T &h, const T &i, const T &j,
                               const T &k, const T &l, const T &m, const T &n, const T &o,
                               const T &p, const Vector<4, P> &interpolants);

    static inline T quadlinear(const T samples[16], const Vector<4, P> &interpolants);
};

namespace {
template <typename T, typename P, typename std::enable_if<util::rank<T>::value == 0, int>::type = 0>
inline T linearVectorInterpolation(const T &a, const T &b, P x) {
    return Interpolation<T, P>::linear(a, b, x);
}

template <typename T, typename P, typename std::enable_if<util::rank<T>::value == 1, int>::type = 0>
inline T linearVectorInterpolation(const T &a, const T &b, P x) {
    auto la = glm::length(a);
    auto lb = glm::length(b);
    auto l = Interpolation<P, P>::linear(la, lb, x);
    auto v = Interpolation<T, P>::linear(a, b, x);
    auto lOut = glm::length(v);
    if (lOut == 0) {
        return v;
    }
    return v * (l / lOut);
}
}  // namespace

template <typename T, typename P>
inline T Interpolation<T, P>::linear(const T &a, const T &b, P x) {
    using VT = typename util::value_type<T>::type;
    if constexpr (std::is_same_v<T, VT> || std::is_signed_v<VT> ) {
        // GLM fails at correctly linearly interpolate between vectors of unsigned types when the
        // difference is negative
        return glm::mix(a, b, x);
    } else {
        using V = typename util::same_extent<T, double>::type;
        return util::glm_convert<T>(glm::mix(util::glm_convert<V>(a), util::glm_convert<V>(b), x));
    }
}

template <typename T, typename P>
inline T Interpolation<T, P>::linearVector(const T &a, const T &b, P x) {
    return linearVectorInterpolation<T, P>(a, b, x);
}

template <typename T, typename P>
inline T Interpolation<T, P>::bilinear(const T &a, const T &b, const T &c, const T &d,
                                       const Vector<2, P> &interpolants) {
    return linear(linear(a, b, interpolants.x), linear(c, d, interpolants.x), interpolants.y);
}

template <typename T, typename P>
inline T Interpolation<T, P>::bilinear(const T samples[4], const Vector<2, P> &interpolants) {
    return bilinear(samples[0], samples[1], samples[2], samples[3], interpolants);
}

template <typename T, typename P>
inline T Interpolation<T, P>::trilinear(const T &a, const T &b, const T &c, const T &d, const T &e,
                                        const T &f, const T &g, const T &h,
                                        const Vector<3, P> &interpolants) {
    return linear(bilinear(a, b, c, d, Vector<2, P>(interpolants)),
                  bilinear(e, f, g, h, Vector<2, P>(interpolants)), interpolants.z);
}

template <typename T, typename P>
inline T Interpolation<T, P>::trilinear(const T samples[8], const Vector<3, P> &interpolants) {
    return trilinear(samples[0], samples[1], samples[2], samples[3], samples[4], samples[5],
                     samples[6], samples[7], interpolants);
}

template <typename T, typename P>
inline T Interpolation<T, P>::quadlinear(const T &a, const T &b, const T &c, const T &d, const T &e,
                                         const T &f, const T &g, const T &h, const T &i, const T &j,
                                         const T &k, const T &l, const T &m, const T &n, const T &o,
                                         const T &p, const Vector<4, P> &interpolants) {
    return linear(trilinear(a, b, c, d, e, f, g, h, Vector<3, P>(interpolants)),
                  trilinear(i, j, k, l, m, n, o, p, Vector<3, P>(interpolants)), interpolants.w);
}

template <typename T, typename P>
inline T Interpolation<T, P>::quadlinear(const T samples[16], const Vector<4, P> &interpolants) {
    return quadlinear(samples[0], samples[1], samples[2], samples[3], samples[4], samples[5],
                      samples[6], samples[7], samples[8], samples[9], samples[10], samples[11],
                      samples[12], samples[13], samples[14], samples[15], interpolants);
}

}  // namespace inviwo

#endif  // IVW_INTERPOLATION_H
