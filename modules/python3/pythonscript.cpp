/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2017 Inviwo Foundation
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

#include "pythonscript.h"
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/filesystem.h>

#include <modules/python3/pythonexecutionoutputobservable.h>
#include <modules/python3/pythoninterpreter.h>

#include <modules/python3/pybindutils.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <modules/python3/python3module.h>

#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>

#include <traceback.h>
#include <frameobject.h>

#define BYTE_CODE static_cast<PyObject*>(byteCode_)

namespace inviwo {

PythonScript::PythonScript() : source_(""), byteCode_(nullptr), isCompileNeeded_(false) {}

    PythonScript::~PythonScript() {
        Py_XDECREF(BYTE_CODE);
    }

    bool PythonScript::compile() {
        Py_XDECREF(BYTE_CODE);
        byteCode_ = Py_CompileString(source_.c_str(), filename_.c_str(), Py_file_input);
        isCompileNeeded_ = !checkCompileError();

        if (isCompileNeeded_) {
            Py_XDECREF(BYTE_CODE);
            byteCode_ = nullptr;
        }

        return !isCompileNeeded_;
    }

    bool PythonScript::run(const VariableMap& extraLocalVariables,
        std::function<void(pybind11::dict)> callback) {

        if (isCompileNeeded_ && !compile()) {
            LogError("Failed to run script, script could not be compiled");
            return false;
        }

        ivwAssert(byteCode_ != nullptr, "No byte code");

        auto m = PyImport_AddModule("__main__");
        if (m == NULL) return false;

        PyObject* copy = PyDict_Copy(PyModule_GetDict(m));
        for (auto ea : extraLocalVariables) {
            PyDict_SetItemString(copy, ea.first.c_str(), ea.second.ptr());
        }

        PyObject* ret = PyEval_EvalCode(BYTE_CODE, copy, copy);

        bool success = checkRuntimeError();
        if (success) {
            callback(pyutil::toPyBindObjectBorrow<pybind11::dict>(copy));
        }

        Py_XDECREF(ret);
        Py_XDECREF(copy);
        return success;
    }

    void PythonScript::setFilename(std::string filename) {
        filename_ = filename;
    }

    std::string PythonScript::getSource() const {
        return source_;
    }

    void PythonScript::setSource(const std::string& source) {
        source_ = source;
        isCompileNeeded_ = true;
        Py_XDECREF(BYTE_CODE);
        byteCode_ = nullptr;
    }

    bool PythonScript::checkCompileError() {
        if (!PyErr_Occurred())
            return true;

        LogError("Compile Error occured when compiling script " << filename_ << ", see below for info");

        PyObject* errtype, *errvalue, *traceback;
        PyErr_Fetch(&errtype, &errvalue, &traceback);
        std::string log = "";
        char* msg = nullptr;
        PyObject* obj = nullptr;

        if (PyArg_ParseTuple(errvalue, "sO", &msg, &obj)) {
            int line, col;
            char* code = nullptr;
            char* mod = nullptr;

            if (PyArg_ParseTuple(obj, "siis", &mod, &line, &col, &code)) {
                log = "[" + toString(line) + ":" + toString(col) + "] " + toString(msg) + ": " + toString(code);
            }
        }

        // convert error to string, if it could not be parsed
        if (log.empty()) {
            LogWarn("Failed to parse exception, printing as string:");
            auto s = pyutil::toPyBindObjectSteal<pybind11::str>(PyObject_Str(errvalue));
            if (pybind11::isinstance<pybind11::str>(s)) {
                log = s;
            }
        }

        Py_XDECREF(errtype);
        Py_XDECREF(errvalue);
        Py_XDECREF(traceback);
        LogError(log);
        return false;
    }

    bool PythonScript::checkRuntimeError() {
        if (!PyErr_Occurred())
            return true;

        LogError("Runtime Error occured when runing script " << filename_ << ", see below for info");

        std::string pyException = "";
        PyObject* pyError_type = nullptr;
        PyObject* pyError_value = nullptr;
        PyObject* pyError_traceback = nullptr;
        PyErr_Fetch(&pyError_type, &pyError_value, &pyError_traceback);
        int errorLine = -1;
        std::string stacktraceStr;

        if (pyError_traceback) {
            PyTracebackObject* traceback = (PyTracebackObject*)pyError_traceback;

            while (traceback) {
                PyFrameObject* frame = traceback->tb_frame;
                std::string stacktraceLine;

                if (frame && frame->f_code) {
                    PyCodeObject* codeObject = frame->f_code;

                    auto co_filename = pyutil::toPyBindObjectBorrow<pybind11::str>(codeObject->co_filename);
                    if (co_filename)
                        stacktraceLine.append(std::string("  File \"") + std::string(co_filename) + std::string("\", "));

                    errorLine = PyCode_Addr2Line(codeObject, frame->f_lasti);
                    stacktraceLine.append(std::string("line ") + toString(errorLine));

                    auto co_name = pyutil::toPyBindObjectBorrow<pybind11::str>(codeObject->co_name);
                    if (co_name)
                        stacktraceLine.append(std::string(", in ") + std::string(co_name));
                }

                stacktraceLine.append("\n");
                stacktraceStr = stacktraceLine + stacktraceStr;
                traceback = traceback->tb_next;
            }
        }

        std::stringstream s;
        s << errorLine;
        pyException.append(std::string("[") + s.str() + std::string("] "));

        if (pyError_value){
            auto pyError_string = pyutil::toPyBindObjectSteal<pybind11::str>(PyObject_Str(pyError_value));
            if(pyError_string){
                pyException.append(pyError_string);
            }
        }
        else {
            pyException.append("<No data available>");
        }

        pyException.append("\n");

        // finally append stacktrace string
        if (!stacktraceStr.empty()) {
            pyException.append("Stacktrace (most recent call first):\n");
            pyException.append(stacktraceStr);
        } else {
            pyException.append("<No stacktrace available>");
            LogWarn("Failed to parse traceback");
        }

        Py_XDECREF(pyError_type);
        Py_XDECREF(pyError_value);
        Py_XDECREF(pyError_traceback);
        LogError(pyException);
        InviwoApplication::getPtr()
            ->getModuleByType<Python3Module>()
            ->getPythonInterpreter()
            ->pythonExecutionOutputEvent(pyException, sysstderr);
        return false;
    }

    PythonScriptDisk::PythonScriptDisk(std::string filename)
        : PythonScript(), SingleFileObserver(filename) {
        setFilename(filename);
        onChange([this]() { readFileAndSetSource(); });
        readFileAndSetSource();
    }

    void PythonScriptDisk::readFileAndSetSource() {
        std::ifstream inFile(getFilename());
        std::string src((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
        setSource(src);
    }

} // namespace

