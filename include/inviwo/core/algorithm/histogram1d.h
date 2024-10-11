/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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
#include <inviwo/core/util/glmutils.h>
#include <inviwo/core/util/glmcomp.h>
#include <inviwo/core/util/glmmatext.h>

#include <glm/common.hpp>

#include <ranges>
#include <vector>
#include <array>
#include <span>
#include <cmath>

namespace inviwo {

namespace util {

IVW_CORE_API std::vector<double> calculatePercentiles(const std::vector<size_t>& hist, dvec2 range,
                                                      size_t sum);
IVW_CORE_API Statistics calculateHistogramStats(const std::vector<size_t>& hist);

template <typename T>
std::vector<Histogram1D> calculateHistograms(std::span<const T> data, const DataMapper& dataMap,
                                             size_t bins) {
    // a double type with the same extent as T
    using D = typename util::same_extent<T, double>::type;
    // a size_t type with same extent as T
    using I = typename util::same_extent<T, ptrdiff_t>::type;

    constexpr size_t extent = util::rank<T>::value > 0 ? util::extent<T>::value : 1;

    auto intRangeBins = [&](size_t irange) -> std::pair<size_t, double> {
        const size_t m = (irange + bins) / bins;
        const auto numBins =
            static_cast<size_t>(std::ceil(static_cast<double>(irange) / static_cast<double>(m)));
        const auto effectiveRange = static_cast<double>((numBins - 1) * m);
        return {numBins, effectiveRange};
    };
    auto isNonFractional = [](double v) { return std::abs(v - std::round(v)) < 1.0e-12; };

    // determine optimal number of bins while considering \p bins as an upper bound
    // for the number of bins
    // 1. if there are more than 100 samples per bin based on data range, use \p bins without
    //    further modification
    // 2. for integral types:
    //    a. set bins to data range if the number of bins exceeds the data range
    //    b. otherwise adjust the number of bins so that the bin size is a whole number,
    //       i.e. \f$ n \in \mathbb{N} \f$
    // 3. for floating point types:
    //    a. if number of bins exceeds |data range|, use original \p bins count
    //    b. if the data range is non-fractional, i.e. whole numbers
    //       [min, max], adjust the number of bins so that the bin size is a whole number
    // 4. otherwise, use original \p bins count
    auto&& [numbins, effectiveRange] = [&]() -> std::pair<size_t, double> {
        const auto range = (dataMap.dataRange.y - dataMap.dataRange.x);
        if (range / static_cast<double>(bins) > 100.0) {  // case 1
            // sufficient samples per bin
            return {bins, range};
        }
        if constexpr (std::is_floating_point_v<util::value_type_t<T>>) {
            if (range < static_cast<double>(bins)) {  // case 3.a
                return {bins, range};
            }
            if (isNonFractional(dataMap.dataRange.x) && isNonFractional(dataMap.dataRange.y)) {
                // case 3.b
                const auto irange = static_cast<size_t>(std::round(dataMap.dataRange.y) -
                                                        std::round(dataMap.dataRange.x) + 1.0);
                return intRangeBins(irange);
            }
        } else {
            // check whether number of bins exceeds the data range only if it is an integral type
            const auto irange = static_cast<std::size_t>(range + 1);
            if (irange <= bins) {  // case 2.a
                return {irange, range};
            }
            return intRangeBins(irange);  // case 2.b
        }
        return {bins, range};
    }();

    D min(std::numeric_limits<double>::max());
    D max(std::numeric_limits<double>::lowest());
    D sum(0);
    D sum2(0);
    size_t count(0);

    std::array<size_t, extent> underflow{0};
    std::array<size_t, extent> overflow{0};

    const D rangeMin(dataMap.dataRange.x);
    const D rangeScaleFactor(static_cast<double>(numbins - 1) / effectiveRange);

    std::array<std::vector<size_t>, extent> hists;
    for (size_t i = 0; i < extent; ++i) {
        hists[i].resize(numbins, 0);
    }
    const ptrdiff_t maxBin = static_cast<ptrdiff_t>(numbins) - 1;

    for (const auto& item : data) {
        const auto val = static_cast<D>(item);

        min = glm::min(min, val);
        max = glm::max(max, val);
        sum += val;
        sum2 += val * val;
        count++;

        const auto ind = static_cast<I>((val - rangeMin) * rangeScaleFactor);
        for (size_t channel = 0; channel < extent; ++channel) {
            const auto v = util::glmcomp(ind, channel);
            if (v < 0) {
                ++underflow[channel];
            } else if (v > maxBin) {
                ++overflow[channel];
            } else {
                ++hists[channel][v];
            }
        }
    }

    const auto dcount = static_cast<double>(count);
    const auto mean = sum / dcount;
    const auto stddev = glm::sqrt((dcount * sum2 - sum * sum) / (dcount * (dcount - D{1})));

    std::vector<Histogram1D> histograms;

    const dvec2 effectiveDataRange{dataMap.dataRange.x, dataMap.dataRange.x + effectiveRange};
    const dvec2 effectiveValueRange{dataMap.valueRange.x,
                                    dataMap.mapFromDataToValue(effectiveDataRange.y)};

    for (size_t channel = 0; channel < extent; ++channel) {
        const auto maxBinCount = *std::ranges::max_element(hists[channel]);

        histograms.push_back(Histogram1D{
            .counts = hists[channel],
            .totalCounts = count,
            .maxCount = maxBinCount,
            .dataMap = dataMap,
            .effectiveDataRange = effectiveDataRange,
            .effectiveValueRange = effectiveValueRange,
            .underflow = underflow[channel],
            .overflow = overflow[channel],
            .dataStats = {.min = util::glmcomp(min, channel),
                          .max = util::glmcomp(max, channel),
                          .mean = util::glmcomp(mean, channel),
                          .standardDeviation = util::glmcomp(stddev, channel),
                          .percentiles =
                              calculatePercentiles(hists[channel], dataMap.dataRange, count)},
            .histStats = calculateHistogramStats(hists[channel]),
        });
    }

    return histograms;
}

}  // namespace util

}  // namespace inviwo
