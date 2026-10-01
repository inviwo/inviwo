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
#include <inviwopy/util/pydatasequence.h>

#include <pybind11/stl.h>
#include <pybind11/native_enum.h>

#include <inviwo/core/datastructures/histogram.h>
#include <inviwo/core/util/glmfmt.h>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>

#include <modules/python3/pyportutils.h>

#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

namespace inviwo {

void exposeHistogram(pybind11::module& m) {
    namespace py = pybind11;

    py::native_enum<HistogramMode>(m, "HistogramMode", "enum.Enum")
        .value("Off", HistogramMode::Off)
        .value("All", HistogramMode::All)
        .value("P99", HistogramMode::P99)
        .value("P95", HistogramMode::P95)
        .value("P90", HistogramMode::P90)
        .value("Log", HistogramMode::Log)
        .finalize();

    py::classh<Statistics>(m, "Statistics")
        .def(py::init<>())
        .def_readwrite("min", &Statistics::min)
        .def_readwrite("max", &Statistics::max)
        .def_readwrite("mean", &Statistics::mean)
        .def_readwrite("standardDeviation", &Statistics::standardDeviation)
        .def_readwrite("percentiles", &Statistics::percentiles)
        .def("__repr__",
             [](const Statistics& self) { return fmt::format("<Statistics: {}>", self); });

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
        .def_readwrite("name", &Histogram1D::name)

        .def("__repr__",
             [](const Histogram1D& self) { return fmt::format("<Histogram1D {}>", self); });

    py::classh<Histogram2D>(m, "Histogram2D")
        .def(py::init<>())
        .def_readwrite("counts", &Histogram2D::counts)
        .def_readwrite("dimensions", &Histogram2D::dimensions)
        .def_readwrite("totalCounts", &Histogram2D::totalCounts)
        .def_readwrite("maxCount", &Histogram2D::maxCount)
        .def_readwrite("dataMap", &Histogram2D::dataMap)
        .def_readwrite("underflow", &Histogram2D::underflow)
        .def_readwrite("overflow", &Histogram2D::overflow)
        .def_readwrite("dataStats", &Histogram2D::dataStats)
        .def_readwrite("histStats", &Histogram2D::histStats)
        .def_readwrite("name", &Histogram2D::name)

        .def("__repr__",
             [](const Histogram2D& self) { return fmt::format("<Histogram2D {}>", self); });

    //util::exportDataSequenceFor<Histogram1D>(m, "Histogram1D");
    exposeStandardDataPorts<Histogram1D>(m, "Histogram1D");
    //exposeStandardDataPorts<DataSequence<Histogram1D>>(m, "Histogram1DSequence");

    //util::exportDataSequenceFor<Histogram2D>(m, "Histogram2D");
    exposeStandardDataPorts<Histogram2D>(m, "Histogram2D");
    //exposeStandardDataPorts<DataSequence<Histogram2D>>(m, "Histogram2DSequence");
}

}  // namespace inviwo
