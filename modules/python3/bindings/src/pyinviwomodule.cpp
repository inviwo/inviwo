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

#include <inviwopy/pyinviwomodule.h>
#include <inviwo/core/common/inviwomodule.h>

namespace inviwo {

void exposeInviwoModule(pybind11::module &m) {
    namespace py = pybind11;

    py::enum_<inviwo::ModulePath>(m, "ModulePath")
        .value("Data", ModulePath::Data)
        .value("Images", ModulePath::Images)
        .value("PortInspectors", ModulePath::PortInspectors)
        .value("Scripts", ModulePath::Scripts)
        .value("Volumes", ModulePath::Volumes)
        .value("Workspaces", ModulePath::Workspaces)
        .value("Tests", ModulePath::Tests)
        .value("TestImages", ModulePath::TestImages)
        .value("TestVolumes", ModulePath::TestVolumes)
        .value("UnitTests", ModulePath::UnitTests)
        .value("RegressionTests", ModulePath::RegressionTests)
        .value("GLSL", ModulePath::GLSL)
        .value("CL", ModulePath::CL);

    py::class_<InviwoModule>(m, "InviwoModule")
        .def_property_readonly("identifier", &InviwoModule::getIdentifier)
        .def_property_readonly("description", &InviwoModule::getDescription)
        .def_property_readonly("path", [](InviwoModule *m) { return m->getPath(); })
        .def_property_readonly("version", &InviwoModule::getVersion)
        .def("getPath", [](InviwoModule *m, ModulePath type) { return m->getPath(type); });
}

}  // namespace inviwo
