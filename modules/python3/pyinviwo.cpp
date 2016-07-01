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
#include <modules/python3/defaultinterface/pyutil.h>
#include <modules/python3/defaultinterface/pyvolume.h>
#include <modules/python3/defaultinterface/pyprocessor.h>
#include <modules/python3/defaultinterface/pyprocessorwidget.h>
#include <modules/python3/defaultinterface/pynetwork.h>

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
                PyInviwo::getPtr()->pythonExecutionOutputEvent(
                    msg, (isStderr == 0) ? sysstdout : sysstderr);
        }
    }

    Py_RETURN_NONE;
}

PyInviwo::PyInviwo(Python3Module* module) : isInit_(false) {
    init(this);

    initPythonCInterface(module);
}

PyInviwo::~PyInviwo() { Py_Finalize(); }

void PyInviwo::registerPyModule(PyModuleDef* def, std::string name) {
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
 
    {"hasPropertyEditor",           py_hasPropertyEditor,           METH_VARARGS, "Dose the property have a property editor."},
    {"setPropertyEditorVisible",    py_setPropertyEditorVisible,    METH_VARARGS, "Show/Hide the property editor for given property."},
    {"isPropertyEditorVisible",     py_isPropertyEditorVisible,     METH_VARARGS, "Is Property Dditor visible"},
    {"setPropertyEditorPosition",   py_setPropertyEditorPosition,   METH_VARARGS, "Set Property Dditor position"},
    {"getPropertyEditorPosition",   py_getPropertyEditorPosition,   METH_VARARGS, "Get Property Dditor position"},
    {"setPropertyEditorDimensions", py_setPropertyEditorDimensions, METH_VARARGS, "Set Property Dditor dimensions"},
    {"getPropertyEditorDimensions", py_getPropertyEditorDimensions, METH_VARARGS, "Get Property Dditor dimensions"},

    // Defined in pyutil.h
    {"snapshotAllCanvases",  py_snapshotAllCanvases,  METH_VARARGS, "Saves a snapshot of each canvas to the given path, with a given prefix (prefix defaults to the empty string)." },
    {"getOutputPath",        py_getOutputPath,        METH_VARARGS, "Returns the output path given by the -o command line option." },
    {"getWorkspaceSavePath", py_getWorkspaceSavePath, METH_VARARGS, "Returns the path to Inviwos workspace folder." },
    {"getModulePath",        py_getModulePath,        METH_VARARGS, "Returns the path to the given module." },
    {"getMemoryUsage",       py_getMemoryUsage,       METH_VARARGS, "Return how big Inviwo's current RAM working set is." },
      
    // Defined in pyprocessor
    {"setProcessorSelected", py_setProcessorSelected, METH_VARARGS, "Control whether a processor is selected"},
    {"isProcessorSelected",  py_isProcessorSelected,  METH_VARARGS, "Is processor selected"},
    {"setProcessorVisible",  py_setProcessorVisible,  METH_VARARGS, "Control whether a processor is visible"},
    {"isProcessorVisible",   py_isProcessorVisible,   METH_VARARGS, "Is processor visible"},
    {"setProcessorPosition", py_setProcessorPosition, METH_VARARGS, "Set the position of a processor"},
    {"getProcessorPosition", py_getProcessorPosition, METH_VARARGS, "Get the position of a processor"},


    {"getProcessorClassIdentifier", py_getProcessorClassIdentifier, METH_VARARGS, "Get the Processor class identifier"},
    {"getProcessorDisplayName",     py_getProcessorDisplayName,     METH_VARARGS, "Get the Processor display name"},
    {"getProcessorCategory",        py_getProcessorCategory,        METH_VARARGS, "Get the Processor category"},
    {"getProcessorState",           py_getProcessorState,           METH_VARARGS, "Get the Processor state"},
    {"getProcessorTags",            py_getProcessorTags,            METH_VARARGS, "Get the Processor tags"},

    {"getProcessorInports",         py_getProcessorInports,         METH_VARARGS, "Get all inports for processor"},
    {"getProcessorOutports",        py_getProcessorOutports,        METH_VARARGS, "Get all outports for processor"},

    // Defined in pyprocessorwidget
    {"hasProcessorWidget",           py_hasProcessorWidget,           METH_VARARGS, "Does processor have a widget"},
    {"setProcessorWidgetVisible",    py_setProcessorWidgetVisible,    METH_VARARGS, "Set processor widget visibility"},
    {"isProcessorWidgetVisible",     py_isProcessorWidgetVisible,     METH_VARARGS, "Is processor widget visible"},
    {"setProcessorWidgetPosition",   py_setProcessorWidgetPosition,   METH_VARARGS, "Set processor widget position"},
    {"getProcessorWidgetPosition",   py_getProcessorWidgetPosition,   METH_VARARGS, "Get processor widget position"},
    {"setProcessorWidgetDimensions", py_setProcessorWidgetDimensions, METH_VARARGS, "Set processor widget dimensions"},
    {"getProcessorWidgetDimensions", py_getProcessorWidgetDimensions, METH_VARARGS, "Get processor widget dimensions"},

    // Defined in pynetwork
    {"getFactoryProcessors", py_getFactoryProcessors, METH_VARARGS, "Get a list of all factory processors"},
    {"getProcessors",    py_getProcessors,    METH_VARARGS, "Get a list of all processors"},
    {"addProcessor",     py_addProcessor,     METH_VARARGS, "Add a processor using classIdentifier and identifier"},
    {"removeProcessor",  py_removeProcessor,  METH_VARARGS, "Remove a processor using a identifier"},

    {"getConnections",   py_getConnections,   METH_VARARGS, "Get a list of all connections"},
    {"addConnection",    py_addConnection,    METH_VARARGS, "Add a connection using two processor id and port id"},
    {"removeConnection", py_removeConnection, METH_VARARGS, "Remove a connection using processor id and port id"},

    {"getLinks",         py_getLinks,         METH_VARARGS, "Get a list of all links"},
    {"addLink",          py_addLink,          METH_VARARGS, "Add a link using two property paths"},
    {"removeLink",       py_removeLink,       METH_VARARGS, "Remove a link using two property paths"},
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

    PythonScriptDisk outputCatcher(directorFileName);
    if (!outputCatcher.run()) {
        LogWarn("Python init script failed to run");
    }
}

}  // namespace
