/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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

#include <pybind11/cast.h>  // for cast, handle::cast, object:...

#include <inviwo/core/common/inviwoapplication.h>             // for InviwoApplication
#include <inviwo/core/common/inviwoapplicationutil.h>         // for getInviwoApplication
#include <inviwo/core/util/assertion.h>                       // for ivwAssert
#include <inviwo/core/util/callback.h>                        // for CallBackList, BaseCallBack
#include <inviwo/core/util/fileobserver.h>                    // for FileObserver
#include <inviwo/core/util/filesystem.h>                      // for ifstream
#include <modules/python3/python3module.h>                    // for Python3Module
#include <modules/python3/pythonexecutionoutputobservable.h>  // for PythonOutputType, PythonOut...
#include <modules/python3/pythoninterpreter.h>                // for PythonInterpreter

#include <fstream>   // for operator<<, basic_ostream
#include <iterator>  // for istreambuf_iterator
#include <utility>   // for pair

#include <fmt/format.h>
#include <fmt/std.h>

namespace inviwo {

PythonScript::PythonScript(std::string_view source, std::string_view name)
    : source_{source}, name_{name}, byteCode_(nullptr), isCompileNeeded_(true) {}

PythonScript PythonScript::fromFile(const std::filesystem::path& path) {
    auto file = std::ifstream(path);
    const std::string source{std::istreambuf_iterator<char>(file),
                             std::istreambuf_iterator<char>()};
    return PythonScript{source, path.generic_string()};
}

PythonScript::~PythonScript() { Py_XDECREF(static_cast<PyObject*>(byteCode_)); }

void PythonScript::setName(std::string_view name) { name_ = name; }

const std::string& PythonScript::getName() const { return name_; }

void PythonScript::setSource(std::string_view source) {
    source_ = source;
    isCompileNeeded_ = true;
    Py_XDECREF(static_cast<PyObject*>(byteCode_));
    byteCode_ = nullptr;
}

const std::string& PythonScript::getSource() const { return source_; }

bool PythonScript::compile() {
    Py_XDECREF(static_cast<PyObject*>(byteCode_));
    byteCode_ = Py_CompileString(source_.c_str(), name_.c_str(), Py_file_input);
    isCompileNeeded_ = !checkCompileError();

    if (isCompileNeeded_) {
        Py_XDECREF(static_cast<PyObject*>(byteCode_));
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

    IVW_ASSERT(byteCode_ != nullptr, "No byte code");

    if (PyEval_EvalCode(static_cast<PyObject*>(byteCode_), locals.ptr(), locals.ptr())) {
        if (callback) {
            callback(locals);
        }
        return true;
    } else {
        checkRuntimeError();
        return false;
    }
}

std::string PythonScript::getAndFormatError() {
    namespace py = pybind11;
    py::object type;
    py::object value;
    py::object traceback;
    PyErr_Fetch(&type.ptr(), &value.ptr(), &traceback.ptr());

    PyErr_NormalizeException(&type.ptr(), &value.ptr(), &traceback.ptr());
    PyException_SetTraceback(value.ptr(), traceback.ptr());

    std::stringstream err;

    if (type) {
        err << type.attr("__name__").cast<std::string>() << ": ";
    }
    if (value) {
        err << std::string(py::str(value));
    } else {
        err << "<No data available>";
    }
    err << "\n";
    if (traceback) {
        auto* tb = reinterpret_cast<PyTracebackObject*>(traceback.ptr());
        err << "Stacktrace (most recent call first):\n";

        /* Get the deepest trace possible */
        while (tb->tb_next) tb = tb->tb_next;

        PyFrameObject* frame = tb->tb_frame;
        while (frame) {
            int line = PyFrame_GetLineNumber(frame);
            PyCodeObject* code = PyFrame_GetCode(frame);

            auto file = py::handle(code->co_filename).cast<std::string>();
            auto name = py::handle(code->co_name).cast<std::string>();

            if (file.empty()) {
                file = "<script>";
            }

            err << file << ":" << line << " in " << name << "\n";
            frame = PyFrame_GetBack(frame);
        }
    } else {
        err << "No stacktrace available";
    }

    return std::move(err).str();
}

bool PythonScript::checkCompileError() {
    if (!PyErr_Occurred()) return true;

    const auto error = getAndFormatError();

    InviwoApplication::getPtr()
        ->getModuleByType<Python3Module>()
        ->getPythonInterpreter()
        ->pythonExecutionOutputEvent(
            fmt::format("Compile Error occurred when compiling script {}\n{}", name_, error),
            PythonOutputType::sysstderr);

    return false;
}

bool PythonScript::checkRuntimeError() {
    if (!PyErr_Occurred()) return true;

    auto error = getAndFormatError();

    InviwoApplication::getPtr()
        ->getModuleByType<Python3Module>()
        ->getPythonInterpreter()
        ->pythonExecutionOutputEvent(error, PythonOutputType::sysstderr);
    return false;
}

}  // namespace inviwo
