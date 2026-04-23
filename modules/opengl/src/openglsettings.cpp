/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2026 Inviwo Foundation
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

#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/util/settings/settings.h>
#include <inviwo/core/util/staticstring.h>
#include <modules/opengl/debugmessages.h>
#include <modules/opengl/openglsettings.h>
#include <modules/opengl/shader/shader.h>

#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace inviwo {

OpenGLSettings::OpenGLSettings()
    : Settings("OpenGL Settings")
    , shaderReloadingProperty_("shaderReloading", "Automatically reload shaders", true)
    , btnOpenGLInfo_("printOpenGLInfo", "Print OpenGL Info")
    , uniformWarnings_("uniformWarnings", "Uniform Warnings",
                       {{"ignore", "Ignore missing locations", Shader::UniformWarning::Ignore},
                        {"warn", "Print warning", Shader::UniformWarning::Warn},
                        {"throw", "Throw error", Shader::UniformWarning::Throw}},
                       0)
    , shaderObjectErrors_("compileWarnings", "Shader Errors",
                          {{"warn", "Print warning", Shader::OnError::Warn},
                           {"throw", "Throw error", Shader::OnError::Throw}},
                          0)
    , debugMessages_("debugMessages", "Debug",
                     {utilgl::debug::Mode::Off, utilgl::debug::Mode::Debug,
                      utilgl::debug::Mode::DebugSynchronous},
                     0)
    , debugSeverity_("debugSeverity", "Severity",
                     {utilgl::debug::Severity::Notification, utilgl::debug::Severity::Low,
                      utilgl::debug::Severity::Medium, utilgl::debug::Severity::High},
                     2)
    , breakOnMessage_("breakOnMessage", "Break on Message",
                      {utilgl::debug::BreakLevel::Off, utilgl::debug::BreakLevel::High,
                       utilgl::debug::BreakLevel::Medium, utilgl::debug::BreakLevel::Low,
                       utilgl::debug::BreakLevel::Notification},
                      0)
    , errorChecking_("errorChecking", "Error Checking",
                     "This will call glGetError after every OpenGL call and log any errors"_help,
                     false)
    , breakOnError_("breakOnError", "Break on Error", false)
    , stackSize_{"stackSize", "Stacktrace Size",
                 "Append a stracktrace of the first N frames to the log message"_help} {

    addProperties(shaderReloadingProperty_, btnOpenGLInfo_, uniformWarnings_, shaderObjectErrors_,
                  debugMessages_, debugSeverity_, breakOnMessage_, errorChecking_, breakOnError_,
                  stackSize_);

    debugSeverity_.onChange(
        [&]() { utilgl::handleOpenGLDebugMessagesChange(debugSeverity_.getSelectedValue()); });

    debugMessages_.onChange([this]() {
        utilgl::handleOpenGLDebugModeChange(debugMessages_.getSelectedValue(),
                                            debugSeverity_.getSelectedValue());
    });

    errorChecking_.onChange([this]() {
        utilgl::handleOpenGLErrorCheckingChange(errorChecking_.get(), breakOnError_.get(),
                                                stackSize_.get());
    });

    breakOnError_.onChange([this]() {
        utilgl::handleOpenGLErrorCheckingChange(errorChecking_.get(), breakOnError_.get(),
                                                stackSize_.get());
    });
    stackSize_.onChange([this]() {
        utilgl::handleOpenGLErrorCheckingChange(errorChecking_.get(), breakOnError_.get(),
                                                stackSize_.get());
    });

    load();
}

}  // namespace inviwo
