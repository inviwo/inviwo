/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2016 Inviwo Foundation
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

#include <modules/python3/pyinviwo.h>
#include <modules/python3/pythoninterface/pyvalueparser.h>
#include <modules/python3/pythoninterface/pythonparameterparser.h>


#include <warn/push>
#include <warn/ignore/all>
#include <QInputDialog>
#include <QCoreApplication>
#include <warn/pop>


namespace inviwo {

#include <warn/push>
#include <warn/ignore/missing-field-initializers>

static PyMethodDef Inviwo_QT_METHODS[] = {
    {"prompt", py_prompt, METH_VARARGS, "Prompts the user for input."},
    {"update", py_update, METH_VARARGS, "Ask QT to update its widgets"},
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
    QCoreApplication::instance()->processEvents();
    Py_RETURN_NONE;
}

}
