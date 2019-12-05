/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <inviwopy/pyport.h>
#include <modules/python3/pyportutils.h>

#include <inviwopy/pyglmtypes.h>

#include <vector>

namespace inviwo {

namespace {

struct ExposePortsFunctor {
    template <typename T>
    void operator()(pybind11::module& m, const std::vector<std::string>& typenames) {
        exposeStandardDataPorts<T>(m, typenames[index]);
        exposeStandardDataPorts<std::vector<T>>(m, typenames[index] + std::string("Vector"));
        ++index;
    }
    size_t index = 0;
};

}  // namespace

void exposePort(pybind11::module& m) {
    namespace py = pybind11;
    py::class_<Port, PortPtr<Port>>(m, "Port")
        .def_property_readonly("identifier", &Port::getIdentifier)
        .def_property_readonly("processor", &Port::getProcessor, py::return_value_policy::reference)
        .def_property_readonly("classIdentifier", &Port::getClassIdentifier)
        .def_property_readonly("contentInfo", &Port::getInfo)
        .def("isConnected", &Port::isConnected)
        .def("isReady", &Port::isReady);

    py::class_<Inport, Port, PortPtr<Inport>>(m, "Inport")
        .def_property("optional", &Inport::isOptional, &Inport::setOptional)
        .def("canConnectTo", &Inport::canConnectTo)
        .def("connectTo", &Inport::connectTo)
        .def("disconnectFrom", &Inport::disconnectFrom)
        .def("isConnectedTo", &Inport::isConnectedTo)
        .def("getConnectedOutport", &Inport::getConnectedOutport,
             py::return_value_policy::reference)
        .def("getConnectedOutports", &Inport::getConnectedOutports,
             py::return_value_policy::reference)
        .def("getMaxNumberOfConnections", &Inport::getMaxNumberOfConnections)
        .def("getNumberOfConnections", &Inport::getNumberOfConnections)
        .def("getChangedOutports", &Inport::getChangedOutports, py::return_value_policy::reference);

    py::class_<Outport, Port, PortPtr<Outport>>(m, "Outport")
        .def("isConnectedTo", &Outport::isConnectedTo)
        .def("getConnectedInports", &Outport::getConnectedInports,
             py::return_value_policy::reference);

    // the datatypes for exposed ports should match those in inviwocore.cpp
    // clang-format off
    using types = std::tuple<
        float, vec2, vec3, vec4,
        double, dvec2, dvec3, dvec4,
        int32_t, ivec2, ivec3, ivec4,
        uint32_t, uvec2, uvec3, uvec4,
        std::int64_t, i64vec2, i64vec3, i64vec4,
        std::uint64_t, u64vec2, u64vec3, u64vec4
    >;
    const std::vector<std::string> typeNames = {
        "float", "vec2", "vec3", "vec4",
        "double", "dvec2", "dvec3", "dvec4",
        "int32", "ivec2", "ivec3", "ivec4",
        "uint32", "uvec2", "uvec3", "uvec4",
        "int64", "i64vec2", "i64vec3", "i64vec4",
        "uint64", "u64vec2", "u64vec3", "u64vec4"
    };
    // clang-format on
    util::for_each_type<types>{}(ExposePortsFunctor{}, m, typeNames);
}

}  // namespace inviwo
