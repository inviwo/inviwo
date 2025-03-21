/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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

#pragma once

#include <modules/python3/python3moduledefine.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/util/commandlineparser.h>
#include <modules/python3/pythonlogger.h>
#include <modules/python3/pythonprocessorfolderobserver.h>
#include <modules/python3/pyutils.h>

#include <modules/python3/pythonworkspacescripts.h>
#include <string>

namespace inviwo {
class PythonInterpreter;

class IVW_MODULE_PYTHON3_API Python3Module : public InviwoModule {
public:
    Python3Module(InviwoApplication* app);
    virtual ~Python3Module();

    PythonInterpreter* getPythonInterpreter();

    PythonWorkspaceScripts& getWorkspaceScripts();

private:
    std::unique_ptr<PythonInterpreter> pythonInterpreter_;
    TCLAP::ValueArg<std::string> scriptArg_;
    CommandLineArgHolder scriptArgHolder_;
    TCLAP::ValueArg<std::string> workspaceScriptArg_;
    CommandLineArgHolder workspaceScriptArgHolder_;
    PythonLogger pythonLogger_;

    pyutil::ModulePath scripts_;
    PythonProcessorFolderObserver pythonFolderObserver_;
    PythonProcessorFolderObserver settingsFolderObserver_;

    PythonWorkspaceScripts workspaceScripts_;
};

}  // namespace inviwo
