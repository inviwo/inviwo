/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2026 Inviwo Foundation
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

#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/numpy.h>

#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/datastructures/unitsystem.h>

#include <inviwo/core/util/glmfmt.h>

#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

#include <fmt/format.h>

namespace py = pybind11;

namespace inviwo {

namespace {

// see inviwo/core/datastructures/unitsystem.h
constexpr const char* formatDoc = R"(The Units have a Format Specification Mini-Language

spec         ::= [unit_spec][":" format_spec]

format_spec ::= fmt standard format specifier

unit_spec   ::= [space][braces][prefix][units]

space       ::= " "
braces      ::= "(" | "["
prefix      ::= "p" | "P"
units       ::= "si" | "ext" | "sys" | "all"

Space:
    " "     Add a leading space to the unit

Braces:    By default no braces are added.
    "("     Surround the unit in (unit)
    "["     Surround the unit in [unit]

Prefix:
    "p"     Use SI prefixes (yocto to Yotta) to reduce any multiplier (default)
    "P"     Don't use any prefixes

Units:
    "si"    Only use the basic SI unit and combination of those.
    "ext"   Use unit from the SI, derived and extra groups.
    "sys"   Use the systems currently selected set of unit groups (default)
    "all"   Use all known unit groups.
)";

}

void exposeDataMapper(py::module& m) {

    py::classh<Unit>(m, "Unit")
        .def(py::init([](std::string unit) { return units::unit_from_string(std::move(unit)); }))
        .def(py::init([](double multiplier, const Unit& unit) { return Unit(multiplier, unit); }),
             py::arg("multiplier"), py::arg("unit"))
        .def(
            "__format__",
            [](const Unit& unit, std::string_view spec) {
                const auto format = fmt::format("{{:{}}}", spec);
                return fmt::format(fmt::runtime(format), unit);
            },
            py::arg("format") = "{}", py::doc(formatDoc))
        .def("__str__", [](const Unit& unit) { return fmt::to_string(unit); })
        .def("__repr__", [](const Unit& unit) { return fmt::to_string(unit); });

    py::classh<Axis>(m, "Axis")
        .def(py::init<std::string, Unit>())
        .def_readwrite("name", &Axis::name)
        .def_readwrite("unit", &Axis::unit)
        .def("__repr__",
             [](const Axis& axis) { return fmt::format("{}{: [}", axis.name, axis.unit); });

    py::classh<DataMapper>(m, "DataMapper")
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
