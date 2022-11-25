/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2022 Inviwo Foundation
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

#include <inviwopy/pyinviwoapplication.h>
#include <inviwopy/vectoridentifierwrapper.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/moduleregistration.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/processors/processorfactory.h>

namespace inviwo {

void exposeInviwoApplication(pybind11::module& m) {
    namespace py = pybind11;

    py::enum_<inviwo::PathType>(m, "PathType")
        .value("Data", PathType::Data)
        .value("Volumes", PathType::Volumes)
        .value("Workspaces", PathType::Workspaces)
        .value("Scripts", PathType::Scripts)
        .value("PortInspectors", PathType::PortInspectors)
        .value("Images", PathType::Images)
        .value("Databases", PathType::Databases)
        .value("Resources", PathType::Resources)
        .value("TransferFunctions", PathType::TransferFunctions)
        .value("Settings", PathType::Settings)
        .value("Help", PathType::Help)
        .value("Tests", PathType::Tests);

    using ModuleVecWrapper = VectorIdentifierWrapper<std::vector<std::unique_ptr<InviwoModule>>>;
    exposeVectorIdentifierWrapper<std::vector<std::unique_ptr<InviwoModule>>>(
        m, "ModuleVectorWrapper");

    py::class_<InviwoApplication>(m, "InviwoApplication", py::multiple_inheritance{})
        .def(py::init<>())
        .def(py::init<std::string>())
        .def("getBasePath", &InviwoApplication::getBasePath)
        .def("getPath", &InviwoApplication::getPath, py::arg("pathType"), py::arg("suffix") = "",
             py::arg("createFolder") = false)
        .def_property_readonly("basePath", &InviwoApplication::getBasePath)
        .def_property_readonly("displayName", &InviwoApplication::getDisplayName)

        .def_property_readonly(
            "modules", [](InviwoApplication* app) { return ModuleVecWrapper(app->getModules()); })
        .def("getModuleByIdentifier", &InviwoApplication::getModuleByIdentifier,
             py::return_value_policy::reference)
        .def("getModuleSettings", &InviwoApplication::getModuleSettings,
             py::return_value_policy::reference)

        .def("waitForPool", &InviwoApplication::waitForPool)
        .def("resizePool", &InviwoApplication::resizePool)
        .def("getPoolSize", &InviwoApplication::getPoolSize)

        .def("getOutputPath",
             [](InviwoApplication* app) { return app->getCommandLineParser().getOutputPath(); })

        .def("registerModules",
             [](InviwoApplication* app) { app->registerModules(inviwo::getModuleList()); })
        .def("registerRuntimeModules",
             [](InviwoApplication* app) { app->registerModules(RuntimeModuleLoading{}); })
        .def("runningBackgroundJobs",
             [](InviwoApplication* app) {
                 return app->getProcessorNetwork()->runningBackgroundJobs();
             })

        .def_property_readonly("network", &InviwoApplication::getProcessorNetwork,
                               "Get the processor network", py::return_value_policy::reference)

        .def_property_readonly("dataReaderFactory", &InviwoApplication::getDataReaderFactory,
                               py::return_value_policy::reference)

        .def_property_readonly("dataWriterFactory", &InviwoApplication::getDataReaderFactory,
                               py::return_value_policy::reference)

        .def_property_readonly("processorFactory", &InviwoApplication::getProcessorFactory,
                               py::return_value_policy::reference)
        .def_property_readonly("propertyFactory", &InviwoApplication::getPropertyFactory,
                               py::return_value_policy::reference);
}

}  // namespace inviwo
