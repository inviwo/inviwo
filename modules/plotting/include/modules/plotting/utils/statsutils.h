/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2023 Inviwo Foundation
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

#include <modules/plotting/plottingmoduledefine.h>  // for IVW_MODULE_PLOTTING_API

#include <inviwo/core/util/exception.h>      // for Exception
#include <inviwo/core/util/glm.h>            // for isnan
#include <inviwo/core/util/glmutils.h>       // for is_floating_point
#include <inviwo/core/util/sourcecontext.h>  // for IVW_CONTEXT_CUSTOM

#include <algorithm>    // for max, sort, partition
#include <cmath>        // for ceil
#include <cstddef>      // for size_t
#include <iosfwd>       // for ostream
#include <iterator>     // for distance
#include <stdexcept>    // for invalid_argument
#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for enable_if
#include <vector>       // for vector

#include <glm/common.hpp>

namespace inviwo {
class BufferBase;

namespace statsutil {
struct RegresionResult {
    double k;  /// y = kx + m
    double m;  /// y = kx + m
    double r2;
    double corr;
};

IVW_MODULE_PLOTTING_API RegresionResult linearRegresion(const BufferBase& X, const BufferBase& Y);

IVW_MODULE_PLOTTING_API std::ostream& operator<<(std::ostream& os, RegresionResult res);

/**
 * \brief Compute value below a percentage of observations in the data.
 * Uses the nearest rank method, i.e. ceil(percentile * N), where N = number of elements in data.
 *
 * NaNs (Not a Numbers) are excluded from the computation.
 * The following example will return {1,2}
 * \code{.cpp}
 *    auto percentiles = utilstats::percentiles({1, 0, 3, 2}, {0.25, 0.75});
 * \endcode
 * See also https://en.wikipedia.org/wiki/Percentile
 *
 * @param data to compute percentiles on
 * @param percentiles in the range [0 1]
 * @return values below the percentage given by the percentiles.
 * @throw Exception if any percentile is less than 0 or larger than 1
 */
template <typename T, typename std::enable_if<!util::is_floating_point<T>::value, int>::type = 0>
std::vector<T> percentiles(std::vector<T> data, const std::vector<double>& percentiles) {
    std::sort(data.begin(), data.end());
    std::vector<T> result;
    result.reserve(percentiles.size());
    auto nElements = data.size();
    for (auto percentile : percentiles) {
        if (percentile < 0.f || percentile > 1.f) {
            throw Exception("Percentile must be between 0 and 1",
                            IVW_CONTEXT_CUSTOM("statsutil::percentiles"));
        }
        // Take care of percentile == 1 using std::min
        result.push_back(
            data[static_cast<size_t>(std::max(std::ceil(nElements * percentile) - 1., 0.))]);
    }
    return result;
}

// Float/double types have special values
template <typename T, typename std::enable_if<util::is_floating_point<T>::value, int>::type = 0>
std::vector<T> percentiles(std::vector<T> data, const std::vector<double>& percentiles) {
    auto noNaN =
        std::partition(data.begin(), data.end(), [](const auto& a) { return glm::isnan(a); });
    std::sort(noNaN, data.end());
    std::vector<T> result;
    result.reserve(percentiles.size());
    size_t nElements = std::distance(noNaN, data.end());
    for (auto percentile : percentiles) {
        if (percentile < 0.f || percentile > 1.f) {
            throw std::invalid_argument("Percentile must be between 0 and 1");
        }
        // Take care of percentile == 1 using std::min
        result.push_back(
            data[static_cast<size_t>(std::max(std::ceil(nElements * percentile) - 1., 0.))]);
    }
    return result;
}

}  // namespace statsutil

}  // namespace inviwo
