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

#include <inviwopy/pyworkspacemanager.h>

#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/core/io/serialization/serializer.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/sourcecontext.h>

#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

#include <sstream>
#include <string>

namespace inviwo {

namespace {

struct WorkspaceManagerOnModifiedCallbackHolder {
    std::shared_ptr<std::function<void(bool)>> value;
};
struct WorkspaceManagerOnClearChangedCallbackHolder {
    std::shared_ptr<std::function<void()>> value;
};

}  // namespace

void exposeWorkspaceManager(pybind11::module& m) {
    namespace py = pybind11;

    py::classh<WorkspaceManagerOnModifiedCallbackHolder>(m,
                                                         "WorkspaceManagerOnModifiedCallbackHolder")
        .def("reset", [](WorkspaceManagerOnModifiedCallbackHolder* h) { h->value.reset(); });
    py::classh<WorkspaceManagerOnClearChangedCallbackHolder>(
        m, "WorkspaceManagerOnClearChangedCallbackHolder")
        .def("reset", [](WorkspaceManagerOnClearChangedCallbackHolder* h) { h->value.reset(); });

    py::enum_<WorkspaceSaveMode>(m, "WorkspaceSaveMode")
        .value("Disk", WorkspaceSaveMode::Disk)
        .value("Undo", WorkspaceSaveMode::Undo);

    // Default exception handler used by python bindings: re-throw so that the exception
    // is propagated to the python interpreter.
    static constexpr auto rethrowHandler = [](SourceContext) { throw; };

    py::classh<WorkspaceManager>(m, "WorkspaceManager")
        .def("clear", &WorkspaceManager::clear,
             "Clear the current workspace. Invokes all registered clear callbacks.")
        .def(
            "save",
            [](WorkspaceManager* wm, const std::filesystem::path& path, WorkspaceSaveMode mode) {
                wm->save(path, rethrowHandler, mode);
            },
            py::arg("path"), py::arg("mode") = WorkspaceSaveMode::Disk,
            "Save the current workspace to a file.")
        .def(
            "saveToString",
            [](WorkspaceManager* wm, const std::filesystem::path& refPath, WorkspaceSaveMode mode) {
                std::pmr::string xml;
                wm->save(xml, refPath, rethrowHandler, mode);
                return std::string{xml.data(), xml.size()};
            },
            py::arg("refPath") = std::filesystem::path{}, py::arg("mode") = WorkspaceSaveMode::Disk,
            "Save the current workspace to a string (xml).")
        .def(
            "load",
            [](WorkspaceManager* wm, const std::filesystem::path& path, WorkspaceSaveMode mode) {
                wm->load(path, rethrowHandler, mode);
            },
            py::arg("path"), py::arg("mode") = WorkspaceSaveMode::Disk,
            "Load a workspace from a file.")
        .def(
            "loadFromString",
            [](WorkspaceManager* wm, const std::string& xml, const std::filesystem::path& refPath,
               WorkspaceSaveMode mode) {
                std::pmr::string pxml{xml.begin(), xml.end()};
                wm->load(pxml, refPath, rethrowHandler, mode);
            },
            py::arg("xml"), py::arg("refPath") = std::filesystem::path{},
            py::arg("mode") = WorkspaceSaveMode::Disk, "Load a workspace from a string (xml).")
        .def("setModified", py::overload_cast<>(&WorkspaceManager::setModified),
             "Mark the workspace as modified.")
        .def("isModified", &WorkspaceManager::isModified,
             "Return true if the workspace has been modified since the last save/load.")
        .def_property_readonly("modified", &WorkspaceManager::isModified)
        .def(
            "onModified",
            [](WorkspaceManager* wm, std::function<void(bool)> callback) {
                return WorkspaceManagerOnModifiedCallbackHolder{
                    wm->onModified(std::move(callback))};
            },
            py::arg("callback"),
            "Register a callback invoked whenever the modified state changes (called every "
            "time setModified is invoked).")
        .def(
            "onModifiedChanged",
            [](WorkspaceManager* wm, std::function<void(bool)> callback) {
                return WorkspaceManagerOnModifiedCallbackHolder{
                    wm->onModifiedChanged(std::move(callback))};
            },
            py::arg("callback"),
            "Register a callback invoked only when the modified state transitions.")
        .def(
            "onClear",
            [](WorkspaceManager* wm, std::function<void()> callback) {
                return WorkspaceManagerOnClearChangedCallbackHolder{
                    wm->onClear(std::move(callback))};
            },
            py::arg("callback"), "Register a callback invoked when the workspace is cleared.")
        .def(
            "createWorkspaceDeserializer",
            [](WorkspaceManager* wm, const std::string& xml, const std::filesystem::path& refPath) {
                std::pmr::string pxml{xml.begin(), xml.end()};
                return wm->createWorkspaceDeserializerAndInfo(pxml, refPath).first;
            },
            py::arg("xml"), py::arg("refPath") = std::filesystem::path{},
            "Create a deserializer for loading a workspace from a xml string with the given "
            "refPath.");
}

}  // namespace inviwo
