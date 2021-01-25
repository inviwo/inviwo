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

#include <inviwo/propertybasedtesting/propertybasedtestingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <random>

namespace inviwo {

// randomly select k elements out of n elements (indexed from 0 to n-1)
// note: if k > n, then all n elements are chosen
// Time complexity: O(k * (1+log(n/k)))
template<typename RNG>
std::vector<size_t> reservoirSampling(RNG&, const size_t n, const size_t k);

template<typename RNG>
std::vector<size_t> reservoirSampling(RNG& rng, const size_t n, const size_t k) {
    if (k > n) return reservoirSampling(rng, n, n);
	std::uniform_real_distribution<double> distribution(0.0,1.0);

    std::vector<size_t> result(k);
    std::iota(result.begin(), result.end(), 0);  // start with the first k values

    double w = exp(log(distribution(rng)) / k);

    for (size_t i = k; i < n;) {
        i += log(distribution(rng)) / log(1 - w);
        if (i < n) {
            result[std::uniform_int_distribution<size_t>(0, k-1)(rng)] = i;
            w *= exp(log(distribution(rng)) / k);
        }
    }

    return result;
}

}  // namespace inviwo
