/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2025 Inviwo Foundation
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

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/functional.h>

#include <inviwo/core/ports/port.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/processors/processor.h>

#include <modules/python3/pythonoutport.h>
#include <modules/python3/pythoninport.h>
#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>
#include <modules/python3/pyportutils.h>

#include <utility>

namespace inviwo {

namespace {

struct InportCallbackHolderVoid {
    std::shared_ptr<std::function<void()>> value;
};
struct InportCallbackHolderOutport {
    std::shared_ptr<std::function<void(Outport*)>> value;
};

}  // namespace
void exposePort(pybind11::module& m) {
    namespace py = pybind11;

    py::classh<InportCallbackHolderVoid>(m, "InportCallbackHolderVoid")
        .def("reset", [](InportCallbackHolderVoid* h) { h->value.reset(); });
    py::classh<InportCallbackHolderOutport>(m, "InportCallbackHolderOutport")
        .def("reset", [](InportCallbackHolderOutport* h) { h->value.reset(); });

    py::classh<Port>(m, "Port")
        .def_property_readonly("identifier", &Port::getIdentifier)
        .def_property_readonly("processor", &Port::getProcessor, py::return_value_policy::reference)
        .def_property_readonly("classIdentifier", &Port::getClassIdentifier)
        .def_property_readonly("contentInfo", &Port::getInfo)
        .def("isConnected", &Port::isConnected)
        .def("isReady", &Port::isReady);

    py::classh<Outport, Port>(m, "Outport")
        .def("isConnectedTo", &Outport::isConnectedTo)
        .def("getConnectedInports", &Outport::getConnectedInports,
             py::return_value_policy::reference)
        .def("hasData", &Outport::hasData)
        .def("clear", &Outport::clear);

    py::classh<Inport, Port>(m, "Inport")
        .def_property("optional", &Inport::isOptional, &Inport::setOptional)
        .def("isChanged", &Inport::isChanged)
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
        .def("getChangedOutports", &Inport::getChangedOutports, py::return_value_policy::reference)

        .def("onChangeScoped",
             [](Inport* p, std::function<void()> func) {
                 return InportCallbackHolderVoid{p->onConnectScoped(std::move(func))};
             })
        .def("onInvalidScoped",
             [](Inport* p, std::function<void()> func) {
                 return InportCallbackHolderVoid{p->onInvalidScoped(std::move(func))};
             })
        .def("onConnectScoped",
             [](Inport* p, std::function<void(Outport*)> func) {
                 return InportCallbackHolderOutport{p->onConnectScoped(std::move(func))};
             })
        .def("onDisconnectScoped", [](Inport* p, std::function<void(Outport*)> func) {
            return InportCallbackHolderOutport{p->onDisconnectScoped(std::move(func))};
        });

    py::classh<PythonInport, Inport>(m, "PythonInport")
        .def(py::init<std::string, Document>(), py::arg("identifier"), py::arg("help") = Document{})
        .def("hasData", &PythonInport::hasData)
        .def("getData", &PythonInport::getData);

    py::classh<PythonOutport, Outport>(m, "PythonOutport")
        .def(py::init<std::string, Document>(), py::arg("identifier"), py::arg("help") = Document{})
        .def("getData", &PythonOutport::getData)
        .def("setData", &PythonOutport::setData);

    exposeInport<DataInport<std::vector<std::string>>>(m, "StringVector");
    exposeOutport<DataOutport<std::vector<std::string>>>(m, "StringVector");
}

}  // namespace inviwo
