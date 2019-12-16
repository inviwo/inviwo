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

#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <warn/pop>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stringconversion.h>

#include <modules/python3/pythoninterpreter.h>
#include <modules/python3/python3module.h>
#include <modules/python3/pythonscript.h>
#include <modules/python3/pyutils.h>

namespace inviwo {

PythonInterpreter::PythonInterpreter() : embedded_{false}, isInit_(false) {
    namespace py = pybind11;

    if (isInit_) {
        throw ModuleInitException("Python already initialized", IVW_CONTEXT);
    }

    if (!Py_IsInitialized()) {

        LogInfo("Python version: " + toString(Py_GetVersion()));
        static wchar_t programName[] = L"PyInviwo";
        Py_SetProgramName(programName);

        try {
            py::initialize_interpreter(false);
        } catch (const std::exception& e) {
            throw ModuleInitException(e.what(), IVW_CONTEXT);
        }

        isInit_ = true;
        embedded_ = true;

        if (!Py_IsInitialized()) {
            throw ModuleInitException("Python is not Initialized", IVW_CONTEXT);
        }

#if defined(__unix__) || defined(__APPLE__)
        auto execpath = filesystem::getExecutablePath();
        auto folder = filesystem::getFileDirectory(execpath);
        addModulePath(folder);
#endif
#if defined(__APPLE__)
        // On OSX the path returned by getExecutablePath includes folder-paths inside the app-binary
        addModulePath(folder + "/../../../");
#endif

        try {
            py::exec(R"(
import sys
import inviwopy

class OutputRedirector:
    def __init__(self, type):
        self.type = type;
        self.buffer = "";

    def flush(self):
        pass


    def write(self, string):
        self.buffer += string
        
        if len(self.buffer) > 0 and self.buffer[-1] == "\n":
            inviwopy.handlePythonOutput(self.buffer[:-1], self.type)
            self.buffer = ""

sys.stdout = OutputRedirector(0)
sys.stderr = OutputRedirector(1)
)",
                     py::globals());
        } catch (const py::error_already_set& e) {
            throw ModuleInitException(e.what(), IVW_CONTEXT);
        }
    }
}

PythonInterpreter::~PythonInterpreter() {
    namespace py = pybind11;
    if (embedded_) {
        py::finalize_interpreter();
    }
}

void PythonInterpreter::addModulePath(const std::string& path) { pyutil::addModulePath(path); }

void PythonInterpreter::importModule(const std::string& moduleName) {
    namespace py = pybind11;

    auto dict = py::globals();
    dict[moduleName.c_str()] = py::module::import(moduleName.c_str());
}

bool PythonInterpreter::runString(std::string code) {
    auto ret = PyRun_SimpleString(code.c_str());
    return ret == 0;
}

}  // namespace inviwo
