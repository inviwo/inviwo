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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/util/filesystem.h>
#include <modules/python3/pythoninterface/pythonparameterparser.h>
#include <modules/python3/pythoninterface/pyvalueparser.h>
#include "pythonqtmethods.h"

#include <inviwo/qt/editor/inviwomainwindow.h>
#include <inviwo/qt/editor/networkeditor.h>
#include <inviwo/qt/widgets/inviwoapplicationqt.h>
#include <inviwo/qt/widgets/inviwoqtutils.h>
#include <inviwo/qt/widgets/properties/transferfunctionpropertywidgetqt.h>

#include <modules/python3/pyinviwo.h>
#include <QDir>
#include <QInputDialog>

namespace inviwo {

#include <warn/push>
#include <warn/ignore/missing-field-initializers>

static PyMethodDef Inviwo_QT_METHODS[] = {
    {"getPathCurrentWorkspace", py_getPathCurrentWorkspace, METH_VARARGS,
     "Return the path to the current loaded workspace."},
    {"loadWorkspace", py_loadWorkspace, METH_VARARGS, "Load a new workspace into the network."},
    {"saveWorkspace", py_saveWorkspace, METH_VARARGS, "Saves the current workspace."},
    {"quitInviwo", py_quitInviwo, METH_VARARGS, "Method to quit Inviwo."},
    {"prompt", py_prompt, METH_VARARGS, "Prompts the user for input."},
    {"update", py_update, METH_VARARGS, "Ask QT to update its widgets"},
    {"showTransferFunctionEditor", py_showTransferFunctionEditor, METH_VARARGS,
     "Show the transfer function editor for given transfer function property."},
    nullptr};

#include <warn/pop>

struct PyModuleDef Inviwo_QT_Module_Def = {PyModuleDef_HEAD_INIT,
                                           "inviwoqt",
                                           nullptr,
                                           -1,
                                           Inviwo_QT_METHODS,
                                           nullptr,
                                           nullptr,
                                           nullptr,
                                           nullptr};

void initPythonQT() { PyInviwo::getPtr()->registerPyModule(&Inviwo_QT_Module_Def, "inviwoqt"); }

PyObject* py_getPathCurrentWorkspace(PyObject* self, PyObject* args) {
    static PythonParameterParser tester;
    if (tester.parse(args) == -1) {
        return nullptr;
    }

    if (auto qt = dynamic_cast<InviwoApplicationQt*>(InviwoApplication::getPtr())) {
        if (auto mw = dynamic_cast<InviwoMainWindow*>(qt->getMainWindow())) {
            return PyValueParser::toPyObject(mw->getNetworkEditor()->getCurrentFilename());
        }
    }
    return nullptr;
}

PyObject* py_loadWorkspace(PyObject* self, PyObject* args) {
    static PythonParameterParser tester;
    std::string filename;
    if (tester.parse(args, filename) == -1) {
        return nullptr;
    }
    if (!filesystem::fileExists(filename)) {
        filename = filesystem::getPath(PathType::Workspaces) + "/" + filename;

        if (!filesystem::fileExists(filename)) {
            std::string msg = std::string("loadWorkspace() could not find file") + filename;
            PyErr_SetString(PyExc_TypeError, msg.c_str());
            return nullptr;
        }
    }

    if (auto qt = dynamic_cast<InviwoApplicationQt*>(InviwoApplication::getPtr())) {
        if (auto mw = dynamic_cast<InviwoMainWindow*>(qt->getMainWindow())) {
            mw->getNetworkEditor()->loadNetwork(filename);
        }
    }

    Py_RETURN_NONE;
}

PyObject* py_saveWorkspace(PyObject* self, PyObject* args) {
    static PythonParameterParser tester;
    std::string filename;
    if (tester.parse(args, filename) == -1) {
        return nullptr;
    }
    if (auto qt = dynamic_cast<InviwoApplicationQt*>(InviwoApplication::getPtr())) {
        if (auto mw = dynamic_cast<InviwoMainWindow*>(qt->getMainWindow())) {
            mw->getNetworkEditor()->saveNetwork(filename);
        }
    }

    Py_RETURN_NONE;
}

PyObject* py_quitInviwo(PyObject* self, PyObject* args) {
    static PythonParameterParser tester;
    if (tester.parse(args) == -1) {
        return nullptr;
    }
    if (auto qt = dynamic_cast<InviwoApplicationQt*>(InviwoApplication::getPtr())) {
        if (auto mw = dynamic_cast<InviwoMainWindow*>(qt->getMainWindow())) {
            mw->getNetworkEditor()->setModified(false);
            mw->close();
        }
    }
    Py_RETURN_NONE;
}

PyObject* py_prompt(PyObject* self, PyObject* args) {
    static PythonParameterParser tester(1);
    std::string title, message, defaultValue = "";
    if (tester.parse(args, title, message, defaultValue) == -1) {
        return nullptr;
    }

    bool ok;
    QString text = QInputDialog::getText(nullptr, title.c_str(), message.c_str(), QLineEdit::Normal,
                                         defaultValue.c_str(), &ok);
    if (ok && !text.isEmpty()) {
        std::string t = text.toLocal8Bit().constData();
        return PyValueParser::toPyObject(t);
    } else if (ok) {
        return PyValueParser::toPyObject(std::string(""));
    }
    Py_RETURN_NONE;
}

PyObject* py_update(PyObject* self, PyObject* args) {
    static PythonParameterParser tester;
    if (tester.parse(args) == -1) {
        return nullptr;
    }
    if (auto qt = dynamic_cast<InviwoApplicationQt*>(InviwoApplication::getPtr())) {
        qt->processEvents();
    }
    Py_RETURN_NONE;
}

PyObject* py_showTransferFunctionEditor(PyObject* self, PyObject* args) {
    static PythonParameterParser tester;

    std::string path;
    if (tester.parse(args, path) == -1) {
        return nullptr;
    }

    Property* theProperty =
        InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(splitString(path, '.'));

    if (!theProperty) {
        std::string msg =
            std::string("showTransferFunctionEditor() no property with path: ") + path;
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }

    if (!dynamic_cast<TransferFunctionProperty*>(theProperty)) {
        std::string msg =
            std::string("showTransferFunctionEditor() not a transfer function property: ") +
            theProperty->getClassIdentifier();
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }

    for (auto w : theProperty->getWidgets()) {
        auto tfw = dynamic_cast<TransferFunctionPropertyWidgetQt*>(w);
        if (tfw) {
            tfw->openTransferFunctionDialog();
        }
    }
    Py_RETURN_NONE;
}
}
