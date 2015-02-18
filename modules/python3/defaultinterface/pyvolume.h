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

#ifndef IVW_PYVOLUMEINVIWO_H
#define IVW_PYVOLUMEINVIWO_H



#include <modules/python3/python3moduledefine.h>

#include <modules/python3/pythoninterface/pymethod.h>

namespace inviwo {


PyObject* py_saveTransferFunction(PyObject* /*self*/, PyObject* args);
PyObject* py_loadTransferFunction(PyObject* /*self*/, PyObject* args);

PyObject* py_clearTransferfunction(PyObject* /*self*/, PyObject* args);
PyObject* py_addPointTransferFunction(PyObject* /*self*/, PyObject* args);



class IVW_MODULE_PYTHON3_API PySaveTransferFunction : public PyMethod {
public:
    PySaveTransferFunction();
    virtual ~PySaveTransferFunction() {}

    virtual std::string getName()const {return "saveTransferFunction";}
    virtual std::string getDesc()const {return "Save a transfer function to file from the specified transfer function property.";}
    virtual PyCFunction getFunc() {return py_saveTransferFunction;}

private:
    PyParamString path_;
    PyParamString filename_;
};



class IVW_MODULE_PYTHON3_API PyLoadTransferFunction : public PyMethod {
public:
    PyLoadTransferFunction();
    virtual ~PyLoadTransferFunction() {}

    virtual std::string getName()const {return "loadTransferFunction";}
    virtual std::string getDesc()const {return "Load a transfer function from file into the specified transfer function property.";}
    virtual PyCFunction getFunc() {return py_loadTransferFunction;}

private:
    PyParamString path_;
    PyParamString filename_;
};



class IVW_MODULE_PYTHON3_API PyClearTransferfunction : public PyMethod {
public:
    PyClearTransferfunction();
    virtual ~PyClearTransferfunction() {}

    virtual std::string getName()const {return "clearTransferfunction";}
    virtual std::string getDesc()const {return "Clears a transfer function.";}
    virtual PyCFunction getFunc() {return py_clearTransferfunction;}

private:
    PyParamString path_;
};


class IVW_MODULE_PYTHON3_API PyAddTransferFunction : public PyMethod {
public:
    PyAddTransferFunction();
    virtual ~PyAddTransferFunction() {}

    virtual std::string getName()const {return "addPointToTransferFunction";}
    virtual std::string getDesc()const {return "Load a transfer function from file into the specified transfer function property.";}
    virtual PyCFunction getFunc() {return py_addPointTransferFunction;}

private:
    PyParamString path_;
    PyParamVec2 pos_;
    PyParamVec3 color_;
};


} //namespace



#endif // IVW_PYSNAPSHOTMEHTODINVIWO_H


