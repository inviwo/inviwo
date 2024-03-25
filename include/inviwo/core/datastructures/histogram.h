/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/glmutils.h>
#include <inviwo/core/util/glmcomp.h>
#include <inviwo/core/util/glmmatext.h>
#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/util/dispatcher.h>

#include <glm/common.hpp>

#include <iterator>
#include <vector>
#include <bitset>
#include <array>
#include <span>

namespace inviwo {
enum class HistogramMode : int { Off = 0, All, P99, P95, P90, Log };
constexpr size_t numberOfHistogramModes = 6;

using HistogramSelection = std::bitset<32>;
constexpr HistogramSelection histogramSelectionAll{0xffffffff};

struct IVW_CORE_API Statistics {
    double min;
    double max;
    double mean;
    double standardDeviation;
    std::vector<double> percentiles;
};

struct IVW_CORE_API Histogram1D {
    std::vector<size_t> counts;
    size_t totalCounts{0};
    size_t maxCount{0};
    DataMapper dataMap{};
    size_t underflow{0};
    size_t overflow{0};

    Statistics dataStats;
    Statistics histStats;
};

namespace util {

IVW_CORE_API std::vector<double> calculatePercentiles(const std::vector<size_t> hist, dvec2 range,
                                                      size_t sum);
IVW_CORE_API Statistics calculateHistogramStats(const std::vector<size_t>& hist);

template <typename T>
std::vector<Histogram1D> calculateHistograms(std::span<const T> data, const DataMapper& dataMap,
                                             size_t bins) {
    // a double type with the same extent as T
    using D = typename util::same_extent<T, double>::type;
    // a ptrdiff_t type with same extent as T
    using I = typename util::same_extent<T, std::ptrdiff_t>::type;

    constexpr size_t extent = util::rank<T>::value > 0 ? util::extent<T>::value : 1;

    // check whether number of bins exceeds the data range only if it is an integral type
    if constexpr (!std::is_floating_point_v<util::value_type_t<T>>) {
        bins =
            std::min(bins, static_cast<std::size_t>(dataMap.dataRange.y - dataMap.dataRange.x + 1));
    }

    std::array<std::vector<size_t>, extent> hists;
    for (size_t i = 0; i < extent; ++i) {
        hists[i].resize(bins, 0);
    }

    D min(std::numeric_limits<double>::max());
    D max(std::numeric_limits<double>::lowest());
    D sum(0);
    D sum2(0);
    size_t count(0);

    std::array<size_t, extent> underflow{0};
    std::array<size_t, extent> overflow{0};

    const D rangeMin(dataMap.dataRange.x);
    const D rangeScaleFactor(static_cast<double>(bins - 1) /
                             (dataMap.dataRange.y - dataMap.dataRange.x));

    const std::ptrdiff_t maxBin = bins - 1;

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

    for (size_t channel = 0; channel < extent; ++channel) {
        const auto maxBinCount = *std::max_element(hists[channel].begin(), hists[channel].end());

        histograms.push_back(Histogram1D{
            .counts = hists[channel],
            .totalCounts = count,
            .maxCount = maxBinCount,
            .dataMap = dataMap,
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
