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

#include <inviwopy/pyinviwoapplication.h>

#include <pybind11/functional.h>

#include <inviwopy/vectoridentifierwrapper.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/common/modulemanager.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/settings/settings.h>
#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/processors/processorfactory.h>

#include <pybind11/stl/filesystem.h>

namespace inviwo {

class ModuleIdentifierWrapper {
    static constexpr auto id = [](const auto& elem) -> decltype(auto) {
        return elem.getIdentifier();
    };

public:
    ModuleIdentifierWrapper(InviwoApplication* app) : app_{app} {}

    decltype(auto) getFromIdentifier(std::string_view identifier) const {
        auto range = app_->getModuleManager().getInviwoModules();
        auto it =
            std::ranges::find_if(range, [&](const auto& elem) { return id(elem) == identifier; });
        if (it != range.end()) {
            return *it;
        } else {
            throw pybind11::key_error();
        }
    }

    decltype(auto) getFromIndex(ptrdiff_t pos) const {
        auto range = app_->getModuleManager().getInviwoModules();
        if (pos >= 0) {
            if (pos < size()) {
                return *(std::next(range.begin(), pos));
            } else {
                throw pybind11::index_error();
            }
        } else {
            const auto ind = size() + pos;
            if (ind >= 0) {
                return *(std::next(range.begin(), ind));
            } else {
                throw pybind11::index_error();
            }
        }
    }

    ptrdiff_t size() const {
        auto range = app_->getModuleManager().getInviwoModules();
        return static_cast<ptrdiff_t>(std::distance(range.begin(), range.end()));
    }

    bool contains(std::string_view identifier) const {
        auto range = app_->getModuleManager().getInviwoModules();
        return std::ranges::find_if(
                   range, [&](const auto& elem) { return id(elem) == identifier; }) != range.end();
    }

    std::vector<std::string> identifiers() const {
        std::vector<std::string> res;
        auto range = app_->getModuleManager().getInviwoModules();
        std::ranges::transform(range, std::back_inserter(res), id);
        return res;
    }

    std::string repr() const {
        std::stringstream ss;
        ss << "[";
        auto joiner = util::make_ostream_joiner(ss, ", ");
        auto range = app_->getModuleManager().getInviwoModules();
        std::ranges::transform(range, joiner, id);
        ss << "]";
        return ss.str();
    }

private:
    InviwoApplication* app_;
};

void exposeModuleIdentifierWrapper(pybind11::module& m, const std::string& name) {
    namespace py = pybind11;

    py::class_<ModuleIdentifierWrapper>(m, name.c_str())
        .def("__getattr__", &ModuleIdentifierWrapper::getFromIdentifier,
             py::return_value_policy::reference)
        .def("__getitem__", &ModuleIdentifierWrapper::getFromIdentifier,
             py::return_value_policy::reference)
        .def("__getitem__", &ModuleIdentifierWrapper::getFromIndex,
             py::return_value_policy::reference)
        .def("__len__", &ModuleIdentifierWrapper::size)
        .def("__contains__", &ModuleIdentifierWrapper::contains)
        .def("__repr__", &ModuleIdentifierWrapper::repr)
        .def("__dir__", &ModuleIdentifierWrapper::identifiers);
}

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

    exposeModuleIdentifierWrapper(m, "ModuleIdentifierWrapper");

    py::class_<InviwoApplication>(m, "InviwoApplication", py::multiple_inheritance{})
        .def(py::init<>())
        .def(py::init<std::string>())
        .def("getBasePath", &InviwoApplication::getBasePath)
        .def("getPath", &InviwoApplication::getPath, py::arg("pathType"), py::arg("suffix") = "",
             py::arg("createFolder") = false)
        .def_property_readonly("basePath", &InviwoApplication::getBasePath)
        .def_property_readonly("displayName", &InviwoApplication::getDisplayName)

        .def_property_readonly("modules",
                               [](InviwoApplication* app) { return ModuleIdentifierWrapper(app); })
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
             [](InviwoApplication* app,
                std::vector<std::unique_ptr<InviwoModuleFactoryObject>> modules) {
                 app->registerModules(std::move(modules));
             })
        .def("registerRuntimeModules",
             [](InviwoApplication* app) { app->registerModules(RuntimeModuleLoading{}); })
        .def("registerRuntimeModules",
             [](InviwoApplication* app, std::function<bool(std::string_view)> filter) {
                 app->registerModules(RuntimeModuleLoading{}, filter);
             })
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
                               py::return_value_policy::reference)
        .def("setProgressCallback", &InviwoApplication::setProgressCallback);
}

}  // namespace inviwo
