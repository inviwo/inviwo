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

#include <inviwopy/pyinviwomodule.h>

#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodulefactoryobject.h>
#include <inviwo/core/processors/processorfactoryobject.h>
#include <inviwo/core/common/version.h>
#include <inviwo/core/util/licenseinfo.h>
#include <inviwo/core/util/stringconversion.h>

#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/stl/filesystem.h>

namespace inviwo {

#include <warn/push>
#include <warn/ignore/attributes>
class ProcessorFactoryObjectPythonWrapper : public ProcessorFactoryObject {
public:
    ProcessorFactoryObjectPythonWrapper(pybind11::object pfo)
        : ProcessorFactoryObject{pfo.cast<ProcessorFactoryObject*>()->getProcessorInfo(),
                                 "PythonProcessor"}
        , pfo_(pfo) {}

    virtual std::unique_ptr<Processor> create(InviwoApplication* app) override {
        return pfo_.cast<ProcessorFactoryObject*>()->create(app);
    }

    pybind11::object pfo_;
};

class InviwoModuleFactoryObjectTrampoline : public InviwoModuleFactoryObject,
                                            public pybind11::trampoline_self_life_support {
public:
    using InviwoModuleFactoryObject::InviwoModuleFactoryObject;

    virtual pybind11::object createModule(InviwoApplication* app) {
        PYBIND11_OVERLOAD(pybind11::object, InviwoModuleFactoryObjectTrampoline, createModule, app);
    }

    virtual std::unique_ptr<InviwoModule> create(InviwoApplication* app) override {
        auto mod = createModule(app);
        auto m = std::unique_ptr<InviwoModule>(mod.cast<InviwoModule*>());
        mod.release();
        return m;
    }
};
#include <warn/pop>

void exposeInviwoModule(pybind11::module& m) {
    namespace py = pybind11;

    py::class_<Version>(m, "Version")
        .def(py::init<std::string_view>())
        .def(py::init<unsigned int, unsigned int, unsigned int, std::string_view,
                      std::string_view>(),
             py::arg("major") = 1, py::arg("minor") = 0, py::arg("patch") = 0,
             py::arg("preRelease") = "", py::arg("build") = "")
        .def("__repr__", [](Version* m) { return fmt::to_string(*m); })
        .def_readwrite("major", &Version::major)
        .def_readwrite("minor", &Version::minor)
        .def_readwrite("patch", &Version::patch)
        .def("preRelease", &Version::preRelease)
        .def("build()", &Version::build)
        .def(py::self < py::self)
        .def(py::self > py::self)
        .def(py::self <= py::self)
        .def(py::self >= py::self)
        .def(py::self != py::self)
        .def(py::self == py::self)
        .def("semanticVersionEqual", &Version::semanticVersionEqual);

    py::class_<LicenseInfo>(m, "LicenseInfo")
        .def(py::init<std::string_view, std::string_view, std::string_view, std::string_view,
                      std::string_view, std::string_view, const std::vector<std::string>&>(),
             py::arg("id"), py::arg("name"), py::arg("version"), py::arg("url"), py::arg("module"),
             py::arg("type"), py::arg("files"))
        .def_readonly("id", &LicenseInfo::id)
        .def_readonly("name", &LicenseInfo::name)
        .def_readonly("version", &LicenseInfo::version)
        .def_readonly("url", &LicenseInfo::url)
        .def_readonly("module", &LicenseInfo::module)
        .def_readonly("type", &LicenseInfo::type)
        .def_readonly("files", &LicenseInfo::files);

    py::enum_<ProtectedModule>(m, "ProtectedModule")
        .value("on", ProtectedModule::on)
        .value("off", ProtectedModule::off);

    py::enum_<ModulePath>(m, "ModulePath")
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
        .def(py::init<InviwoApplication*, std::string_view>())
        .def("__repr__",
             [](InviwoModule* m) {
                 return fmt::format("{} v{}", m->getIdentifier(), m->getVersion());
             })
        .def_property_readonly("identifier", &InviwoModule::getIdentifier)
        .def_property_readonly("description", &InviwoModule::getDescription)
        .def_property_readonly("path", [](InviwoModule* m) { return m->getPath(); })
        .def_property_readonly("version", &InviwoModule::getVersion)
        .def("getPath", [](InviwoModule* m, ModulePath type) { return m->getPath(type); })
        .def("registerProcessor", [](InviwoModule* m, py::object pfo) {
            m->registerProcessor(std::make_unique<ProcessorFactoryObjectPythonWrapper>(pfo));
        });

    py::class_<InviwoModuleFactoryObject, InviwoModuleFactoryObjectTrampoline>(
        m, "InviwoModuleFactoryObject")
        .def(py::init<std::string_view, Version, std::string_view, Version,
                      std::vector<std::string>, std::vector<Version>, std::vector<std::string>,
                      std::vector<LicenseInfo>, ProtectedModule>(),
             py::arg("name"), py::arg("version"), py::arg("description") = "",
             py::arg("inviwoCoreVersion"), py::arg("dependencies") = std::vector<std::string>{},
             py::arg("dependenciesVersion") = std::vector<Version>{},
             py::arg("aliases") = std::vector<std::string>{},
             py::arg("licenses") = std::vector<LicenseInfo>{},
             py::arg("protectedModule") = ProtectedModule::off)
        .def(
            "__repr__",
            [](InviwoModuleFactoryObject* m) { return fmt::format("{} v{}", m->name, m->version); })
        .def("create", &InviwoModuleFactoryObject::create)
        .def_readonly("name", &InviwoModuleFactoryObject::name)
        .def_readonly("version", &InviwoModuleFactoryObject::version)
        .def_readonly("description", &InviwoModuleFactoryObject::description)
        .def_readonly("inviwoCoreVersion", &InviwoModuleFactoryObject::inviwoCoreVersion)
        .def_readonly("dependencies", &InviwoModuleFactoryObject::dependencies)
        .def_readonly("aliases", &InviwoModuleFactoryObject::aliases)
        .def_readonly("licenses", &InviwoModuleFactoryObject::licenses)
        .def_readonly("protectedModule", &InviwoModuleFactoryObject::protectedModule);

    /* TODO implement these, need to figure out how to handle the unique_ptrs.
    .def("registerDataReader", &InviwoModule::registerDataReader)
    .def("registerDataWriter", &InviwoModule::registerDataWriter);
    .def("registerCamera",
         static_cast<void (InviwoModule::*)(std::unique_ptr<CameraFactoryObject>)>(
             &InviwoModule::registerCamera))

    .def("registerDialog",
         static_cast<void (InviwoModule::*)(std::unique_ptr<DialogFactoryObject>)>(
             &InviwoModule::registerDialog))
    .def("registerDrawer", &InviwoModule::registerDrawer)
    .def("registerMetaData", &InviwoModule::registerMetaData)

    .def("registerCompositeProcessor", &InviwoModule::registerCompositeProcessor)
    .def("registerProcessorWidget",
         static_cast<void (InviwoModule::*)(std::unique_ptr<ProcessorWidgetFactoryObject>)>(
             &InviwoModule::registerProcessorWidget))
    .def("registerPortInspector", &InviwoModule::registerPortInspector)
    .def("registerDataVisualizer", &InviwoModule::registerDataVisualizer)

    .def("registerInport", &InviwoModule::registerInport)
    .def("registerOutport", &InviwoModule::registerOutport)
    .def("registerProperty",
         static_cast<void (InviwoModule::*)(std::unique_ptr<PropertyFactoryObject>)>(
             &InviwoModule::registerProperty))
    .def("registerPropertyWidget",
         static_cast<void (InviwoModule::*)(std::unique_ptr<PropertyWidgetFactoryObject>)>(
             &InviwoModule::registerPropertyWidget))
    .def("registerSettings", &InviwoModule::registerSettings);
    */
}

}  // namespace inviwo
