/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2023 Inviwo Foundation
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

#include <inviwopy/pydatamapper.h>

#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/datastructures/unitsystem.h>

#include <inviwo/core/util/glmfmt.h>
#include <pybind11/numpy.h>
#include <fmt/core.h>

namespace py = pybind11;

namespace inviwo {

void exposeDataMapper(py::module& m) {

    py::class_<Unit>(m, "Unit")
        .def(py::init([](std::string unit) { return units::unit_from_string(unit); }))
        .def("to_string", [](const Unit& unit,
                             std::string_view format = "{}") { return fmt::format(fmt::runtime(format), unit); })
        .def("__repr__", [](const Unit& unit) { return fmt::format("{}", unit); });

    py::class_<Axis>(m, "Axis")
        .def(py::init<std::string, Unit>())
        .def_readwrite("name", &Axis::name)
        .def_readwrite("unit", &Axis::unit)
        .def("__repr__",
             [](const Axis& axis) { return fmt::format("{}{: [}", axis.name, axis.unit); });

    py::class_<DataMapper>(m, "DataMapper")
        .def(py::init())
        .def_readwrite("dataRange", &DataMapper::dataRange)
        .def_readwrite("valueRange", &DataMapper::valueRange)
        .def_readwrite("valueAxis", &DataMapper::valueAxis)
        .def("__repr__", [](const DataMapper& dataMapper) {
            return fmt::format("DataMapper[data: {}, value: {}, axis: {}{: [}]",
                               dataMapper.dataRange, dataMapper.valueRange,
                               dataMapper.valueAxis.name, dataMapper.valueAxis.unit);
        });
}

}  // namespace inviwo
