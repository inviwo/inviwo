/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2024 Inviwo Foundation
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

#include <inviwo/core/ports/port.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/processors/processor.h>

#include <modules/python3/pythonoutport.h>
#include <modules/python3/pythoninport.h>

#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/pybind11.h>
#include <warn/pop>

namespace inviwo {

void exposePort(pybind11::module& m) {
    namespace py = pybind11;
    py::class_<Port>(m, "Port")
        .def_property_readonly("identifier", &Port::getIdentifier)
        .def_property_readonly("processor", &Port::getProcessor, py::return_value_policy::reference)
        .def_property_readonly("classIdentifier", &Port::getClassIdentifier)
        .def_property_readonly("contentInfo", &Port::getInfo)
        .def("isConnected", &Port::isConnected)
        .def("isReady", &Port::isReady);

    py::class_<Inport, Port>(m, "Inport")
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

    py::class_<Outport, Port>(m, "Outport")
        .def("isConnectedTo", &Outport::isConnectedTo)
        .def("getConnectedInports", &Outport::getConnectedInports,
             py::return_value_policy::reference)
        .def("hasData", &Outport::hasData)
        .def("clear", &Outport::clear);

    py::class_<PythonInport, Inport>(m, "PythonInport")
        .def(py::init<std::string, Document>(), py::arg("identifier"), py::arg("help") = Document{})
        .def("hasData", &PythonInport::hasData)
        .def("getData", &PythonInport::getData);

    py::class_<PythonOutport, Outport>(m, "PythonOutport")
        .def(py::init<std::string, Document>(), py::arg("identifier"), py::arg("help") = Document{})
        .def("getData", &PythonOutport::getData)
        .def("setData", &PythonOutport::setData);
}

}  // namespace inviwo
