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
#include <modules/python3/python3module.h>

#if defined(__unix__)
#include <unistd.h>
#elif defined(__APPLE__)
#include <libproc.h> // proc_pidpath
#endif

#include <modules/python3/pythoninterpreter.h>
#include <modules/python3/pythonexecutionoutputobservable.h>

#include <modules/python3/processors/numpymandelbrot.h>
#include <modules/python3/processors/numpyvolume.h>
#include <modules/python3/processors/numpymeshcreatetest.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/filesystem.h>
#include <modules/python3/pythonscript.h>
#include <modules/python3/pythonlogger.h>


namespace inviwo {

Python3Module::Python3Module(InviwoApplication* app)
    : InviwoModule(app, "Python3")
    , pythonInterpreter_(util::make_unique<PythonInterpreter>(this))
    , pythonScriptArg_("p", "pythonScript", "Specify a python script to run at startup", false, "",
                       "Path to the file containing the script") {

    registerProcessor<NumPyVolume>();
    registerProcessor<NumpyMandelbrot>();
    registerProcessor<NumPyMeshCreateTest>();



    pythonInterpreter_->addObserver(&pythonLogger_);
    app->getCommandLineParser().add(
        &pythonScriptArg_,
        [this]() {
            auto filename = pythonScriptArg_.getValue();
            if (!filesystem::fileExists(filename)) {
                LogWarn("Could not run script, file does not exist: " << filename);
                return;
            }
            PythonScriptDisk s(filename);
            s.run();
        },
        100);

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
    pythonInterpreter_->addModulePath(folder);
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
    pythonInterpreter_->addModulePath(folder);
    pythonInterpreter_->addModulePath(folder + "/../../../");
    
#endif

    app->dispatchFront([&]() {
        PythonScript ps;
        ps.setSource("import inviwopy\n");
        ps.run();  // we need to import inviwopy to trigger the initialization code in inviwopy.cpp,
        //           // this is needed to be able to cast cpp/inviwo objects to python objects

        PythonScriptDisk(getPath() + "/scripts/documentgenerator.py").run();
    });

    //registerPythonInitCallback([&](pybind11::module *m) {
    //    app->dispatchFront([&]() {
    //        
    //        
    //        PythonScript ps;
    //        ps.setSource("import inviwopy\n");
    //        ps.run();  

    //    });
    //
    //});
}

Python3Module::~Python3Module() { pythonInterpreter_->removeObserver(&pythonLogger_); }

void Python3Module::registerPythonInitCallback(PythonInitCallback callback) {
    callbackObjects_.push_back(callback);
}

void Python3Module::invokePythonInitCallbacks(pybind11::module* objects) {
    for (auto& c : callbackObjects_) {
        c(objects);
    }
}

}  // namespace
