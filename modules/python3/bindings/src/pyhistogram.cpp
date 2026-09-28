/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwopy/pyhistogram.h>
#include <inviwopy/pyglmtypes.h>

#include <pybind11/stl.h>

#include <inviwo/core/datastructures/histogram.h>
#include <inviwo/core/util/glmfmt.h>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>

namespace inviwo {

namespace {
std::string formatStatistics(const Statistics& stats) {
    const std::string percentiles =
        stats.percentiles.size() >= 100
            ? fmt::format("(1: {}, 25: {}, 50: {}, 75: {}, 99: {})", stats.percentiles[1],
                          stats.percentiles[25], stats.percentiles[50], stats.percentiles[75],
                          stats.percentiles[99])
            : fmt::format("{}", stats.percentiles);
    return fmt::format("min: {}, max: {}, mean: {}, stdDev: {}, percentiles: {}", stats.min,
                       stats.max, stats.mean, stats.standardDeviation, percentiles);
}
}  // namespace

void exposeHistogram(pybind11::module& m) {
    namespace py = pybind11;

    py::enum_<HistogramMode>(m, "HistogramMode")
        .value("Off", HistogramMode::Off)
        .value("All", HistogramMode::All)
        .value("P99", HistogramMode::P99)
        .value("P95", HistogramMode::P95)
        .value("P90", HistogramMode::P90)
        .value("Log", HistogramMode::Log);

    py::classh<Statistics>(m, "Statistics")
        .def(py::init<>())
        .def_readwrite("min", &Statistics::min)
        .def_readwrite("max", &Statistics::max)
        .def_readwrite("mean", &Statistics::mean)
        .def_readwrite("standardDeviation", &Statistics::standardDeviation)
        .def_readwrite("percentiles", &Statistics::percentiles)
        .def("__repr__", [](const Statistics& self) {
            return fmt::format("<Statistics: {}>", formatStatistics(self));
        });

    py::classh<Histogram1D>(m, "Histogram1D")
        .def(py::init<>())
        .def_readwrite("counts", &Histogram1D::counts)
        .def_readwrite("totalCounts", &Histogram1D::totalCounts)
        .def_readwrite("maxCount", &Histogram1D::maxCount)
        .def_readwrite("dataMap", &Histogram1D::dataMap)
        .def_readwrite("underflow", &Histogram1D::underflow)
        .def_readwrite("overflow", &Histogram1D::overflow)
        .def_readwrite("dataStats", &Histogram1D::dataStats)
        .def_readwrite("histStats", &Histogram1D::histStats)
        .def("__repr__", [](const Histogram1D& self) {
            return fmt::format(
                "<Histogram1D: binCount: {}, totalCounts: {}, maxCount: {}, "
                "underflow: {}, overflow: {}, dataRange: {}, valueRange: {}>\n"
                "dataStats: {}\n"
                "histStats: {}",
                self.counts.size(), self.totalCounts, self.maxCount, self.underflow, self.overflow,
                self.dataMap.dataRange, self.dataMap.valueRange, formatStatistics(self.dataStats),
                formatStatistics(self.histStats));
        });

    py::classh<Histogram2D>(m, "Histogram2D")
        .def(py::init<>())
        .def_readwrite("counts", &Histogram2D::counts)
        .def_readwrite("dimensions", &Histogram2D::dimensions)
        .def_readwrite("totalCounts", &Histogram2D::totalCounts)
        .def_readwrite("maxCount", &Histogram2D::maxCount)
        .def_readwrite("dataMap", &Histogram2D::dataMap)
        .def_readwrite("underflow", &Histogram2D::underflow)
        .def_readwrite("overflow", &Histogram2D::overflow)
        .def("__repr__", [](const Histogram2D& self) {
            return fmt::format(
                "<Histogram2D: counts: {} x {}, totalCounts: {}, maxCount: {}, "
                "underflow: {}, overflow: {}, dataMap: {}/{}>",
                self.dimensions.x, self.dimensions.y, self.totalCounts, self.maxCount,
                self.underflow, self.overflow, self.dataMap[0].valueRange,
                self.dataMap[1].valueRange);
        });
}

}  // namespace inviwo
