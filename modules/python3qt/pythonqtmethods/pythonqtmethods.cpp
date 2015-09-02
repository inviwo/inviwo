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

#include "pythonqtmethods.h"
#include <modules/python3/pythoninterface/pyvalueparser.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/properties/transferfunctionproperty.h>

#include <inviwo/qt/editor/networkeditor.h>
#include <inviwo/qt/widgets/inviwoapplicationqt.h>
#include <inviwo/qt/widgets/inviwoqtutils.h>
#include <inviwo/qt/widgets/properties/transferfunctionpropertywidgetqt.h>

#include <QInputDialog>
#include <QDir>

namespace inviwo {
    

    PyObject* py_getPathCurrentWorkspace(PyObject* /*self*/, PyObject* args) {
        if (!(PyTuple_Size(args) == 0)) {
            std::ostringstream errStr;
            utilqt::localizeStream(errStr);
            errStr << "loadWorkspace() takes no parameters";
            errStr << " (" << PyTuple_Size(args) << " given)";
            PyErr_SetString(PyExc_TypeError, errStr.str().c_str());
            return nullptr;
        }

        return PyValueParser::toPyObject(NetworkEditor::getPtr()->getCurrentFilename());
    }

        PyObject* py_loadWorkspace(PyObject* /*self*/, PyObject* args) {
        if (!(PyTuple_Size(args) == 1)) {
            std::ostringstream errStr;
            utilqt::localizeStream(errStr);
            errStr << "loadWorkspace() takes 1 argument: filename";
            errStr << " (" << PyTuple_Size(args) << " given)";
            PyErr_SetString(PyExc_TypeError, errStr.str().c_str());
            return nullptr;
        }

        // check parameter if is string
        if (!PyValueParser::is<std::string>(PyTuple_GetItem(args, 0))) {
            PyErr_SetString(PyExc_TypeError, "loadWorkspace() first argument must be a string");
            return nullptr;
        }

        std::string filename = PyValueParser::parse<std::string>(PyTuple_GetItem(args, 0));

        if (!filesystem::fileExists(filename)) {
            filename = InviwoApplication::getPtr()->getPath(InviwoApplication::PATH_MODULES) + "/" + filename;

            if (!filesystem::fileExists(filename)) {
                std::string msg = std::string("loadWorkspace() could not find file") + filename;
                PyErr_SetString(PyExc_TypeError, msg.c_str());
                return nullptr;
            }
        }

        NetworkEditor::getPtr()->loadNetwork(filename);
        Py_RETURN_NONE;
    }


    PyObject* py_saveWorkspace(PyObject* /*self*/, PyObject* args) {
        if (!(PyTuple_Size(args) == 1)) {
            std::ostringstream errStr;
            utilqt::localizeStream(errStr);
            errStr << "saveWorkspace() takes 1 argument: filename";
            errStr << " (" << PyTuple_Size(args) << " given)";
            PyErr_SetString(PyExc_TypeError, errStr.str().c_str());
            return nullptr;
        }

        // check parameter if is string
        if (!PyValueParser::is<std::string>(PyTuple_GetItem(args, 0))) {
            PyErr_SetString(PyExc_TypeError, "saveWorkspace() first argument must be a string");
            return nullptr;
        }

        std::string filename = PyValueParser::parse<std::string>(PyTuple_GetItem(args, 0));

        NetworkEditor::getPtr()->saveNetwork(filename);
        Py_RETURN_NONE;
    }


PyObject* py_quitInviwo(PyObject* /*self*/, PyObject* /*args*/) {
    NetworkEditor::getPtr()->setModified(false);
    static_cast<InviwoApplicationQt*>(InviwoApplication::getPtr())->getMainWindow()->close();
    Py_RETURN_NONE;
}

PyObject* py_prompt(PyObject* /*self*/, PyObject* args) {
    PyPromptMethod p;
    if (!p.testParams(args)) {
        return nullptr;
    }
    std::string title = std::string(PyValueParser::parse<std::string>(PyTuple_GetItem(args, 0)));
    std::string message = std::string(PyValueParser::parse<std::string>(PyTuple_GetItem(args, 1)));
    std::string defaultValue = "";

    size_t size = static_cast<size_t>(PyTuple_Size(args));
    if (size == 3) {
        defaultValue = std::string(PyValueParser::parse<std::string>(PyTuple_GetItem(args, 2)));
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


PyObject* py_showTransferFunctionEditor(PyObject* /*self*/, PyObject* args) {
    PyShowPropertyWidgetMethod p;
    if (!p.testParams(args)) {
        return nullptr;
    }
    std::string path = std::string(PyValueParser::parse<std::string>(PyTuple_GetItem(args, 0)));
    Property* theProperty = InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(splitString(path, '.'));

    if (!theProperty) {
        std::string msg = std::string("showTransferFunctionEditor() no property with path: ") + path;
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }

    if (!dynamic_cast<TransferFunctionProperty*>(theProperty)){
        std::string msg = std::string("showTransferFunctionEditor() not a transfer function property: ") + theProperty->getClassIdentifier();
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;

    }


    for (auto w : theProperty->getWidgets()){
        auto tfw = dynamic_cast<TransferFunctionPropertyWidgetQt*>(w);
        if (tfw){
            tfw->openTransferFunctionDialog();
        }
    }
    Py_RETURN_NONE;
}

}
