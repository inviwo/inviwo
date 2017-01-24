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
#include <modules/python3/python3module.h>
#include <modules/python3/pyinviwo.h>
#include <modules/python3/pythonexecutionoutputobservable.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/filesystem.h>
#include <modules/python3/pythonscript.h>
#include <modules/python3/pythonlogger.h>

#include <modules/python3/pybindutils.h>
namespace inviwo {

Python3Module::Python3Module(InviwoApplication* app)
    : InviwoModule(app, "Python3")
    , pyInviwo_(util::make_unique<PyInviwo>(this))
    , pythonScriptArg_("p", "pythonScript", "Specify a python script to run at startup", false, "",
        "Path to the file containing the script") 
{    
    pyInviwo_->addObserver(&pythonLogger_);
    app->getCommandLineParser().add(&pythonScriptArg_, [this]() {
        auto filename = pythonScriptArg_.getValue();
        if (!filesystem::fileExists(filename)) {
            LogWarn("Could not run script, file does not exist: " << filename);
            return;
        }
        PythonScriptDisk s(filename);
        s.run();
    }, 100);

    auto logger = std::make_shared<ConsoleLogger>();
    LogCentral::getPtr()->registerLogger(logger);

    PythonScript tmp;
    tmp.setSource("import sys\nfor p in sys.path:\n\tprint(p)");
    tmp.run();


    PythonScript tmp2;
    tmp2.setSource("import inviwopy\nprint(inviwopy.app)\nprint(inviwopy.app.displayName)");
    tmp2.run();

    PythonScriptDisk(getPath() + "/scripts/documentgenerator.py").run();
}

Python3Module::~Python3Module() {
    pyInviwo_->removeObserver(&pythonLogger_);
}

}  // namespace
