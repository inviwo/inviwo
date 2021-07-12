/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <inviwo/core/util/glm.h>

#include <bitset>

namespace inviwo {
namespace discretedata {
namespace dd_util {

static double EPSILON_ABS = 0.0001;

template <typename T, size_t N>
double arrDotProduct(const std::array<T, N>& a, const std::array<T, N>& b) {
    double prod = 0;
    for (size_t n = 0; n < N; ++n) prod += a[n] * b[n];
    return prod;
}

template <typename T, size_t N>
std::array<T, N> arrMinus(const std::array<T, N>& a, const std::array<T, N>& b) {
    std::array<T, N> res;
    for (size_t n = 0; n < N; ++n) res[n] = a[n] - b[n];
    return res;
}

template <typename T, size_t N>
bool arrParallel(const std::array<T, N>& a, const std::array<T, N>& b, double eps = EPSILON_ABS) {
    double div = 0;
    for (size_t n = 0; n < N; ++n) {
        if (b[n] != 0.0) {
            if (a[n] == 0.0) return false;
            div = double(b[n]) / a[n];
            break;
        }
    }
    if (div == 0) return false;
    for (size_t n = 0; n < N; ++n)
        if (std::abs((a[n] * div - b[n]) > eps)) return false;
    return true;
}

}  // namespace dd_util
}  // namespace discretedata
}  // namespace inviwo