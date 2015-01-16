/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#ifndef IVW_PYCAMERAMEHTODINVIWO_H
#define IVW_PYCAMERAMEHTODINVIWO_H


#include <modules/python3/python3moduledefine.h>

#include <modules/python3/pythoninterface/pymethod.h>

namespace inviwo {
PyObject* py_setCameraFocus(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_setCameraUp(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_setCameraPos(PyObject* /*self*/, PyObject* /*args*/);


class IVW_MODULE_PYTHON3_API PySetCameraFocusMethod : public PyMethod {
public:
    PySetCameraFocusMethod();
    virtual ~PySetCameraFocusMethod() {}

    virtual std::string getName()const {return "setCameraFocus";}
    virtual std::string getDesc()const {return "Function to set the cameras focal point.";}
    virtual PyCFunction getFunc() {return py_setCameraFocus;}
private:
    PyParamString path_;
    PyParamVec3   focusPoint_;
};

class IVW_MODULE_PYTHON3_API PySetCameraUpMethod : public PyMethod {
public:
    PySetCameraUpMethod();
    virtual ~PySetCameraUpMethod() {}

    virtual std::string getName()const {return "setCameraUp";}
    virtual std::string getDesc()const {return "Function to set the cameras up direction.";}
    virtual PyCFunction getFunc() {return py_setCameraUp;}
private:
    PyParamString path_;
    PyParamVec3   upVector_;

};

class IVW_MODULE_PYTHON3_API PySetCameraPosMethod : public PyMethod {
public:
    PySetCameraPosMethod();
    virtual ~PySetCameraPosMethod() {}

    virtual std::string getName()const {return "setCameraPosition";}
    virtual std::string getDesc()const {return "Function to set the cameras position.";}
    virtual PyCFunction getFunc() {return py_setCameraPos;}
private:
    PyParamString path_;
    PyParamVec3   position_;

};

} //namespace





#endif // IVW_PYCAMERAMEHTODINVIWO_H


