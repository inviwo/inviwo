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
#include <warn/pop>
#include <modules/python3/python3module.h>

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
#include <modules/python3/processors/pythonscriptprocessor.h>

#include <modules/python3/pythonprocessorfactoryobject.h>

namespace inviwo {

Python3Module::Python3Module(InviwoApplication* app)
    : InviwoModule(app, "Python3")
    , pythonInterpreter_(std::make_unique<PythonInterpreter>())
    , pythonScriptArg_("p", "pythonScript", "Specify a python script to run at startup", false, "",
                       "python script")
    , argHolder_{app, pythonScriptArg_,
                 [this]() {
                     auto filename = pythonScriptArg_.getValue();
                     if (!filesystem::fileExists(filename)) {
                         LogWarn("Could not run script, file does not exist: " << filename);
                         return;
                     }
                     PythonScriptDisk s(filename);
                     s.run();
                 },
                 100}
    , pythonLogger_{}
    , scripts_{getPath() + "/scripts"}
    , pythonFolderObserver_{app, getPath() + "/processors", *this}
    , settingsFolderObserver_{app, app->getPath(PathType::Settings, "/python_processors", true),
                              *this} {

    pythonInterpreter_->addObserver(&pythonLogger_);

    registerProcessor<NumPyVolume>();
    registerProcessor<NumpyMandelbrot>();
    registerProcessor<NumPyMeshCreateTest>();
    registerProcessor<PythonScriptProcessor>();

    // We need to import inviwopy to trigger the initialization code in inviwopy.cpp, this is needed
    // to be able to cast cpp/inviwo objects to python objects.
    try {
        pybind11::module::import("inviwopy");
    } catch (const std::exception& e) {
        throw ModuleInitException(e.what(), IVW_CONTEXT);
    }
}

Python3Module::~Python3Module() {
    pythonInterpreter_->removeObserver(&pythonLogger_);
    app_->getCommandLineParser().remove(&pythonScriptArg_);
}

PythonInterpreter* Python3Module::getPythonInterpreter() { return pythonInterpreter_.get(); }

}  // namespace inviwo
