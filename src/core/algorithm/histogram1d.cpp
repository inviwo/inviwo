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

#include <inviwo/core/algorithm/histogram1d.h>
#include <algorithm>
#include <numeric>

namespace inviwo::util {

std::vector<double> calculatePercentiles(const std::vector<size_t>& hist, dvec2 range,
                                         const size_t sum) {
    size_t i{0};
    size_t accumulation{0};
    std::vector<double> percentiles(101, 0.0);

    const double binSize = 1.0 / static_cast<double>(hist.size() - 1) * (range.y - range.x);
    const auto dSum = static_cast<double>(sum);

    for (size_t j = 0; j < hist.size(); ++j) {
        accumulation += hist[j];
        while (static_cast<double>(accumulation) / dSum >= static_cast<double>(i) / 100.0) {
            percentiles[i] = static_cast<double>(j) * binSize + range.x;
            i++;
        }
    }
    return percentiles;
}

Statistics calculateHistogramStats(const std::vector<size_t>& hist) {
    std::vector<size_t> sorted = hist;
    std::sort(sorted.begin(), sorted.end());
    const size_t sum = std::accumulate(hist.begin(), hist.end(), size_t{0});
    const size_t sum2 =
        std::accumulate(hist.begin(), hist.end(), 0, [](size_t a, size_t b) { return a + b * b; });

    std::vector<double> percentiles(101, 0.0);
    for (size_t i = 1; i < percentiles.size(); ++i) {
        percentiles[i] = static_cast<double>(
            sorted.at(static_cast<size_t>(std::ceil(static_cast<double>(i) /
                                                    static_cast<double>(percentiles.size() - 1) *
                                                    static_cast<double>(hist.size()))) -
                      1));
    }

    return {.min = static_cast<double>(sorted.front()),
            .max = static_cast<double>(sorted.back()),
            .mean = static_cast<double>(sum) / static_cast<double>(hist.size()),
            .standardDeviation = std::sqrt(static_cast<double>((hist.size() * sum2 - sum * sum)) /
                                           static_cast<double>(hist.size() * (hist.size() - 1))),
            .percentiles = std::move(percentiles)};
}

}  // namespace inviwo::util
