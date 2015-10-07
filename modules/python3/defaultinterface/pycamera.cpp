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
#include <inviwo/core/network/processornetwork.h>

namespace inviwo {

PyObject* py_setCameraFocus(PyObject* /*self*/, PyObject* args) {
    static PySetCameraFocusMethod p;

    if (!p.testParams(args)) return nullptr;

    std::string path = std::string(PyValueParser::parse<std::string>(PyTuple_GetItem(args, 0)));

    Property* theProperty = InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(splitString(path, '.'));

    if (!theProperty) {
        std::string msg = std::string("setPropertyValue() no property with path: ") + path;
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }

    CameraProperty* cam = dynamic_cast<CameraProperty*>(theProperty);
    if (cam) {
        vec3 focus;
        char* dummy1;
        int d1;

        if (!PyArg_ParseTuple(args, "s#(fff)", &dummy1, &d1,
                              &focus.x,&focus.y,&focus.z
                             )) {
            std::string msg = std::string("setCameraFocus() Failed to parse values for camera, needs to be on the format: (posX,posY,posZ) ") +
                              path;
            PyErr_SetString(PyExc_TypeError, msg.c_str());
            return nullptr;
        }

        cam->setLookTo(focus);
        Py_RETURN_NONE;
    } else {
        std::string msg = std::string("setCameraFocus() not a camera property: ") + theProperty->getClassIdentifier();
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }
}

PyObject* py_setCameraUp(PyObject* /*self*/, PyObject* args) {
    static PySetCameraUpMethod p;

    if (!p.testParams(args)) return nullptr;

    std::string path = std::string(PyValueParser::parse<std::string>(PyTuple_GetItem(args, 0)));

    Property* theProperty = InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(splitString(path, '.'));

    if (!theProperty) {
        std::string msg = std::string("setPropertyValue() no property with path: ") + path;
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }

    CameraProperty* cam = dynamic_cast<CameraProperty*>(theProperty);
    if (cam) {
        vec3 up;
        char* dummy1;
        int d1;

        if (!PyArg_ParseTuple(args,"s#(fff)", &dummy1,&d1,
                              &up.x,&up.y,&up.z
                             )) {
            std::string msg = std::string("setCameraUp() Failed to parse values for camera, needs to be on the format: (dirX,dirY,dirZ) ") + path;
            PyErr_SetString(PyExc_TypeError, msg.c_str());
            return nullptr;
        }

        cam->setLookUp(up);
        Py_RETURN_NONE;
    } else {
        std::string msg = std::string("setCameraUp() not a camera property: ") + theProperty->getClassIdentifier();
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }
}



PyObject* py_setCameraPos(PyObject* /*self*/, PyObject* args) {
    static PySetCameraPosMethod p;

    if (!p.testParams(args)) return nullptr;

    std::string path = std::string(PyValueParser::parse<std::string>(PyTuple_GetItem(args, 0)));
    Property* theProperty = InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(splitString(path, '.'));
    if (!theProperty) {
        std::string msg = std::string("setPropertyValue() no property with path: ") + path;
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }


    CameraProperty* cam = dynamic_cast<CameraProperty*>(theProperty);
    if (cam) {
        vec3 from;
        char* dummy1;
        int d1;

        if (!PyArg_ParseTuple(args, "s#(fff)", &dummy1, &d1,
                              &from.x,&from.y,&from.z
                             )) {
            std::string msg = std::string("setPropertyValue() Failed to parse values for camera, needs to be on the format: (posX,posY,posZ) ") +
                              path;
            PyErr_SetString(PyExc_TypeError, msg.c_str());
            return nullptr;
        }

       cam->setLookFrom(from);
        Py_RETURN_NONE;
    } else {
        std::string msg = std::string("setCameraPosition() not a camera property: ") + theProperty->getClassIdentifier();
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }
}


PySetCameraFocusMethod::PySetCameraFocusMethod()
    : path_("path")
    , focusPoint_("focusPoint")
{
    addParam(&path_);
    addParam(&focusPoint_);
}

PySetCameraUpMethod::PySetCameraUpMethod()
    : path_("path")
    , upVector_("upVector")
{
    addParam(&path_);
    addParam(&upVector_);
}

PySetCameraPosMethod::PySetCameraPosMethod()
    : path_("path")
    , position_("position")
{
    addParam(&path_);
    addParam(&position_);
}

}