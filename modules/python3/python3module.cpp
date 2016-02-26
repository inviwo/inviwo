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
#include <modules/python3/python3module.h>
#include <modules/python3/pyinviwo.h>
#include <modules/python3/pythonexecutionoutputobservable.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/commandlineparser.h>
#include <modules/python3/pythonscript.h>

namespace inviwo {

Python3Module::Python3Module(InviwoApplication* app)
    : InviwoModule(app, "Python3")
    , pyInviwo_(nullptr)
    , pythonScriptArg_("p", "pythonScript", "Specify a python script to run at startup", false, "",
        "Path to the file containing the script") {

    PythonExecutionOutputObservable::init();
    pyInviwo_ = util::make_unique<PyInviwo>(this);

    app->getCommandLineParser().add(&pythonScriptArg_, [this]() {
        PythonScript s;
        auto filename = pythonScriptArg_.getValue();
        std::ifstream file(filename.c_str());
        std::string src((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        s.setSource(src);
        s.setFilename(filename);

        s.run();


    }, 100);
}

Python3Module::~Python3Module() {
    pyInviwo_.reset();  // issue destruction before PythonExecutionOutputObservable
    PythonExecutionOutputObservable::deleteInstance();
}

}  // namespace
