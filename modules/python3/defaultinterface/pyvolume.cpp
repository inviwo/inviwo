/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <modules/python3/pythonincluder.h>

#include "pyvolume.h"

#include <modules/python3/pythoninterface/pyvalueparser.h>
#include <modules/python3/pythoninterface/pythonparameterparser.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/network/processornetwork.h>
#include <modules/opengl/canvasprocessorgl.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/util/filesystem.h>

namespace inviwo {

    TransferFunctionProperty *getTF(std::string path,std::string functionName) {
        Property* theProperty =
            InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(splitString(path, '.'));

        if (!theProperty) {
            std::string msg = std::string("setPropertyValue() no property with path: ") + path;
            PyErr_SetString(PyExc_TypeError, msg.c_str());
            return nullptr;
        }

        TransferFunctionProperty* tf = dynamic_cast<TransferFunctionProperty*>(theProperty);

        if (!tf) {
            std::string msg =
                std::string(functionName + "() no transfer function property at path: ") + ", (" +
                path + " is of type " + theProperty->getClassIdentifier() + ")";
            PyErr_SetString(PyExc_TypeError, msg.c_str());
            return nullptr;
        }
        return tf;
    }

PyObject* py_saveTransferFunction(PyObject* /*self*/, PyObject* args) {
    static PythonParameterParser tester;
    std::string path;
    std::string filename;
    if (tester.parse(args,path,filename) == -1) {
        return nullptr;
    }

    auto tf = getTF(path,"saveTransferFunction");
    if (!tf) { return nullptr; }

    Serializer serializer(filename);
    tf->serialize(serializer);
    serializer.writeFile();
    Py_RETURN_NONE;
}

PyObject* py_loadTransferFunction(PyObject* /*self*/, PyObject* args) {
    static PythonParameterParser tester;
    std::string path;
    std::string filename;
    if (tester.parse(args, path, filename) == -1) {
        return nullptr;
    }

    auto tf = getTF(path, "loadTransferFunction");
    if (!tf) { return nullptr; }

    auto app = InviwoApplication::getPtr();

    if (!filesystem::fileExists(filename)) {
        if (filesystem::fileExists(app->getPath(PathType::TransferFunctions, "/" + filename))) {
            filename = app->getPath(PathType::TransferFunctions, "/" + filename);
        } else {
            std::string msg = "loadTransferFunction() file not found (" + filename + ")";
            PyErr_SetString(PyExc_TypeError, msg.c_str());
            return nullptr;
        }
    }

    Deserializer deserializer(app, filename);
    tf->deserialize(deserializer);

    Py_RETURN_NONE;
}

PyObject* py_clearTransferfunction(PyObject* /*self*/, PyObject* args) {
    static PythonParameterParser tester;
    std::string path;
    if (tester.parse(args, path) == -1) {
        return nullptr;
    }

    auto tf = getTF(path, "clearTransferfunction");
    if (!tf) { return nullptr; }
    
    tf->get().clearPoints();
    Py_RETURN_NONE;
}

PyObject* py_addPointTransferFunction(PyObject* /*self*/, PyObject* args) {
    static PythonParameterParser tester;
    std::string path;
    vec2 pos;
    vec3 color;
    if (tester.parse(args, path, pos, color) == -1) {
        return nullptr;
    }

    auto tf = getTF(path, "addPointTransferFunction");
    if (!tf) { return nullptr; }
    
    tf->get().addPoint(pos, vec4(color, pos.y));
    tf->setPropertyModified(true);
    Py_RETURN_NONE;
}

}
