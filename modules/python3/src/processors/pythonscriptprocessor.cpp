/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2022 Inviwo Foundation
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

#include <modules/python3/processors/pythonscriptprocessor.h>

#include <inviwo/core/common/inviwoapplication.h>      // for InviwoApplication
#include <inviwo/core/common/modulepath.h>             // for ModulePath, ModulePath::Data
#include <inviwo/core/processors/processor.h>          // for Processor
#include <inviwo/core/processors/processorinfo.h>      // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>     // for CodeState, CodeState::Deprecated
#include <inviwo/core/processors/processortags.h>      // for Tags, Tags::PY
#include <inviwo/core/properties/fileproperty.h>       // for FileProperty
#include <inviwo/core/properties/invalidationlevel.h>  // for InvalidationLevel, InvalidationLev...
#include <inviwo/core/properties/propertysemantics.h>  // for PropertySemantics, PropertySemanti...
#include <inviwo/core/util/logcentral.h>               // for LogCentral, LogError
#include <inviwo/core/util/statecoordinator.h>         // for StateCoordinator
#include <modules/python3/python3module.h>             // for Python3Module
#include <modules/python3/pythonscript.h>              // for PythonScriptDisk

#include <array>                                       // for array
#include <exception>                                   // for exception
#include <functional>                                  // for __base
#include <string>                                      // for operator+, string, basic_string
#include <string_view>                                 // for string_view

#include <pybind11/cast.h>                             // for cast, object_api::operator()
#include <pybind11/detail/common.h>                    // for pybind11
#include <pybind11/pybind11.h>                         // for globals

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo PythonScriptProcessor::processorInfo_{
    "org.inviwo.PythonScriptProcessor",  // Class identifier
    "Python Script Processor",           // Display name
    "Python",                            // Category
    CodeState::Deprecated,               // Code state
    Tags::PY,                            // Tags
};
const ProcessorInfo PythonScriptProcessor::getProcessorInfo() const { return processorInfo_; }

PythonScriptProcessor::PythonScriptProcessor(InviwoApplication* app)
    : Processor()
    , scriptFileName_("scriptFileName", "File Name",
                      app->getModuleByType<Python3Module>()->getPath(ModulePath::Data) +
                          "/scripts/scriptprocessorexample.py",
                      "python", InvalidationLevel::InvalidOutput, PropertySemantics::PythonEditor)
    , script_(scriptFileName_.get()) {

    namespace py = pybind11;

    isSink_.setUpdate([]() { return true; });

    auto runscript = [this]() {
        auto locals = py::globals();
        locals["self"] = pybind11::cast(this);
        try {
            script_.run(locals);
        } catch (std::exception& e) {
            LogError(e.what())
        }
        invalidate(InvalidationLevel::InvalidOutput);
    };
    addProperty(scriptFileName_);

    scriptFileName_.onChange([this, runscript]() {
        script_.setFilename(scriptFileName_.get());
        runscript();
    });

    script_.onChange([runscript]() { runscript(); });

    runscript();
}

void PythonScriptProcessor::initializeResources() {
    if (initializeResources_) initializeResources_(pybind11::cast(this));
}

void PythonScriptProcessor::process() {
    if (process_) process_(pybind11::cast(this));
}

void PythonScriptProcessor::setInitializeResources(pybind11::function func) {
    initializeResources_ = func;
}
void PythonScriptProcessor::setProcess(pybind11::function func) { process_ = func; }

}  // namespace inviwo
