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
#include <inviwo/core/util/filesystem.h>
#include <modules/python3/pyinviwo.h>

#include <modules/python3/python3module.h>

#include <modules/python3/pythonscript.h>
#include <modules/python3/pythonexecutionoutputobservable.h>

#include <modules/python3/defaultinterface/pyproperties.h>
#include <modules/python3/defaultinterface/pycamera.h>
#include <modules/python3/defaultinterface/pycanvas.h>
#include <modules/python3/defaultinterface/pylist.h>
#include <modules/python3/defaultinterface/pyutil.h>
#include <modules/python3/defaultinterface/pyvolume.h>

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
                PythonExecutionOutputObservable::getPtr()->pythonExecutionOutputEvent(
                    msg, (isStderr == 0) ? sysstdout : sysstderr);
        }
    }

    Py_RETURN_NONE;
}

PyInviwo::PyInviwo(Python3Module* module)
    : isInit_(false)
{
    init(this);

    initPythonCInterface(module);
}

PyInviwo::~PyInviwo() {
    Py_Finalize();
}

void PyInviwo::registerPyModule(PyModuleDef *def, std::string name) {
    PyObject* obj = PyModule_Create(def);
    if (!obj) {
        LogError("Failed to init " << name);
    }
    PyDict_SetItemString(dict_, name.c_str(), obj);
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

std::vector<PyModule*> PyInviwo::getAllPythonModules() { return registeredModules_; }

#include <warn/push>
#include <warn/ignore/missing-field-initializers>

static PyMethodDef Inviwo_Internals_METHODS[] =
{
    { "ivwPrint" , py_stdout , METH_VARARGS, "A simple example of an embedded function." },
    nullptr
};

static PyMethodDef Inviwo_METHODS[] =
{
    //Defined in pyproperties.h
    { "setPropertyValue" , py_setPropertyValue , METH_VARARGS, "Assigns a value to a processor property. The value has to be passed as scalar or "
    "tuple, depending on the property's cardinality. Camera properties take a 3-tuple "
    "of 3-tuples, containing the position, focus and up vectors. Option properties "
    "expect an option key." },
    { "setPropertyMaxValue" , py_setPropertyMaxValue , METH_VARARGS, "Sets the max value for a property." },
    { "setPropertyMinValue" , py_setPropertyMinValue , METH_VARARGS, "Sets the min value for a property." },
    { "getPropertyValue" , py_getPropertyValue , METH_VARARGS, "Returns the current value of a processor property (scalar or tuple)." },
    { "getPropertyMaxValue" , py_getPropertyMaxValue , METH_VARARGS, "Returns the max value for a property (scalar or tuple)." },
    { "getPropertyMinValue" , py_getPropertyMinValue , METH_VARARGS, "Returns the min value for a property (scalar or tuple)." },
    { "clickButton" , py_clickButton , METH_VARARGS, "Simulates a click on a button property." }, 

    //Defined in pycamera.h
    { "setCameraFocus" , py_setCameraFocus , METH_VARARGS, "Function to set the cameras focal point." },
    { "setCameraUp" , py_setCameraUp , METH_VARARGS, "Function to set the cameras up direction." },
    { "setCameraPos" , py_setCameraPos , METH_VARARGS, "Function to set the cameras position." },


    //Defined in pycanvas.h
    { "canvascount" , py_canvascount , METH_VARARGS, "Returns the number of canvases in the current network." },
    { "resizecanvas" , py_resizecanvas , METH_VARARGS, "Resizes the canvas in the network to the given size. Canvas can either be given using a canvas index (starting at 0) or a canvas ID string." },

    //Defined in pylist.h
    { "listProperties" , py_listProperties , METH_VARARGS, "List all properties for a processor." },
    { "listProcesoors" , py_listProcesoors , METH_VARARGS, "Lists all processors in the current network." },

    //Defined in pyutil.h
    { "wait" , py_wait , METH_VARARGS, "Make the script wait for all processors in the network to finish their work." },
    { "snapshot" , py_snapshot , METH_VARARGS, "Saves a snapshot of the specified canvas to the given file. If no canvas name is "
                                               "passed, the first canvas in the network is chosen." },
    { "snapshotCanvas" , py_snapshotCanvas , METH_VARARGS, "Saves a snapshot of the ith canvas to the given file." },
    { "snapshotAllCanvases" , py_snapshotAllCanvases , METH_VARARGS, "Saves a snapshot of each canvas to the given path, with a given prefix (prefix defaults to the empty string)." },
    { "getBasePath" , py_getBasePath , METH_VARARGS, "Returns the path to Inviwos base folder." },
    { "getDataPath" , py_getDataPath , METH_VARARGS, "Returns the path to Inviwos data folder." },
    { "getWorkspaceSavePath" , py_getWorkspaceSavePath , METH_VARARGS, "Returns the path to Inviwos workspace folder." },
    { "getVolumePath" , py_getVolumePath , METH_VARARGS, "Returns the path to Inviwos volume folder." },
    { "getImagePath" , py_getImagePath , METH_VARARGS, "Returns the path to Inviwos image folder." },
    { "getModulePath" , py_getModulePath , METH_VARARGS, "Returns the path to the given module." },
    { "getTransferFunctionPath" , py_getTransferFunctionPath , METH_VARARGS, "Returns the path to Inviwo transfer function folder." },
    { "getMemoryUsage" , py_getMemoryUsage , METH_VARARGS, "Return how big Inviwo's current RAM working set is." },
    { "clearResourceManager" , py_clearResourceManager , METH_VARARGS, "Method to clear Inviwo's resource manager." },
    { "disableEvaluation" , py_disableEvaluation , METH_VARARGS, "Method to disable evaluation of Inviwo's network." },
    { "enableEvaluation" , py_enableEvaluation , METH_VARARGS, "Method to re-enable evaluation of Inviwo's network." },

    //Defined in pyvolume.h

    { "saveTransferFunction" , py_saveTransferFunction , METH_VARARGS, "Save a transfer function to file from the specified transfer function property." },
    { "loadTransferFunction" , py_loadTransferFunction , METH_VARARGS, "Load a transfer function from file into the specified transfer function property." },
    { "clearTransferfunction" , py_clearTransferfunction , METH_VARARGS, "Clears a transfer function." },
    { "addPointToTransferFunction" , py_addPointTransferFunction , METH_VARARGS, "Load a transfer function from file into the specified transfer function property." },

    nullptr
};

#include <warn/pop>

struct PyModuleDef Inviwo_Internals_Module_Def = {
    PyModuleDef_HEAD_INIT,
    "inviwo_internal",
    nullptr,
    -1,
    Inviwo_Internals_METHODS,
    nullptr, nullptr, nullptr, nullptr
};


struct PyModuleDef Inviwo_Module_Def = {
    PyModuleDef_HEAD_INIT,
    "inviwo",
    nullptr,
    -1,
    Inviwo_METHODS,
    nullptr, nullptr, nullptr, nullptr
};

void PyInviwo::initPythonCInterface(Python3Module* module) {
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
    registerPyModule(&Inviwo_Internals_Module_Def, "inviwo_internal");
    registerPyModule(&Inviwo_Module_Def, "inviwo");

    addModulePath(module->getPath() + "/scripts");
    initOutputRedirector(module);
}

void PyInviwo::importModule(const std::string& moduleName) {
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

void PyInviwo::initOutputRedirector(Python3Module* module) {
    std::string directorFileName = module->getPath() + "/scripts/outputredirector.py";

    if (!filesystem::fileExists(directorFileName)) {
        LogError("Could not open outputredirector.py");
        return;
    }

    std::ifstream file(directorFileName.c_str());
    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    PythonScript outputCatcher;
    outputCatcher.setSource(text);
    outputCatcher.setFilename(directorFileName);

    if (!outputCatcher.run(false)) {
        LogWarn("Python init script failed to run");
    }
}

}  // namespace
