/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include "pycamera.h"
#include <math.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <modules/python3/pythoninterface/pyvalueparser.h>
#include <modules/python3/pythoninterface/pythonparameterparser.h>
#include <inviwo/core/network/processornetwork.h>

namespace inviwo {

PyObject* py_setCameraFocus(PyObject* /*self*/, PyObject* args) {
    static PythonParameterParser tester;
    std::string path;
    vec3 focus;
    if (tester.parse(args, path, focus) == -1) {
        return nullptr;
    }

    Property* theProperty =
        InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(splitString(path, '.'));

    if (!theProperty) {
        std::string msg = std::string("setPropertyValue() no property with path: ") + path;
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }

    CameraProperty* cam = dynamic_cast<CameraProperty*>(theProperty);
    if (cam) {
        cam->setLookTo(focus);
        Py_RETURN_NONE;
    } else {
        std::string msg = std::string("setCameraFocus() not a camera property: ") +
                          theProperty->getClassIdentifier();
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }
}

PyObject* py_setCameraUp(PyObject* /*self*/, PyObject* args) {
    static PythonParameterParser tester;
    std::string path;
    vec3 up;
    if (tester.parse(args, path, up) == -1) {
        return nullptr;
    }

    Property* theProperty =
        InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(splitString(path, '.'));

    if (!theProperty) {
        std::string msg = std::string("setPropertyValue() no property with path: ") + path;
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }

    CameraProperty* cam = dynamic_cast<CameraProperty*>(theProperty);
    if (cam) {
        cam->setLookUp(up);
        Py_RETURN_NONE;
    } else {
        std::string msg = std::string("setCameraUp() not a camera property: ") +
                          theProperty->getClassIdentifier();
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }
}

PyObject* py_setCameraPos(PyObject* /*self*/, PyObject* args) {
    static PythonParameterParser tester;
    std::string path;
    vec3 from;
    if (tester.parse(args, path, from) == -1) {
        return nullptr;
    }

    Property* theProperty =
        InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(splitString(path, '.'));

    if (!theProperty) {
        std::string msg = std::string("setPropertyValue() no property with path: ") + path;
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }

    CameraProperty* cam = dynamic_cast<CameraProperty*>(theProperty);
    if (cam) {
        cam->setLookFrom(from);
        Py_RETURN_NONE;
    } else {
        std::string msg = std::string("setCameraPosition() not a camera property: ") +
                          theProperty->getClassIdentifier();
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }
}

}