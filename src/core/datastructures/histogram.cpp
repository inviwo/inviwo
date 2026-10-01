/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2026 Inviwo Foundation
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

#include <inviwo/core/datastructures/histogram.h>

namespace inviwo {

Document DataTraits<Histogram1D>::info(const Histogram1D& histogram) {

    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;
    Document doc;
    doc.append("b", fmt::format("Histogram 1D {}", histogram.name), {{"style", "color:white;"}});
    utildoc::TableBuilder tb(doc.handle(), P::end());

    tb(H("Stats"), fmt::format("Min: {}, Mean: {}, Max: {}, Std: {}", histogram.dataStats.min,
                               histogram.dataStats.mean, histogram.dataStats.max,
                               histogram.dataStats.standardDeviation));
    tb(H("Percentiles"),
       fmt::format("(1: {}, 25: {}, 50: {}, 75: {}, 99: {})", histogram.dataStats.percentiles[1],
                   histogram.dataStats.percentiles[25], histogram.dataStats.percentiles[50],
                   histogram.dataStats.percentiles[75], histogram.dataStats.percentiles[99]));

    return doc;
}

Document DataTraits<Histogram2D>::info(const Histogram2D& histogram) {

    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;
    Document doc;
    doc.append("b", fmt::format("Histogram 2D {}", histogram.name), {{"style", "color:white;"}});
    utildoc::TableBuilder tb(doc.handle(), P::end());

    for (const auto& stats : histogram.dataStats) {
        tb(H("Stats"), fmt::format("Min: {}, Mean: {}, Max: {}, Std: {}", stats.min, stats.mean,
                                   stats.max, stats.standardDeviation));
        tb(H("Percentiles"),
           fmt::format("(1: {}, 25: {}, 50: {}, 75: {}, 99: {})", stats.percentiles[1],
                       stats.percentiles[25], stats.percentiles[50], stats.percentiles[75],
                       stats.percentiles[99]));
    }

    return doc;
}

std::string format_as(const Statistics& stats) {
    const std::string percentiles =
        stats.percentiles.size() >= 100
            ? fmt::format("(1: {:.3g}, 25: {:.3g}, 50: {:.3g}, 75: {:.3g}, 99: {:.3g})",
                          stats.percentiles[1], stats.percentiles[25], stats.percentiles[50],
                          stats.percentiles[75], stats.percentiles[99])
            : fmt::format("{::.3g}", stats.percentiles);
    return fmt::format("min: {:.3g}, max: {:.3g}, mean: {:.3g}, stdDev: {:.3g}, percentiles: {}",
                       stats.min, stats.max, stats.mean, stats.standardDeviation, percentiles);
}

std::string format_as(const Histogram1D& stats) {
    return fmt::format(
        "{}, bins: {}, totalCounts: {}, maxCount: {}, "
        "underflow: {}, overflow: {},\n"
        "dataMap:   {},\n"
        "dataStats: {},\n"
        "histStats: {}",
        stats.name, stats.counts.size(), stats.totalCounts, stats.maxCount, stats.underflow,
        stats.overflow, stats.dataMap, stats.dataStats, stats.histStats);
}

std::string format_as(const Histogram2D& stats) {
    return fmt::format(
        "{}, bins: {} x {}, totalCounts: {}, maxCount: {}, underflow: {}, "
        "overflow: {},\n"
        "dataMap   1: {},\n"
        "dataStats 1: {},\n"
        "histStats 1: {},\n"
        "dataMap   2: {},\n"
        "dataStats 2: {},\n"
        "histStats 2: {}",
        stats.name, stats.dimensions.x, stats.dimensions.y, stats.totalCounts, stats.maxCount,
        stats.underflow, stats.overflow, stats.dataMap[0], stats.dataStats[0], stats.histStats[0],
        stats.dataMap[1], stats.dataStats[1], stats.histStats[1]);
}

}  // namespace inviwo
