/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2021 Inviwo Foundation
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
#include <inviwopy/pyglmmattypes.h>
#include <inviwopy/pyglmports.h>
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
#include <inviwopy/pyevent.h>
#include <inviwopy/pycamera.h>
#include <inviwopy/pycameraproperty.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/commandlineparser.h>

#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/util/settings/settings.h>
#include <inviwo/core/util/exception.h>

namespace py = pybind11;

PYBIND11_MODULE(inviwopy, m) {

#ifdef IVW_ENABLE_MSVC_MEM_LEAK_TEST
    VLDDisable();
#endif

    using namespace inviwo;
    m.doc() = R"doc(
        Core API
        
        All the bindings for the Inviwo core classes, Processor, Property, Port etc.
         
        .. rubric:: Modules
        
        .. autosummary::
            :toctree: .
            
            data
            data.formats
            glm
            properties
            camerautil
            
        )doc";

    auto glmModule = m.def_submodule("glm", "GML vec and mat types");
    auto propertiesModule = m.def_submodule("properties", "Inviwo Properties");
    auto dataModule = m.def_submodule("data", "Inviwo Data Structures");
    auto formatsModule = dataModule.def_submodule("formats", "Inviwo Data Formats");

    // Note the order is important here, we need to load all base classes before any derived clases
    exposeGLMTypes(glmModule);
    exposeGLMMatTypes(glmModule);

    exposeLogging(m);
    exposeInviwoApplication(m);
    exposeDataFormat(formatsModule);
    exposePropertyOwner(propertiesModule);
    exposeTFPrimitiveSet(dataModule);  // defines TFPrimitiveData used in exposeProperties
    exposeProperties(propertiesModule);
    exposePort(m);

    exposeProcessors(m);
    exposeNetwork(m);
    exposeEvents(m);
    exposePickingMapper(m);

    exposeGLMPorts(m);
    exposeDataMapper(dataModule);
    exposeImage(dataModule);
    exposeVolume(dataModule);
    exposeBuffer(dataModule);
    exposeMesh(dataModule);
    exposeCamera(dataModule);
    exposeInviwoModule(m);
    exposeCameraProperty(m, propertiesModule);

    py::class_<Settings, PropertyOwner>(m, "Settings");

    m.def("debugBreak", []() { util::debugBreak(); });

    if (InviwoApplication::isInitialized()) {
        m.attr("app") = py::cast(InviwoApplication::getPtr(), py::return_value_policy::reference);
    }

    m.def(
        "getApp", []() { return InviwoApplication::getPtr(); }, py::return_value_policy::reference);

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
