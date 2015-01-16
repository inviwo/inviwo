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

#ifndef IVW_PROPERTIESMEHTODSINVIWO_H
#define IVW_PROPERTIESMEHTODSINVIWO_H



#include <modules/python3/python3moduledefine.h>

#include <modules/python3/pythoninterface/pymethod.h>


namespace inviwo {

PyObject* py_setPropertyValue(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_setPropertyMaxValue(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_setPropertyMinValue(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_getPropertyValue(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_getPropertyMaxValue(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_getPropertyMinValue(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_clickButton(PyObject* /*self*/, PyObject* /*args*/);

class IVW_MODULE_PYTHON3_API PySetPropertyValueMethod : public PyMethod {
public:
    PySetPropertyValueMethod();
    virtual ~PySetPropertyValueMethod() {}

    virtual std::string getName()const {return "setPropertyValue";}
    virtual std::string getDesc()const {return "Assigns a value to a processor property. The value has to be passed as scalar or tuple, depending on the property's cardinality. Camera properties take a 3-tuple of 3-tuples, containing the position, focus and up vectors. Option properties expect an option key.";}
    virtual PyCFunction getFunc() {return py_setPropertyValue;}
private:
    PyParamString path_;
    PyParamVarious value_;
};

class IVW_MODULE_PYTHON3_API PySetPropertyMaxValueMethod : public PyMethod {
public:
    PySetPropertyMaxValueMethod();
    virtual ~PySetPropertyMaxValueMethod() {}

    virtual std::string getName()const {return "setPropertyMaxValue";}
    virtual std::string getDesc()const {return "Defines the max value for a property.";}
    virtual PyCFunction getFunc() {return py_setPropertyMaxValue;}
private:
    PyParamString path_;
    PyParamVarious maxValue_;
};

class IVW_MODULE_PYTHON3_API PySetPropertyMinValueMethod : public PyMethod {
public:
    PySetPropertyMinValueMethod();
    virtual ~PySetPropertyMinValueMethod() {}

    virtual std::string getName()const {return "setPropertyMinValue";}
    virtual std::string getDesc()const {return "Defines the min value for a property.";}
    virtual PyCFunction getFunc() {return py_setPropertyMinValue;}
private:
    PyParamString path_;
    PyParamVarious minValue_;
};

class IVW_MODULE_PYTHON3_API PyGetPropertyValueMethod : public PyMethod {
public:
    PyGetPropertyValueMethod();
    virtual ~PyGetPropertyValueMethod() {}

    virtual std::string getName()const {return "getPropertyValue";}
    virtual std::string getDesc()const {return "Returns the current value of a processor property (scalar or tuple).";}
    virtual PyCFunction getFunc() {return py_getPropertyValue;}
private:
    PyParamString path_;
};

class IVW_MODULE_PYTHON3_API PyGetPropertyMaxValueMethod : public PyMethod {
public:
    PyGetPropertyMaxValueMethod();
    virtual ~PyGetPropertyMaxValueMethod() {}

    virtual std::string getName()const {return "getPropertyMaxValue";}
    virtual std::string getDesc()const {return "Returns the max value for a property (scalar or tuple).";}
    virtual PyCFunction getFunc() {return py_getPropertyMaxValue;}
private:
    PyParamString path_;
};

class IVW_MODULE_PYTHON3_API PyGetPropertyMinValueMethod : public PyMethod {
public:
    PyGetPropertyMinValueMethod();
    virtual ~PyGetPropertyMinValueMethod() {}

    virtual std::string getName()const {return "getPropertyMinValue";}
    virtual std::string getDesc()const {return "Returns the min value for a property (scalar or tuple).";}
    virtual PyCFunction getFunc() {return py_getPropertyMinValue;}
private:
    PyParamString path_;
};

class IVW_MODULE_PYTHON3_API PyClickButtonMethod : public PyMethod {
public:
    PyClickButtonMethod();
    virtual ~PyClickButtonMethod() {}

    virtual std::string getName()const {return "clickButton";}
    virtual std::string getDesc()const {return "Simulates a click on a button property.";}
    virtual PyCFunction getFunc() {return py_clickButton;}
private:
    PyParamString path_;
};

} //namespace



#endif // IVW_PROPERTIESMEHTODSINVIWO_H


