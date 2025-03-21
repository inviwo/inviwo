/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2025 Inviwo Foundation
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

#include <inviwo/volume/processors/histogramtodataframe.h>
#include <inviwo/core/util/zip.h>

#include <ranges>
#include <fmt/core.h>

namespace inviwo::detail {

std::vector<double> scaleHistogram(const Histogram1D& hist, HistogramMode mode) {
    std::vector<double> result(hist.counts.size());

    if (mode == HistogramMode::Log) {
        std::ranges::transform(hist.counts, result.begin(),
                               [](auto& count) { return std::log10(1.0 + count); });
    } else {
        const double percentile = [&]() {
            switch (mode) {
                case HistogramMode::All:  // show all
                    return hist.histStats.percentiles[100];
                case HistogramMode::P99:  // show 99%
                    return hist.histStats.percentiles[99];
                case HistogramMode::P95:  // show 95%
                    return hist.histStats.percentiles[95];
                case HistogramMode::P90:  // show 90%
                    return hist.histStats.percentiles[90];
                default:
                    return hist.histStats.percentiles[100];
            }
        }();
        std::ranges::transform(hist.counts, result.begin(), [percentile](auto& count) {
            return std::min<double>(static_cast<double>(count), percentile);
        });
    }
    return result;
}

std::shared_ptr<DataFrame> createDataFrame(const std::vector<Histogram1D>& histograms,
                                           HistogramMode mode) {
    auto dataframe = std::make_shared<DataFrame>();

    if (histograms.empty()) {
        return dataframe;
    }

    const size_t bins = histograms[0].counts.size();

    auto generateBinCenters = [bins](const dvec2& range) {
        const double binSize = (range.y - range.x) / static_cast<double>(bins - 1);
        const double rangeMin = range.x + binSize * 0.5;

        std::vector<double> binCenters(bins);
        std::ranges::generate(binCenters, [current = rangeMin, binSize]() mutable {
            const double v = current;
            current += binSize;
            return v;
        });
        return binCenters;
    };

    const std::string colName = !histograms[0].dataMap.valueAxis.name.empty()
                                    ? histograms[0].dataMap.valueAxis.name
                                    : "Scalars";
    dataframe->addColumn(colName, generateBinCenters(histograms[0].dataMap.valueRange),
                         histograms[0].dataMap.valueAxis.unit, histograms[0].dataMap.valueRange);

    for (auto&& [index, hist] : util::enumerate<int>(histograms)) {
        dataframe->addColumn(fmt::format("Channel{}", index), scaleHistogram(hist, mode));
    }
    dataframe->updateIndexBuffer();

    return dataframe;
}

}  // namespace inviwo::detail
