/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#include <modules/python3/interface/inviwopy.h>

#include <modules/python3/python3module.h>
#include <modules/python3/pybindutils.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/properties/propertyowner.h>

#include <modules/python3/interface/pyimage.h>
#include <modules/python3/interface/pynetwork.h>
#include <modules/python3/interface/pyprocessors.h>
#include <modules/python3/interface/pyglmtypes.h>
#include <modules/python3/interface/pyproperties.h>
#include <modules/python3/interface/pyvolume.h>
#include <modules/python3/interface/pybuffer.h>
#include <modules/python3/interface/pymesh.h>

#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/util/settings/settings.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/formats.h>

#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/logcentral.h>

namespace py = pybind11;

namespace inviwo {

struct DataFormatHelper {
    template <typename DataFormat>
    auto operator()(pybind11::module &m) {
        m.attr(("Data" + DataFormat::str()).c_str()) =
            py::cast(static_cast<const DataFormatBase *>(DataFormat::get()),
                     py::return_value_policy::reference);
    }
};
}

PYBIND11_MAKE_OPAQUE(std::vector<int>);
PYBIND11_MAKE_OPAQUE(std::vector<float>);
PYBIND11_MAKE_OPAQUE(std::vector<double>);

PYBIND11_PLUGIN(inviwopy) {

#ifdef IVW_ENABLE_MSVC_MEM_LEAK_TEST
    VLDDisable();
#endif

    using namespace inviwo;
    py::module m("inviwopy", "Python interface for Inviwo");

    exposeGLMTypes(m);

    auto propertiesModule = m.def_submodule("properties", "Exposing various Inviwo Properties");
    auto dataModule =
        m.def_submodule("data", "Module containing class mapping to the Inviwo data structures");
    auto formatsModule =
        dataModule.def_submodule("formats", "Module containing the various data formats");

    auto getModules = [](InviwoApplication *app) {
        std::vector<InviwoModule *> modules;
        for (auto &mod : app->getModules()) {
            modules.push_back(mod.get());
        }
        return modules;
    };

    py::class_<InviwoApplication>(m, "InviwoApplication")
        .def("getPath", &InviwoApplication::getPath, py::arg("pathType"), py::arg("suffix") = "",
             py::arg("createFolder") = false)
        .def("getModuleByIdentifier", &InviwoApplication::getModuleByIdentifier,
             py::return_value_policy::reference)
        .def("getModuleSettings", &InviwoApplication::getModuleSettings,
             py::return_value_policy::reference)
        .def("waitForPool", &InviwoApplication::waitForPool)
        .def("closeInviwoApplication", &InviwoApplication::closeInviwoApplication)
        .def("getOutputPath",
             [](InviwoApplication *app) { return app->getCommandLineParser().getOutputPath(); })
        .def_property_readonly("network", &InviwoApplication::getProcessorNetwork,
                               py::return_value_policy::reference)
        .def_property_readonly("basePath", &InviwoApplication::getBasePath)
        .def_property_readonly("displayName", &InviwoApplication::getDisplayName)
        .def_property_readonly("modules", getModules, py::return_value_policy::reference)
        .def_property_readonly("processorFactory", &InviwoApplication::getProcessorFactory,
                               py::return_value_policy::reference)
        .def_property_readonly("propertyFactory", &InviwoApplication::getPropertyFactory,
                               py::return_value_policy::reference);

    py::class_<InviwoModule>(m, "InviwoModule")
        .def_property_readonly("identifier", &InviwoModule::getIdentifier)
        .def_property_readonly("description", &InviwoModule::getDescription)
        .def_property_readonly("path", [](InviwoModule *m) { return m->getPath(); })
        .def_property_readonly("version", &InviwoModule::getVersion)
        .def("getPath", [](InviwoModule *m, ModulePath type) { return m->getPath(type); });

    py::class_<PropertyOwner, std::unique_ptr<PropertyOwner, py::nodelete>>(m, "PropertyOwner")
        .def("getPath", &PropertyOwner::getPath)
        .def_property_readonly("properties", &PropertyOwner::getProperties,
                               py::return_value_policy::reference)
        .def("__getattr__", &getPropertyById<PropertyOwner>, py::return_value_policy::reference)
        .def("getPropertiesRecursive", &PropertyOwner::getPropertiesRecursive)
        .def("addProperty",
             [](PropertyOwner &po, Property *pr) { po.addProperty(pr->clone(), true); })
        .def("getPropertyByIdentifier", &PropertyOwner::getPropertyByIdentifier,
             py::return_value_policy::reference, py::arg("identifier"),
             py::arg("recursiveSearch") = false)
        .def("getPropertyByPath", &PropertyOwner::getPropertyByPath,
             py::return_value_policy::reference)
        .def("size", &PropertyOwner::size)
        //.def("setValid", &PropertyOwner::setValid)
        //.def("getInvalidationLevel", &PropertyOwner::getInvalidationLevel)
        .def("invalidate",
             [](PropertyOwner *po) { po->invalidate(InvalidationLevel::InvalidOutput); })
        .def_property_readonly("processor", [](PropertyOwner &p) { return p.getProcessor(); },
                               py::return_value_policy::reference)
        .def("setAllPropertiesCurrentStateAsDefault",
             &PropertyOwner::setAllPropertiesCurrentStateAsDefault)
        .def("resetAllPoperties", &PropertyOwner::resetAllPoperties);

    py::class_<DataFormatBase>(formatsModule, "DataFormat")
        .def_property_readonly("size", &DataFormatBase::getSize)
        .def_property_readonly("components", &DataFormatBase::getComponents)
        .def_property_readonly("precision", &DataFormatBase::getPrecision)
        //.def_property_readonly("numericType", &DataFormatBase::getNumericTypve)
        //.def_property_readonly("id", &DataFormatBase::getId)
        .def_property_readonly("max", &DataFormatBase::getMax)
        .def_property_readonly("min", &DataFormatBase::getMin)
        .def_property_readonly("lowest", &DataFormatBase::getLowest)
        .def_property_readonly("__str__", &DataFormatBase::getString);

    util::for_each_type<DefaultDataFormats>{}(DataFormatHelper{}, formatsModule);

    exposeNetwork(m);
    exposeProcessors(m);
    exposeProperties(propertiesModule);
    exposeImage(dataModule);
    exposeVolume(dataModule);
    exposeBuffer(dataModule);
    exposeMesh(dataModule);

    py::class_<Settings, PropertyOwner, std::unique_ptr<Settings, py::nodelete>>(m, "Settings");

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

    m.def("logInfo", [](std::string msg) { LogInfoCustom("inviwopy", msg); });
    m.def("logWarn", [](std::string msg) { LogWarnCustom("inviwopy", msg); });
    m.def("logError", [](std::string msg) { LogErrorCustom("inviwopy", msg); });

    try {
        if (auto app = util::getInviwoApplication()) {
            m.attr("app") = py::cast(app, py::return_value_policy::reference);

            auto module = app->getModuleByType<Python3Module>();
            if (module) {
                setInviwopyModule(module,m);
            }
        }
    } catch (SingletonException &) {
        LogErrorCustom("inviwopy", "Failed to get Inviwo application, inviwopy will not function as expected");
    } catch (...) {
        throw;
    }

#ifdef IVW_ENABLE_MSVC_MEM_LEAK_TEST
    VLDEnable();
#endif

    return m.ptr();
}

namespace inviwo {
void setInviwopyModule(Python3Module *ivwmodule, pybind11::module pymodule) {
    ivwmodule->setInviwopyModule(std::make_shared<pybind11::module>(pymodule));
}
}