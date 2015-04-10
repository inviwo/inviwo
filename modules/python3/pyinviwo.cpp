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
#include <modules/python3/pyinviwo.h>

#include <modules/python3/python3module.h>

#include <modules/python3/pythonscript.h>
#include <modules/python3/pythonexecutionoutputobservable.h>

#include <modules/python3/pythoninterface/pymodule.h>

#include <modules/python3/pyinviwoobserver.h>

#include <modules/python3/defaultinterface/pyproperties.h>
#include <modules/python3/defaultinterface/pycamera.h>
#include <modules/python3/defaultinterface/pycanvas.h>
#include <modules/python3/defaultinterface/pylist.h>
#include <modules/python3/defaultinterface/pyutil.h>
#include <modules/python3/defaultinterface/pyvolume.h>

namespace inviwo {
static PyObject* py_stdout(PyObject* /*self*/, PyObject* args);
class PyStdOutCatcher : public PyMethod {
public:
    virtual std::string getName() const override { return "ivwPrint"; }
    virtual std::string getDesc() const override {
        return " Only for internal use. Redirect std output to python editor widget.";
    }
    virtual PyCFunction getFunc() override { return py_stdout; }
};

static PyObject* py_stdout(PyObject* /*self*/, PyObject* args) {
    char* msg;
    int len;
    int isStderr;

    if (!PyArg_ParseTuple(args, "s#i", &msg, &len, &isStderr)) {
        LogWarnCustom("inviwo.Python.py_print", "failed to parse log message");
    } else {
        if (len != 0) {
            if (!(len == 1 && (msg[0] == '\n' || msg[0] == '\r' || msg[0] == '\0')))
                PythonExecutionOutputObservable::getPtr()->pythonExecutionOutputEvent(
                    msg, (isStderr == 0) ? sysstdout : sysstderr);
        }
    }

    Py_RETURN_NONE;
}

PyInviwo::PyInviwo()
    : isInit_(false)
    , inviwoPyModule_(nullptr)
    , inviwoInternalPyModule_(nullptr)
    , mainDict_(nullptr)
    , modulesDict_(nullptr) {
    init(this);

    initPythonCInterface();
}

PyInviwo::~PyInviwo() {
    delete inviwoPyModule_;
    delete inviwoInternalPyModule_;
}

void PyInviwo::registerPyModule(PyModule* pyModule) {
    if (Py_IsInitialized()) {
        struct PyModuleDef moduleDef = {PyModuleDef_HEAD_INIT, pyModule->getModuleName(), nullptr,
                                        -1, pyModule->getPyMethodDefs()};

        PyObject* obj = PyModule_Create(&moduleDef);

        if (!obj) {
            LogWarn("Failed to init python module '" << pyModule->getModuleName() << "' ");
        }
        PyDict_SetItemString(modulesDict_, pyModule->getModuleName(), obj);
        PyDict_SetItemString(mainDict_, pyModule->getModuleName(), obj);

        pyModule->setPyObject(obj);
        registeredModules_.push_back(pyModule);

        for (ObserverSet::reverse_iterator it = observers_->rbegin(); it != observers_->rend();
             ++it) {
            static_cast<PyInviwoObserver*>(*it)->onModuleRegistered(pyModule);
        }
    } else {
        LogError("Python environment not initialized");
    }
}

void PyInviwo::addModulePath(const std::string& path) {
    if (!Py_IsInitialized()) {
        LogWarn("addModulePath(): not initialized");
        return;
    }

    std::string pathConv = path;
    replaceInString(pathConv, "\\", "/");
    std::string runString = "import sys\n";
    runString.append(std::string("sys.path.append('") + pathConv + std::string("')"));
    int ret = PyRun_SimpleString(runString.c_str());

    if (ret != 0) LogWarn("Failed to add '" + pathConv + "' to Python module search path");
}

void PyInviwo::initPythonCInterface() {
    if (isInit_) return;

    isInit_ = true;
    LogInfo("Python version: " + toString(Py_GetVersion()));
    wchar_t programName[] = L"PyInviwo";
    Py_SetProgramName(programName);
#ifdef WIN32
    Py_NoSiteFlag = 1;
#endif
    Py_InitializeEx(false);

    if (!Py_IsInitialized()) {
        LogError("Python is not Initialized");
        return;
    }

    PyEval_InitThreads();
    mainDict_ = PyDict_New();
    modulesDict_ = PyImport_GetModuleDict();
    importModule("builtins");
    importModule("sys");
    importModule("os");
    importModule("glob");
    importModule("random");


    addModulePath(InviwoApplication::getPtr()->getBasePath() + "/modules/python3/scripts");

    initDefaultInterfaces();

    initOutputRedirector();
}

void PyInviwo::importModule(const std::string &moduleName){
    const static std::string __key__ = "__";
    char * key = new char[moduleName.size() + 5];
    sprintf(key,"__%s__", moduleName.c_str());
    if (PyDict_GetItemString(mainDict_, key) == nullptr) {
        PyObject* pMod = PyImport_ImportModule(moduleName.c_str());
        if (nullptr != pMod) {
            PyDict_SetItemString(mainDict_, key, pMod);
        }
        else{
            LogWarn("Failed to import python module: " << moduleName);
        }
    }
    delete [] key;
}

void PyInviwo::initDefaultInterfaces() {
    inviwoInternalPyModule_ = new PyModule("inviwo_internal");
    inviwoInternalPyModule_->addMethod(new PyStdOutCatcher());

    inviwoPyModule_ = new PyModule("inviwo");
    inviwoPyModule_->addMethod(new PySetPropertyValueMethod());
    inviwoPyModule_->addMethod(new PySetPropertyMaxValueMethod());
    inviwoPyModule_->addMethod(new PySetPropertyMinValueMethod());
    inviwoPyModule_->addMethod(new PyGetPropertyValueMethod());
    inviwoPyModule_->addMethod(new PyGetPropertyMaxValueMethod());
    inviwoPyModule_->addMethod(new PyGetPropertyMinValueMethod());
    inviwoPyModule_->addMethod(new PyClickButtonMethod());
    inviwoPyModule_->addMethod(new PySetCameraFocusMethod());
    inviwoPyModule_->addMethod(new PySetCameraUpMethod());
    inviwoPyModule_->addMethod(new PySetCameraPosMethod());
    inviwoPyModule_->addMethod(new PyListPropertiesMethod());
    inviwoPyModule_->addMethod(new PyListProcessorsMethod());
    inviwoPyModule_->addMethod(new PyCanvasCountMethod());
    inviwoPyModule_->addMethod(new PyResizeCanvasMethod());
    inviwoPyModule_->addMethod(new PySnapshotMethod());
    inviwoPyModule_->addMethod(new PySnapshotCanvasMethod());
    inviwoPyModule_->addMethod(new PyGetBasePathMethod());
    inviwoPyModule_->addMethod(new PyGetWorkspaceSavePathMethod());
    inviwoPyModule_->addMethod(new PyGetVolumePathMethod());
    inviwoPyModule_->addMethod(new PyGetDataPathMethod());
    inviwoPyModule_->addMethod(new PyGetImagePathMethod());
    inviwoPyModule_->addMethod(new PyGetModulePathMethod());
    inviwoPyModule_->addMethod(new PyGetTransferFunctionPath());
    inviwoPyModule_->addMethod(new PyGetMemoryUsage());
    inviwoPyModule_->addMethod(new PyClearResourceManage());
    inviwoPyModule_->addMethod(new PyEnableEvaluation());
    inviwoPyModule_->addMethod(new PyDisableEvaluation());
    inviwoPyModule_->addMethod(new PySaveTransferFunction());
    inviwoPyModule_->addMethod(new PyLoadTransferFunction());
    inviwoPyModule_->addMethod(new PyClearTransferfunction());
    inviwoPyModule_->addMethod(new PyAddTransferFunction());

    registerPyModule(inviwoPyModule_);
    registerPyModule(inviwoInternalPyModule_);
}

void PyInviwo::initOutputRedirector() {
    std::string directorFileName =
        InviwoApplication::getPtr()->getModuleByType<Python3Module>()->getPath() +
        "/scripts/outputredirector.py";

    if (!filesystem::fileExists(directorFileName)) {
        LogError("Could not open outputredirector.py");
        return;
    }

    std::ifstream file(directorFileName.c_str());
    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    

    PythonScript outputCatcher; 
    outputCatcher.setSource(text);

    if (!outputCatcher.run(false)) {
        LogWarn("Python init script failed to run");
    }
}

}  // namespace
