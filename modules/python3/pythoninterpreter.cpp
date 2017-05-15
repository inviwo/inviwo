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

#include <pybind11/pybind11.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/filesystem.h>
#include <modules/python3/pythoninterpreter.h>

#include <modules/python3/python3module.h>
#include <inviwo/core/util/filesystem.h>

#include <modules/python3/pythonscript.h>
#include <modules/python3/pythonexecutionoutputobservable.h>

#if defined(__unix__)
#include <unistd.h>
#elif defined(__APPLE__)
#include <libproc.h>  // proc_pidpath
#endif

namespace inviwo {

static PyObject* py_stdout(PyObject* /*self*/, PyObject* args) {
    char* msg;
    int len;
    int isStderr;

    if (!PyArg_ParseTuple(args, "s#i", &msg, &len, &isStderr)) {
        LogWarnCustom("inviwo.Python.py_print", "failed to parse log message");
    } else {
        if (len != 0) {
            if (!(len == 1 && (msg[0] == '\n' || msg[0] == '\r' || msg[0] == '\0')))
                if (auto app = InviwoApplication::getPtr()) {
                    if (auto module = app->getModuleByType<Python3Module>()) {
                        if (auto interpreter = module->getPythonInterpreter()) {
                            interpreter->pythonExecutionOutputEvent(
                                msg, (isStderr == 0) ? sysstdout : sysstderr);
                        }
                    }
                }
        }
    }

    Py_RETURN_NONE;
}

PythonInterpreter::PythonInterpreter(Python3Module* module) : isInit_(false) { init(module); }

PythonInterpreter::~PythonInterpreter() { Py_Finalize(); }

void PythonInterpreter::addModulePath(const std::string& path) {
    if (!Py_IsInitialized()) {
        LogWarn("addModulePath(): Python is not initialized");
        return;
    }

    std::string pathConv = path;
    replaceInString(pathConv, "\\", "/");
    std::string code = "import sys\n";
    code.append(std::string("sys.path.append('") + pathConv + std::string("')"));
    if (!runString(code)) {
        LogWarn("Failed to add '" + pathConv + "' to Python module search path");
    }
}

#include <warn/push>
#include <warn/ignore/missing-field-initializers>

static PyMethodDef Inviwo_Internals_METHODS[] = {
    {"ivwPrint", py_stdout, METH_VARARGS, "A simple example of an embedded function."}, nullptr};

#include <warn/pop>

struct PyModuleDef Inviwo_Internals_Module_Def = {PyModuleDef_HEAD_INIT,
                                                  "inviwo_internal",
                                                  nullptr,
                                                  -1,
                                                  Inviwo_Internals_METHODS,
                                                  nullptr,
                                                  nullptr,
                                                  nullptr,
                                                  nullptr};

void PythonInterpreter::init(Python3Module* module) {
    if (isInit_) return;

    isInit_ = true;
    LogInfo("Python version: " + toString(Py_GetVersion()));
    wchar_t programName[] = L"PyInviwo";
    Py_SetProgramName(programName);

    Py_InitializeEx(false);

    if (!Py_IsInitialized()) {
        LogError("Python is not Initialized");
        return;
    }

    PyEval_InitThreads();
    importModule("builtins");
    importModule("sys");

    dict_ = PyImport_GetModuleDict();

    PyObject* internalModule = PyModule_Create(&Inviwo_Internals_Module_Def);
    if (!internalModule) {
        LogError("Failed to init inviwo_internal");
    } else {
        PyDict_SetItemString(dict_, "inviwo_internal", internalModule);
    }

    addModulePath(module->getPath() + "/scripts");
    initOutputRedirector(module);

#if defined(__unix__)
    char executablePath[PATH_MAX];
    auto size = ::readlink("/proc/self/exe", executablePath, sizeof(executablePath) - 1);
    if (size != -1) {
        // readlink does not append a null character to the path
        executablePath[size] = '\0';
    } else {
        // Error retrieving path
        executablePath[0] = '\0';
    }

    std::string execpath(executablePath);
    auto folder = filesystem::getFileDirectory(execpath);
    addModulePath(folder);
#elif defined(__APPLE__)
    // http://stackoverflow.com/questions/799679/programatically-retrieving-the-absolute-path-of-an-os-x-command-line-app/1024933#1024933
    char executablePath[PATH_MAX];
    auto pid = getpid();
    if (proc_pidpath(pid, executablePath, sizeof(executablePath)) <= 0) {
        // Error retrieving path
        executablePath[0] = '\0';
    }
    std::string execpath(executablePath);
    auto folder = filesystem::getFileDirectory(execpath);
    addModulePath(folder);
    addModulePath(folder + "/../../../");

#endif
}

void PythonInterpreter::importModule(const std::string& moduleName) {
    auto mainDict = PyImport_GetModuleDict();
    const static std::string __key__ = "__";
    char* key = new char[moduleName.size() + 5];
    sprintf(key, "__%s__", moduleName.c_str());
    if (PyDict_GetItemString(mainDict, key) == nullptr) {
        PyObject* pMod = PyImport_ImportModule(moduleName.c_str());
        if (nullptr != pMod) {
            PyDict_SetItemString(mainDict, key, pMod);
        } else {
            LogWarn("Failed to import python module: " << moduleName);
        }
    }
    delete[] key;
}

bool PythonInterpreter::runString(std::string code) {
    auto ret = PyRun_SimpleString(code.c_str());
    return ret == 0;
}

void PythonInterpreter::initOutputRedirector(Python3Module* module) {
    std::string directorFileName = module->getPath() + "/scripts/outputredirector.py";

    if (!filesystem::fileExists(directorFileName)) {
        LogError("Could not open outputredirector.py");
        return;
    }

    PythonScriptDisk outputCatcher(directorFileName);
    if (!outputCatcher.run()) {
        LogWarn("Python init script failed to run");
    }
}

}  // namespace
