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

#include <inviwopy/pymodulemanager.h>

#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include <inviwo/core/common/modulemanager.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/common/inviwomodulefactoryobject.h>

#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

#include <vector>
#include <memory>

namespace inviwo {

namespace {

struct ModuleManagerCallbackHolder {
    std::shared_ptr<std::function<void()>> value;
};

}  // namespace

void exposeModuleManager(pybind11::module& m) {
    namespace py = pybind11;

    py::classh<ModuleManagerCallbackHolder>(m, "ModuleManagerCallbackHolder")
        .def("reset", [](ModuleManagerCallbackHolder* h) { h->value.reset(); });

    py::classh<ModuleManager>(m, "ModuleManager")
        .def("__contains__",
             [](const ModuleManager& mm, std::string_view identifier) {
                 return mm.getModuleByIdentifier(identifier) != nullptr;
             })
        .def(
            "__getitem__",
            [](const ModuleManager& mm, std::string_view identifier) {
                if (auto* mod = mm.getModuleByIdentifier(identifier)) {
                    return mod;
                }
                throw py::key_error(std::string{identifier});
            },
            py::return_value_policy::reference)
        .def(
            "modules",
            [](ModuleManager& mm) {
                std::vector<InviwoModule*> res;
                for (auto& mod : mm.getInviwoModules()) {
                    res.push_back(&mod);
                }
                return res;
            },
            py::return_value_policy::reference,
            "Return a list of all registered InviwoModule instances.")
        .def("getModuleByIdentifier", &ModuleManager::getModuleByIdentifier, py::arg("identifier"),
             py::return_value_policy::reference,
             "Retrieve a module by its identifier or None if not found.")
        .def("getModuleByIndex", &ModuleManager::getModuleByIndex, py::arg("index"),
             py::return_value_policy::reference, "Retrieve a module by index.")
        .def("getModulesByAlias", &ModuleManager::getModulesByAlias, py::arg("alias"),
             py::return_value_policy::reference, "Retrieve all modules matching the given alias.")
        .def("getFactoryObject", &ModuleManager::getFactoryObject, py::arg("identifier"),
             py::return_value_policy::reference,
             "Retrieve the factory object for the module with the given identifier.")
        .def(
            "factoryObjects",
            [](ModuleManager& mm) {
                std::vector<const InviwoModuleFactoryObject*> res;
                for (const auto& fo : mm.getFactoryObjects()) {
                    res.push_back(&fo);
                }
                return res;
            },
            py::return_value_policy::reference,
            "Return a list of all registered module factory objects.")
        .def("findDependentModules",
             py::overload_cast<std::string_view>(&ModuleManager::findDependentModules, py::const_),
             py::arg("module"),
             "Return identifiers of modules that depend (transitively) on the given module.")
        .def("isRuntimeModuleReloadingEnabled", &ModuleManager::isRuntimeModuleReloadingEnabled,
             "True if runtime reloading of modules is enabled.")
        .def("reloadModules", &ModuleManager::reloadModules, "Reload all runtime-loadable modules.")
        .def("locateModule", &ModuleManager::locateModule, py::arg("module"),
             "Locate the source/path of the given module.")
        .def(
            "onModulesDidRegister",
            [](ModuleManager& mm, std::function<void()> callback) {
                return ModuleManagerCallbackHolder{mm.onModulesDidRegister(std::move(callback))};
            },
            py::arg("callback"), "Register a callback invoked after modules have been registered.")
        .def(
            "onModulesWillUnregister",
            [](ModuleManager& mm, std::function<void()> callback) {
                return ModuleManagerCallbackHolder{mm.onModulesWillUnregister(std::move(callback))};
            },
            py::arg("callback"),
            "Register a callback invoked before modules will be unregistered.");
}

}  // namespace inviwo
