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

#include <glm/common.hpp>

#include <iterator>
#include <vector>
#include <bitset>
#include <array>

namespace inviwo {
enum class HistogramMode { Off, All, P99, P95, P90, Log };

using HistogramSelection = std::bitset<32>;
constexpr HistogramSelection histogramSelectionAll{0xffffffff};

/**
 *The NormalizedHistogram has a array of bins and all bins are normalized.
 *It can be de-normalized using the maxiumBinValue_.
 */
class IVW_CORE_API NormalizedHistogram {
public:
    struct Stats {
        double min;
        double max;
        double mean;
        double standardDeviation;
        std::vector<double> percentiles;
    };

    NormalizedHistogram();

    NormalizedHistogram(dvec2 dataRange, std::vector<double> counts, double min, double max,
                        double mean, double standardDeviation);

    std::vector<double>& getData();
    const std::vector<double>& getData() const;
    double& operator[](size_t i);
    const double& operator[](size_t i) const;

    double getMaximumBinValue() const;

    Stats stats_;
    Stats histStats_;
    dvec2 dataRange_;

protected:
    std::vector<double> data_;
    double maximumBinCount_;
};

class IVW_CORE_API HistogramContainer {
public:
    HistogramContainer() = default;
    template <typename FirstIter, typename LastIter>
    HistogramContainer(dvec2 range, size_t bins, FirstIter begin, LastIter end);

    const NormalizedHistogram& operator[](size_t i) const;
    const NormalizedHistogram& get(size_t i) const;

    NormalizedHistogram& operator[](size_t i);
    NormalizedHistogram& get(size_t i);

    size_t size() const;
    bool empty() const;

    void clear();

private:
    std::vector<NormalizedHistogram> histograms_;
};

template <typename FirstIter, typename LastIter>
HistogramContainer::HistogramContainer(dvec2 dataRange, size_t bins, FirstIter begin,
                                       LastIter end) {
    using T = typename std::iterator_traits<FirstIter>::value_type;

    // a double type with the same extent as T
    using D = typename util::same_extent<T, double>::type;
    // a size_t type with same extent as T
    using I = typename util::same_extent<T, size_t>::type;

    constexpr size_t extent = util::rank<T>::value > 0 ? util::extent<T>::value : 1;

    // check whether number of bins exceeds the data range only if it is an integral type
    if constexpr (!util::is_floating_point<typename util::value_type<T>::type>::value) {
        bins = std::min(bins, static_cast<std::size_t>(dataRange.y - dataRange.x + 1));
    }

    std::array<std::vector<double>, extent> histData;
    for (size_t i = 0; i < extent; ++i) {
        histData[i].resize(bins, 0.0);
    }

    D min(std::numeric_limits<double>::max());
    D max(std::numeric_limits<double>::lowest());
    D sum(0);
    D sum2(0);
    size_t count(0);

    const D rangeMin(dataRange.x);
    const D rangeScaleFactor(static_cast<double>(bins - 1) / (dataRange.y - dataRange.x));

    for (; begin != end; ++begin) {

        const auto val = static_cast<D>(*begin);

        min = glm::min(min, val);
        max = glm::max(max, val);
        sum += val;
        sum2 += val * val;
        count++;

        const auto ind = static_cast<I>(glm::clamp((val - rangeMin) * rangeScaleFactor, D{0.0},
                                                   D{static_cast<double>(bins - 1)}));
        for (size_t i = 0; i < extent; ++i) {
            const auto v = util::glmcomp(ind, i);
            ++histData[i][v];
        }
    }

    const auto dcount = static_cast<double>(count);
    const auto mean = sum / dcount;
    const auto stddev = glm::sqrt((dcount * sum2 - sum * sum) / (dcount * (dcount - D{1})));

    for (size_t i = 0; i < extent; ++i) {
        histograms_.emplace_back(dataRange, std::move(histData[i]), util::glmcomp(min, i),
                                 util::glmcomp(max, i), util::glmcomp(mean, i),
                                 util::glmcomp(stddev, i));
    }
}

}  // namespace inviwo
