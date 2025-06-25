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
#include <inviwo/core/datastructures/histogram.h>
#include <inviwo/core/algorithm/histogram1d.h>
#include <inviwo/core/util/glmcomp.h>
#include <inviwo/core/util/indexmapper.h>

#include <glm/gtx/component_wise.hpp>
#include <memory>
#include <span>
#include <ranges>

namespace inviwo::util {

namespace detail {

IVW_CORE_API DataMapper histogramDataMap(const DataMapper& datamap, double effectiveRange);

}  // namespace detail

/**
 * Calculate a 2D histogram and statistics for two given spans \p data1 and
 * \p data2 of type \p T and \p U, respectively. This function assumes that the data values between
 * the two spans are aligned, that is <tt>data1[n]</tt> corresponds to <tt>data2[n]</tt>. Note that
 * \p data1 and \p data2 must have the same dimensions. No interpolation is performed.
 *
 * @tparam T        underlying data type of the first dimension, can be a scalar or glm vector type
 * @tparam U        underlying data type of the second dimension, can be a scalar or glm vector type
 * @param data1     first dimension
 * @param channel1  channel of data1 used as first dimension
 * @param dataMap1  provides the data range used for bin positions and size of the first dimension
 * @param data2     second dimension
 * @param channel2  channel of data1 used as second dimension
 * @param dataMap2  provides the data range used for bin positions and size of the second dimension
 * @param bins      upper limit of bins to use, actual number of bins might be lower based on data
 *                  range and data types of \p T and \p U
 * @return 2D histogram of \p data1[channel1] and \p data2[channel2]
 * @throws Exception if the sizes of \p data1 and \p data2 do not match
 * \see util::detail::optimalBinCount
 */
template <typename T, typename U>
Histogram2D calculateHistogram2D(std::span<const T> data1, size_t channel1,
                                 const DataMapper& dataMap1, std::span<const U> data2,
                                 size_t channel2, const DataMapper& dataMap2, size2_t bins) {
    if (data1.size() != data2.size()) {
        throw Exception{SourceContext{}, "Dimensions must match ({} and {})", data1.size(),
                        data2.size()};
    }

    auto [numbins1, effectiveRange1] = detail::optimalBinCount<T>(dataMap1, bins.x);
    auto [numbins2, effectiveRange2] = detail::optimalBinCount<U>(dataMap2, bins.y);
    const dvec2 effectiveRange{effectiveRange1, effectiveRange2};
    const size2_t numbins{numbins1, numbins2};

    size_t underflow{0u};
    size_t overflow{0u};

    const dvec2 rangeMin{dataMap1.dataRange.x, dataMap2.dataRange.x};
    const dvec2 rangeScaleFactor{dvec2{numbins - size2_t{1}} / effectiveRange};

    std::vector<size_t> hist(glm::compMul(numbins), 0);
    const glm::vec<2, ptrdiff_t> maxBins{static_cast<ptrdiff_t>(numbins.x) - 1,
                                         static_cast<ptrdiff_t>(numbins.y) - 1};
    const IndexMapper<2, std::ptrdiff_t> indexMapper{numbins};

    // Do not use util::zip or std::views::zip here. This will increase compile time drastically.
    for (size_t i = 0; i < data1.size(); ++i) {
        const dvec2 val{static_cast<double>(util::glmcomp(data1[i], channel1)),
                        static_cast<double>(util::glmcomp(data2[i], channel2))};

        const glm::vec<2, ptrdiff_t> index{
            static_cast<std::ptrdiff_t>((val.x - rangeMin.x) * rangeScaleFactor.x),
            static_cast<std::ptrdiff_t>((val.y - rangeMin.y) * rangeScaleFactor.y)};
        if (index.x < 0 || index.y < 0) {
            ++underflow;
        } else if (index.x > maxBins.x || index.y > maxBins.y) {
            ++overflow;
        } else {
            ++hist[indexMapper(index)];
        }
    }

    const auto maxBinCount = *std::ranges::max_element(hist);

    return Histogram2D{
        .counts = hist,
        .dimensions = numbins,
        .totalCounts = data1.size(),
        .maxCount = maxBinCount,
        .dataMap = {detail::histogramDataMap(dataMap1, effectiveRange.x),
                    detail::histogramDataMap(dataMap2, effectiveRange.y)},
        .underflow = underflow,
        .overflow = overflow,
    };
}

}  // namespace inviwo::util
