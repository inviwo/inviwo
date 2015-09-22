/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

/**
 * \class Interpolation
 *
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 *
 * DESCRIBE_THE_CLASS
 */
class IVW_CORE_API Interpolation {
public:
    template <typename T>
    static T linear(const T &a, const T &b, double x) {
        return static_cast<T>(a + (b - a) * x);
    }

    template <typename T>
    static T linear(const T samples[2], double interpolant) {
        return linear(samples[0], samples[1], interpolant);
    }

    template <typename T>
    static T bilinear(const T &a, const T &b, const T &c, const T &d, const dvec2 &interpolants) {
        return linear(linear(a, b, interpolants.x), linear(c, d, interpolants.x), interpolants.y);
    }

    template <typename T>
    static T bilinear(const T samples[4], const dvec2 &interpolants) {
        return bilinear(samples[0], samples[1], samples[2], samples[3], interpolants);
    }

    template <typename T>
    static T trilinear(const T &a, const T &b, const T &c, const T &d, const T &e, const T &f,
        const T &g, const T &h, const dvec3 &interpolants) {
        return linear(bilinear(a, b, c, d, interpolants.xy()), bilinear(e, f, g, h, interpolants.xy()), interpolants.z);
    }

    template <typename T>
    static T trilinear(const T samples[8], const dvec3 &interpolants) {
        return trilinear(samples[0], samples[1], samples[2], samples[3]
            , samples[4], samples[5], samples[6], samples[7], interpolants);
    }

    template <typename T>
    static T quadlinear(const T &a, const T &b, const T &c, const T &d, const T &e, const T &f,
        const T &g, const T &h, const T &i, const T &j, const T &k, const T &l, const T &m, const T &n,
        const T &o, const T &p, const dvec4 &interpolants) {
        return linear(trilinear(a, b, c, d, e, f, g, h, interpolants.xyz()), trilinear(i,j,k,l,m,n,o,p, interpolants.xyz()), interpolants.w);
    }

    template <typename T>
    static T trilinear(const T samples[16], const dvec4 &interpolants) {
        return trilinear(samples[0], samples[1], samples[2], samples[3], samples[4], samples[5],
            samples[6], samples[7], samples[8], samples[9], samples[10], samples[11],
            samples[12], samples[13], samples[14], samples[15], interpolants);
    }

private:
    Interpolation() {}
    virtual ~Interpolation() {}
};

}  // namespace

#endif  // IVW_INTERPOLATION_H
