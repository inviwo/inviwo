/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2023 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <warn/pop>

#include <modules/python3/python3module.h>
#include <modules/python3/pybindutils.h>
#include <modules/python3/pythoninterpreter.h>

#include <inviwopy/pybitset.h>
#include <inviwopy/pydataformat.h>
#include <inviwopy/pydatareaders.h>
#include <inviwopy/pydatawriters.h>
#include <inviwopy/pyinviwoapplication.h>
#include <inviwopy/pyinviwomodule.h>
#include <inviwopy/pydocument.h>
#include <inviwopy/pyimage.h>
#include <inviwopy/pynetwork.h>
#include <inviwopy/pyprocessors.h>
#include <inviwopy/pyglmtypes.h>
#include <inviwopy/pyglmmattypes.h>
#include <inviwopy/pyglmports.h>
#include <inviwopy/pyport.h>
#include <inviwopy/pycompositeproperties.h>
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
#include <inviwo/core/util/assertion.h>

namespace py = pybind11;

#ifdef INVIWO_ALL_DYN_LINK
PYBIND11_MODULE(inviwopy, m) {
#else
// PYBIND11_EMBEDDED_MODULE(inviwopy, m) {

static ::pybind11::module_::module_def pybind11_module_def_inviwopy;
static void pybind11_init_inviwopy(::pybind11::module_&);
static PyObject* pybind11_init_wrapper_inviwopy() {
    auto m = ::pybind11::module_::create_extension_module("inviwopy", nullptr,
                                                          &pybind11_module_def_inviwopy);
    try {
        pybind11_init_inviwopy(m);
        return m.ptr();
    } catch (pybind11::error_already_set& e) {
        pybind11::raise_from(e, PyExc_ImportError, "initialization failed");
        return nullptr;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_ImportError, e.what());
        return nullptr;
    }
}
extern "C" PyObject* pybind11_init_impl_inviwopy();
extern "C" PyObject* pybind11_init_impl_inviwopy() { return pybind11_init_wrapper_inviwopy(); }
//::pybind11::detail::embedded_module pybind11_module_inviwopy("inviwopy",
//                                                             pybind11_init_impl_inviwopy);

namespace inviwo {
void initInviwoPy() {
    if (Py_IsInitialized() != 0) {
        pybind11::pybind11_fail("Can't add new modules after the interpreter has been initialized");
    }

    auto result = PyImport_AppendInittab("inviwopy", pybind11_init_impl_inviwopy);
    if (result == -1) {
        pybind11::pybind11_fail("Insufficient memory to add a new module");
    }
}
}  // namespace inviwo

void pybind11_init_inviwopy(::pybind11::module_& m) {

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
            doc
            
        )doc";

    auto glmModule = m.def_submodule("glm", "GML vec and mat types");
    auto propertiesModule = m.def_submodule("properties", "Inviwo Properties");
    auto dataModule = m.def_submodule("data", "Inviwo Data Structures");
    auto formatsModule = dataModule.def_submodule("formats", "Inviwo Data Formats");

    // Note the order is important here, we need to load all base classes before any derived clases
    exposeGLMTypes(glmModule);
    exposeGLMMatTypes(glmModule);

    exposeDocument(m);

    exposeLogging(m);
    exposeInviwoApplication(m);
    exposeDataFormat(formatsModule);
    exposeBitset(dataModule);
    exposeTFPrimitiveSet(dataModule);  // defines TFPrimitiveData used in exposeProperties
    exposeProperties(propertiesModule);
    exposePropertyOwner(propertiesModule);
    exposeCompositeProperties(propertiesModule);

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
    exposeDataReaders(m);
    exposeDataWriters(m);

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
}
