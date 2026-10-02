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

#include <inviwopy/pyhelp.h>
#include <pybind11/stl.h>
#include <pybind11/native_enum.h>

#include <inviwo/core/util/docbuilder.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/processors/processor.h>

#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

namespace inviwo {

void exposeHelp(pybind11::module& m) {
    namespace py = pybind11;

    py::classh<help::HelpInport>(m, "HelpInport")
        .def(py::init<>())
        .def_readwrite("classIdentifier", &help::HelpInport::classIdentifier)
        .def_readwrite("displayName", &help::HelpInport::displayName)
        .def_readwrite("typeName", &help::HelpInport::typeName)
        .def_readwrite("colorCode", &help::HelpInport::colorCode)
        .def_readwrite("data", &help::HelpInport::data)
        .def_readwrite("help", &help::HelpInport::help)
        .def("serialize", &help::HelpInport::serialize)
        .def("deserialize", &help::HelpInport::deserialize)
        .def("__repr__", [](const help::HelpInport& self) {
            return fmt::format("<help::HelpInport {}>", self.classIdentifier);
        });

    py::classh<help::HelpOutport>(m, "HelpOutport")
        .def(py::init<>())
        .def_readwrite("classIdentifier", &help::HelpOutport::classIdentifier)
        .def_readwrite("displayName", &help::HelpOutport::displayName)
        .def_readwrite("typeName", &help::HelpOutport::typeName)
        .def_readwrite("colorCode", &help::HelpOutport::colorCode)
        .def_readwrite("data", &help::HelpOutport::data)
        .def_readwrite("help", &help::HelpOutport::help)
        .def("serialize", &help::HelpOutport::serialize)
        .def("deserialize", &help::HelpOutport::deserialize)
        .def("__repr__", [](const help::HelpOutport& self) {
            return fmt::format("<help::HelpOutport {}>", self.classIdentifier);
        });

    py::classh<help::HelpProperty>(m, "HelpProperty")
        .def(py::init<>())
        .def_readwrite("classIdentifier", &help::HelpProperty::classIdentifier)
        .def_readwrite("displayName", &help::HelpProperty::displayName)
        .def_readwrite("typeName", &help::HelpProperty::typeName)
        .def_readwrite("help", &help::HelpProperty::help)
        .def_readwrite("properties", &help::HelpProperty::properties)
        .def("serialize", &help::HelpProperty::serialize)
        .def("deserialize", &help::HelpProperty::deserialize)
        .def("__repr__", [](const help::HelpProperty& self) {
            return fmt::format("<help::HelpProperty {}>", self.classIdentifier);
        });

    py::classh<help::HelpProcessor>(m, "HelpProcessor")
        .def(py::init<>())
        .def_readwrite("classIdentifier", &help::HelpProcessor::classIdentifier)
        .def_readwrite("displayName", &help::HelpProcessor::displayName)
        .def_readwrite("typeName", &help::HelpProcessor::typeName)
        .def_readwrite("category", &help::HelpProcessor::category)
        .def_readwrite("codeState", &help::HelpProcessor::codeState)
        .def_readwrite("tags", &help::HelpProcessor::tags)
        .def_readwrite("help", &help::HelpProcessor::help)
        .def_readwrite("file", &help::HelpProcessor::file)
        .def_readwrite("inports", &help::HelpProcessor::inports)
        .def_readwrite("outports", &help::HelpProcessor::outports)
        .def_readwrite("properties", &help::HelpProcessor::properties)
        .def_readwrite("inviwoModule", &help::HelpProcessor::inviwoModule)
        .def_readwrite("meta", &help::HelpProcessor::meta)
        .def("serialize", &help::HelpProcessor::serialize)
        .def("deserialize", &help::HelpProcessor::deserialize)
        .def("__repr__", [](const help::HelpProcessor& self) {
            return fmt::format("<help::HelpProcessor {}>", self.classIdentifier);
        });

    m.def("toDocument", [](const help::HelpProcessor& p) { return help::toDocument(p); });
    m.def("toDocument", [](const help::HelpProperty& p, std::string_view path = "") {
        return help::toDocument(p, path);
    });

    m.def("buildProcessorHelp", [](Processor& processor, InviwoApplication& app) {
        return help::buildProcessorHelp(processor, app);
    });
}

}  // namespace inviwo
