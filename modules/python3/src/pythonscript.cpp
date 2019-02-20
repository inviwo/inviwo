/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <modules/python3/pythonscript.h>

#include <modules/python3/python3module.h>
#include <modules/python3/pythonexecutionoutputobservable.h>
#include <modules/python3/pythoninterpreter.h>
#include <modules/python3/pybindutils.h>

#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/common/inviwoapplication.h>

#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>

#include <traceback.h>
#include <frameobject.h>

#define BYTE_CODE static_cast<PyObject*>(byteCode_)

namespace inviwo {

PythonScript::PythonScript() : source_(""), byteCode_(nullptr), isCompileNeeded_(false) {}

PythonScript::~PythonScript() { Py_XDECREF(BYTE_CODE); }

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

bool PythonScript::run(std::function<void(pybind11::dict)> callback) {
    namespace py = pybind11;

    // Copy the dict to get a clean slate every time we run the script
    py::dict global = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    return run(global, callback);
}

bool PythonScript::run(std::unordered_map<std::string, pybind11::object> locals,
                       std::function<void(pybind11::dict)> callback) {
    namespace py = pybind11;

    // Copy the dict to get a clean slate every time we run the script
    py::dict global = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));
    for (auto& item : locals) {
        global[py::str(item.first)] = item.second;
    }
    return run(global, callback);
}

bool PythonScript::run(pybind11::dict locals, std::function<void(pybind11::dict)> callback) {
    namespace py = pybind11;

    if (isCompileNeeded_ && !compile()) {
        return false;
    }

    ivwAssert(byteCode_ != nullptr, "No byte code");

    auto ret = PyEval_EvalCode(BYTE_CODE, locals.ptr(), locals.ptr());
    if (ret) {
        if (callback) {
            callback(locals);
        }
        return true;
    } else {
        checkRuntimeError();
        return false;
    }
}

void PythonScript::setFilename(const std::string& filename) { filename_ = filename; }

const std::string& PythonScript::getFilename() const { return filename_; }

std::string PythonScript::getSource() const { return source_; }

void PythonScript::setSource(const std::string& source) {
    source_ = source;
    isCompileNeeded_ = true;
    Py_XDECREF(BYTE_CODE);
    byteCode_ = nullptr;
}

bool PythonScript::checkCompileError() {
    namespace py = pybind11;
    if (!PyErr_Occurred()) return true;

    std::stringstream errstr;
    errstr << "Compile Error occurred when compiling script " << filename_ << "\n";

    py::object type;
    py::object value;
    py::object traceback;
    PyErr_Fetch(&type.ptr(), &value.ptr(), &traceback.ptr());

    std::stringstream log;
    char* msg = nullptr;
    PyObject* obj = nullptr;

    if (PyArg_ParseTuple(value.ptr(), "sO", &msg, &obj)) {
        int line, col;
        char* code = nullptr;
        char* mod = nullptr;

        if (PyArg_ParseTuple(obj, "siis", &mod, &line, &col, &code)) {
            log << "[" << line << ":" << col << "] " << msg << ": " << code;
        }
    }

    // convert error to string, if it could not be parsed
    if (log.str().empty()) {
        log << std::string(py::str(value));
    }

    errstr << log.str();
    InviwoApplication::getPtr()
        ->getModuleByType<Python3Module>()
        ->getPythonInterpreter()
        ->pythonExecutionOutputEvent(errstr.str(), PythonOutputType::sysstderr);

    return false;
}

bool PythonScript::checkRuntimeError() {
    namespace py = pybind11;
    if (!PyErr_Occurred()) return true;

    py::object type;
    py::object value;
    py::object traceback;
    PyErr_Fetch(&type.ptr(), &value.ptr(), &traceback.ptr());

    PyErr_NormalizeException(&type.ptr(), &value.ptr(), &traceback.ptr());
    PyException_SetTraceback(value.ptr(), traceback.ptr());

    std::stringstream errstr;

    if (type) {
        errstr << type.attr("__name__").cast<std::string>() << ": ";
    }
    if (value) {
        errstr << std::string(py::str(value));
    } else {
        errstr << "<No data available>";
    }
    errstr << "\n";
    if (traceback) {
        errstr << "Stacktrace (most recent call first):\n";
        PyTracebackObject* tb = (PyTracebackObject*)traceback.ptr();

        /* Get the deepest trace possible */
        while (tb->tb_next) tb = tb->tb_next;

        PyFrameObject* frame = tb->tb_frame;
        while (frame) {
            int line = PyFrame_GetLineNumber(frame);
            auto file = py::handle(frame->f_code->co_filename).cast<std::string>();
            auto name = py::handle(frame->f_code->co_name).cast<std::string>();

            if (file.empty()) {
                file = "<script>";
            }

            errstr << file << ":" << line << " in " << name << "\n";
            frame = frame->f_back;
        }
    } else {
        errstr << "No stacktrace available";
    }

    InviwoApplication::getPtr()
        ->getModuleByType<Python3Module>()
        ->getPythonInterpreter()
        ->pythonExecutionOutputEvent(errstr.str(), PythonOutputType::sysstderr);
    return false;
}

PythonScriptDisk::PythonScriptDisk(const std::string& filename) : PythonScript() {
    setFilename(filename);
}

const inviwo::BaseCallBack* PythonScriptDisk::onChange(std::function<void()> callback) {
    return onChangeCallbacks_.addLambdaCallback(callback);
}

void PythonScriptDisk::removeOnChange(const BaseCallBack* callback) {
    onChangeCallbacks_.remove(callback);
}

void PythonScriptDisk::readFileAndSetSource() {
    auto inFile = filesystem::ifstream(getFilename());
    std::string src((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    setSource(src);
}

void PythonScriptDisk::setFilename(const std::string& filename) {
    PythonScript::setFilename(filename);
    if (!filename.empty()) {
        stopAllObservation();
        startFileObservation(filename);
    }
    readFileAndSetSource();
}

void PythonScriptDisk::fileChanged(const std::string&) {
    readFileAndSetSource();
    onChangeCallbacks_.invokeAll();
}

}  // namespace inviwo
