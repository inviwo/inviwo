/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2026 Inviwo Foundation
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

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include <modules/python3/python3module.h>
#include <modules/python3/pybindutils.h>
#include <modules/python3/pybindmodule.h>
#include <modules/python3/pythoninterpreter.h>
#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

#include <inviwopy/pybitset.h>
#include <inviwopy/pybuffer.h>
#include <inviwopy/pycamera.h>
#include <inviwopy/pycameraproperty.h>
#include <inviwopy/pycompositeproperties.h>
#include <inviwopy/pydataformat.h>
#include <inviwopy/pydatamapper.h>
#include <inviwopy/pydatareaders.h>
#include <inviwopy/pydatawriters.h>
#include <inviwopy/pydocument.h>
#include <inviwopy/pyevent.h>
#include <inviwopy/pyglmmattypes.h>
#include <inviwopy/pyglmports.h>
#include <inviwopy/pyglmtypes.h>
#include <inviwopy/pyimage.h>
#include <inviwopy/pyimagetypes.h>
#include <inviwopy/pyinviwoapplication.h>
#include <inviwopy/pyinviwomodule.h>
#include <inviwopy/pylayer.h>
#include <inviwopy/pylogging.h>
#include <inviwopy/pymesh.h>
#include <inviwopy/pynetwork.h>
#include <inviwopy/pypickingmapper.h>
#include <inviwopy/pyport.h>
#include <inviwopy/pyprocessors.h>
#include <inviwopy/pyproperties.h>
#include <inviwopy/pypropertyowner.h>
#include <inviwopy/pyserialization.h>
#include <inviwopy/pyspatialdata.h>
#include <inviwopy/pytfprimitiveset.h>
#include <inviwopy/pyvolume.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/moduleutils.h>
#include <inviwo/core/util/commandlineparser.h>

#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/util/settings/settings.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/assertion.h>

namespace py = pybind11;

INVIWO_PYBIND_MODULE(inviwopy, m) {
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

    // Since we have a "global" std::string type here bind_vector will create module local bindings
    // for StringVector. But since we have included it in opaquetypes we need to create a
    // definition of this in each python module that uses it, unless we are linking statically.
    // See https://pybind11.readthedocs.io/en/stable/advanced/classes.html#module-local
    // and https://pybind11.readthedocs.io/en/stable/advanced/cast/stl.html#binding-stl-containers
    // for more details.
    py::bind_vector<std::vector<std::string>, py::smart_holder>(m, "StringVector");
    py::implicitly_convertible<py::list, std::vector<std::string>>();

    // For these cases I have used module_local(false) to make them available globally
    // Not sure if that is a good idea though?
    py::bind_vector<std::vector<float>, py::smart_holder>(glmModule, "floatVector",
                                                          py::module_local(false));
    py::bind_vector<std::vector<double>, py::smart_holder>(glmModule, "doubleVector",
                                                           py::module_local(false));
    py::bind_vector<std::vector<int>, py::smart_holder>(glmModule, "intVector",
                                                        py::module_local(false));
    py::bind_vector<std::vector<unsigned int>, py::smart_holder>(glmModule, "uintVector",
                                                                 py::module_local(false));
    // making them global means that we can find them in for example inviwo-unittests-python
    // when we do a py::cast(std::vector<int>) for example. That will fail if the type is not found
    // and we have the declared PYBIND11_MAKE_OPAQUE(std::vector<int>).
    // hence at the moment py::cast(std::vector<std::string>) will not work there.

    py::implicitly_convertible<py::buffer, std::vector<float>>();
    py::implicitly_convertible<py::buffer, std::vector<double>>();
    py::implicitly_convertible<py::buffer, std::vector<int>>();
    py::implicitly_convertible<py::buffer, std::vector<unsigned int>>();

    // Note the order is important here, we need to load all base classes before any derived clases
    exposeGLMTypes(glmModule);
    exposeGLMMatTypes(glmModule);

    exposeDocument(m);

    exposeLogging(m);
    exposeInviwoApplication(m);
    exposeDataFormat(formatsModule);
    exposeBitset(dataModule);
    exposeSerialization(m);
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
    exposeSpatialData(dataModule);
    exposeImageTypes(dataModule);
    exposeLayer(dataModule);
    exposeImage(dataModule);
    exposeVolume(dataModule);
    exposeBuffer(dataModule);
    exposeMesh(dataModule);
    exposeCamera(dataModule);
    exposeInviwoModule(m);
    exposeCameraProperty(m, propertiesModule);
    exposeDataReaders(m);
    exposeDataWriters(m);

    py::classh<Settings, PropertyOwner>(m, "Settings")
        .def("load", &Settings::load)
        .def("save", &Settings::save);

    m.def("debugBreak", []() { util::debugBreak(); });

    if (InviwoApplication::isInitialized()) {
        m.attr("app") = py::cast(InviwoApplication::getPtr(), py::return_value_policy::reference);
    }

    m.def(
        "getApp", []() { return InviwoApplication::getPtr(); }, py::return_value_policy::reference);

    m.def("handlePythonOutput", [](const std::string& msg, int isStderr) {
        if (auto* pythonModule = util::getModuleByType<Python3Module>()) {
            if (auto* interpreter = pythonModule->getPythonInterpreter()) {
                interpreter->pythonExecutionOutputEvent(msg, (isStderr == 0)
                                                                 ? PythonOutputType::sysstdout
                                                                 : PythonOutputType::sysstderr);
            }
        }
    });
}
