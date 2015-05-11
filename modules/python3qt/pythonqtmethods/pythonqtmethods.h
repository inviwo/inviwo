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

#ifndef IVW_PYTONQTFUNCTIONS_H
#define IVW_PYTONQTFUNCTIONS_H

#include <modules/python3qt/python3qtmoduledefine.h>
#include <modules/python3/pythoninterface/pymethod.h>

namespace inviwo {

    PyObject* py_getPathCurrentWorkspace(PyObject* /*self*/, PyObject* /*args*/);
    PyObject* py_loadWorkspace(PyObject* /*self*/, PyObject* /*args*/);
    PyObject* py_saveWorkspace(PyObject* /*self*/, PyObject* /*args*/);
    PyObject* py_quitInviwo(PyObject* /*self*/, PyObject* /*args*/);
    PyObject* py_prompt(PyObject* /*self*/, PyObject* /*args*/);
    PyObject* py_showTransferFunctionEditor(PyObject* /*self*/, PyObject* /*args*/);

    class IVW_MODULE_PYTHON3QT_API PyGetPathCurrentWorkspace : public PyMethod {
    public:
        std::string getName() const { return "getPathCurrentWorkspace"; }
        std::string getDesc() const { return "Return the path to the current loaded workspace."; }
        virtual PyCFunction getFunc() { return py_getPathCurrentWorkspace; }
    };

    class IVW_MODULE_PYTHON3QT_API PyLoadNetworkMethod : public PyMethod {
    public:
        std::string getName() const { return "loadWorkspace"; }
        std::string getDesc() const { return "Load a new workspace into the network."; }
        virtual PyCFunction getFunc() { return py_loadWorkspace; }
    };

    class IVW_MODULE_PYTHON3QT_API PySaveNetworkMethod : public PyMethod {
    public:
        std::string getName() const { return "saveWorkspace"; }
        std::string getDesc() const { return "Saves the current workspace."; }
        virtual PyCFunction getFunc() { return py_saveWorkspace; }
    };


class IVW_MODULE_PYTHON3QT_API PyQuitInviwoMethod : public PyMethod {
public:
    virtual std::string getName() const { return "quit"; }
    virtual std::string getDesc() const { return "Method to quit Inviwo."; }
    virtual PyCFunction getFunc() { return py_quitInviwo; }
};


class IVW_MODULE_PYTHON3QT_API PyPromptMethod : public PyMethod {
public:
    PyPromptMethod()
        : title_("title")
        , message_("message")
        , default_("default", true)
    {
        addParam(&title_);
        addParam(&message_);
        addParam(&default_);
    }

    virtual std::string getName() const { return "prompt"; }
    virtual std::string getDesc() const { return "Prompts the user for input."; }
    virtual PyCFunction getFunc() { return py_prompt; }

protected:
    PyParamString title_;
    PyParamString message_;
    PyParamString default_;
};



class IVW_MODULE_PYTHON3QT_API PyShowPropertyWidgetMethod: public PyMethod {
public:
    PyShowPropertyWidgetMethod()
        : path_("path")
    {
        addParam(&path_);
    }

    virtual std::string getName() const { return "showTransferFunctionEditor"; }
    virtual std::string getDesc() const { return "Show the transfer function editor for given transfer function property."; }
    virtual PyCFunction getFunc() { return py_showTransferFunctionEditor; }

protected:
    PyParamString path_;
};


}

#endif