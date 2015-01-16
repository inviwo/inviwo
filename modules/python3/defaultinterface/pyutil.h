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

#ifndef IVW_PYSNAPSHOTMEHTODINVIWO_H
#define IVW_PYSNAPSHOTMEHTODINVIWO_H

#include <modules/python3/python3moduledefine.h>

#include <modules/python3/pythoninterface/pymethod.h>

namespace inviwo {

PyObject* py_snapshot(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_snapshotCanvas(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_getBasePath(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_getDataPath(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_getWorkspaceSavePath(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_getVolumePath(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_getImagePath(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_getModulePath(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_getTransferFunctionPath(PyObject* /*self*/, PyObject* /*args*/);

PyObject* py_getMemoryUsage(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_clearResourceManager(PyObject* /*self*/, PyObject* /*args*/);

PyObject* py_disableEvaluation(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_enableEvaluation(PyObject* /*self*/, PyObject* /*args*/);


class IVW_MODULE_PYTHON3_API PySnapshotMethod : public PyMethod {
public:
    PySnapshotMethod();
    virtual ~PySnapshotMethod() {}
    virtual std::string getName() const { return "snapshot"; }
    virtual std::string getDesc() const {
        return "Saves a snapshot of the specified canvas to the given file. If no canvas name is "
               "passed, the first canvas in the network is chosen.";
    }
    virtual PyCFunction getFunc() { return py_snapshot; }

private:
    PyParamString filename_;
    PyParamString canvas_;
};

class IVW_MODULE_PYTHON3_API PySnapshotCanvasMethod : public PyMethod {
public:
    PySnapshotCanvasMethod();
    virtual ~PySnapshotCanvasMethod() {}
    virtual std::string getName() const { return "snapshotCanvas"; }
    virtual std::string getDesc() const {
        return "Saves a snapshot of the ith canvas to the given file.";
    }
    virtual PyCFunction getFunc() { return py_snapshotCanvas; }

private:
    PyParamInt canvasID_;
    PyParamString filename_;
};

class IVW_MODULE_PYTHON3_API PyGetBasePathMethod : public PyMethod {
public:
    virtual std::string getName() const { return "getBasePath"; }
    virtual std::string getDesc() const { return "Returns the path to Inviwos base folder."; }
    virtual PyCFunction getFunc() { return py_getBasePath; }
};

class IVW_MODULE_PYTHON3_API PyGetDataPathMethod : public PyMethod {
public:
    virtual std::string getName() const { return "getDataPath"; }
    virtual std::string getDesc() const { return "Returns the path to Inviwos data folder."; }
    virtual PyCFunction getFunc() { return py_getDataPath; }
};

class IVW_MODULE_PYTHON3_API PyGetWorkspaceSavePathMethod : public PyMethod {
public:
    virtual std::string getName() const { return "getWorkspaceSavePatht"; }
    virtual std::string getDesc() const { return "Returns the path to Inviwos workspace folder."; }
    virtual PyCFunction getFunc() { return py_getWorkspaceSavePath; }
};

class IVW_MODULE_PYTHON3_API PyGetVolumePathMethod : public PyMethod {
public:
    virtual std::string getName() const { return "getVolumePath"; }
    virtual std::string getDesc() const { return "Returns the path to Inviwos volume folder."; }
    virtual PyCFunction getFunc() { return py_getVolumePath; }
};

class IVW_MODULE_PYTHON3_API PyGetImagePathMethod : public PyMethod {
public:
    virtual std::string getName() const { return "getImagePath"; }
    virtual std::string getDesc() const { return "Returns the path to Inviwos image folder."; }
    virtual PyCFunction getFunc() { return py_getImagePath; }
};

class IVW_MODULE_PYTHON3_API PyGetModulePathMethod : public PyMethod {
public:
    virtual std::string getName() const { return "getModulePath"; }
    virtual std::string getDesc() const { return "Returns the path to Inviwo module folder."; }
    virtual PyCFunction getFunc() { return py_getModulePath; }
};

class IVW_MODULE_PYTHON3_API PyGetTransferFunctionPath : public PyMethod {
public:
    virtual std::string getName() const { return "getTransferFunctionPath"; }
    virtual std::string getDesc() const {
        return "Returns the path to Inviwo transfer function folder.";
    }
    virtual PyCFunction getFunc() { return py_getTransferFunctionPath; }
};

class IVW_MODULE_PYTHON3_API PyGetMemoryUsage : public PyMethod {
public:
    virtual std::string getName() const { return "getMemoryUsage"; }
    virtual std::string getDesc() const {
        return "Return how big Inviwo's current RAM working set is.";
    }
    virtual PyCFunction getFunc() { return py_getMemoryUsage; }
};

class IVW_MODULE_PYTHON3_API PyClearResourceManage : public PyMethod {
public:
    virtual std::string getName() const { return "clearResourceManager"; }
    virtual std::string getDesc() const { return "Method to clear Inviwo's resource manager."; }
    virtual PyCFunction getFunc() { return py_clearResourceManager; }
};

class IVW_MODULE_PYTHON3_API PyDisableEvaluation : public PyMethod {
public:
    virtual std::string getName() const { return "beginUpdate"; }
    virtual std::string getDesc() const {
        return "Method to disable evaluation of Inviwo's network.";
    }
    virtual PyCFunction getFunc() { return py_disableEvaluation; }
};

class IVW_MODULE_PYTHON3_API PyEnableEvaluation : public PyMethod {
public:
    virtual std::string getName() const { return "endUpdate"; }
    virtual std::string getDesc() const {
        return "Method to re-enable evaluation of Inviwo's network.";
    }
    virtual PyCFunction getFunc() { return py_enableEvaluation; }
};


}  // namespace

#endif  // IVW_PYSNAPSHOTMEHTODINVIWO_H
