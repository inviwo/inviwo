/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <inviwopy/inviwopy.h>

#include <modules/python3/python3module.h>
#include <modules/python3/pybindutils.h>
#include <modules/python3/pythoninterpreter.h>

#include <inviwopy/pydataformat.h>
#include <inviwopy/pyinviwoapplication.h>
#include <inviwopy/pyinviwomodule.h>
#include <inviwopy/pyimage.h>
#include <inviwopy/pynetwork.h>
#include <inviwopy/pyprocessors.h>
#include <inviwopy/pyglmtypes.h>
#include <inviwopy/pyport.h>
#include <inviwopy/pyproperties.h>
#include <inviwopy/pypropertyowner.h>
#include <inviwopy/pyvolume.h>
#include <inviwopy/pydatamapper.h>
#include <inviwopy/pybuffer.h>
#include <inviwopy/pymesh.h>
#include <inviwopy/pytfprimitiveset.h>
#include <inviwopy/pypickingmapper.h>
#include <inviwopy/pylogging.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/commandlineparser.h>

#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/util/settings/settings.h>
#include <inviwo/core/util/exception.h>

namespace py = pybind11;

PYBIND11_MAKE_OPAQUE(std::vector<int>)
PYBIND11_MAKE_OPAQUE(std::vector<float>)
PYBIND11_MAKE_OPAQUE(std::vector<double>)

PYBIND11_MODULE(inviwopy, m) {

#ifdef IVW_ENABLE_MSVC_MEM_LEAK_TEST
    VLDDisable();
#endif

    using namespace inviwo;
    m.doc() = "Python interface for Inviwo";

    exposeGLMTypes(m);

    auto propertiesModule = m.def_submodule("properties", "Exposing various Inviwo Properties");
    auto dataModule =
        m.def_submodule("data", "Module containing class mapping to the Inviwo data structures");
    auto formatsModule =
        dataModule.def_submodule("formats", "Module containing the various data formats");

    exposeLogging(m);
    exposeInviwoApplication(m);
    exposeDataFormat(formatsModule);
    exposePropertyOwner(propertiesModule);
    exposeProperties(propertiesModule);
    exposePort(m);
    exposeProcessors(m);
    exposeNetwork(m);
    exposePickingMapper(m);

    exposeDataMapper(dataModule);
    exposeImage(dataModule);
    exposeVolume(dataModule);
    exposeBuffer(dataModule);
    exposeMesh(dataModule);
    exposeTFPrimitiveSet(dataModule);
    exposeInviwoModule(m);

    py::class_<Settings, PropertyOwner, std::unique_ptr<Settings, py::nodelete>>(m, "Settings");

    m.def("logInfo", [](const std::string& msg) { LogInfoCustom("inviwopy", msg); });
    m.def("logWarn", [](const std::string& msg) { LogWarnCustom("inviwopy", msg); });
    m.def("logError", [](const std::string& msg) { LogErrorCustom("inviwopy", msg); });
    m.def("debugBreak", []() { util::debugBreak(); });

    if (InviwoApplication::isInitialized()) {
        m.attr("app") = py::cast(InviwoApplication::getPtr(), py::return_value_policy::reference);
    }

    m.def("getApp", []() { return InviwoApplication::getPtr(); },
          py::return_value_policy::reference);

    m.def("handlePythonOutput", [](const std::string& msg, int isStderr) {
        if (auto module = InviwoApplication::getPtr()->getModuleByType<Python3Module>()) {
            if (auto interpreter = module->getPythonInterpreter()) {
                interpreter->pythonExecutionOutputEvent(msg, (isStderr == 0)
                                                                 ? PythonOutputType::sysstdout
                                                                 : PythonOutputType::sysstderr);
            }
        }
    });

#ifdef IVW_ENABLE_MSVC_MEM_LEAK_TEST
    VLDEnable();
#endif
}
