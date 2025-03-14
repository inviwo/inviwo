/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

#include <inviwo/core/properties/boolproperty.h>    // for BoolProperty
#include <inviwo/core/properties/buttonproperty.h>  // for ButtonProperty
#include <inviwo/core/properties/optionproperty.h>  // for OptionPropertyOption, OptionProperty
#include <inviwo/core/util/settings/settings.h>     // for Settings
#include <inviwo/core/util/staticstring.h>          // for operator+
#include <modules/opengl/debugmessages.h>           // for BreakLevel, Severity, Mode, operator<<
#include <modules/opengl/openglsettings.h>          // for OpenGLSettings
#include <modules/opengl/shader/shader.h>           // for Shader::UniformWarning, Shader::OnError

#include <functional>   // for __base
#include <string>       // for operator==, string
#include <string_view>  // for operator==, string_view
#include <vector>       // for operator!=, vector, operator==

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
                      0) {

    addProperty(shaderReloadingProperty_);
    addProperty(btnOpenGLInfo_);
    addProperty(uniformWarnings_);
    addProperty(shaderObjectErrors_);
    addProperty(debugMessages_);
    addProperty(debugSeverity_);
    addProperty(breakOnMessage_);

    breakOnMessage_.setVisible(false);

    debugSeverity_.onChange(
        [&]() { utilgl::handleOpenGLDebugMessagesChange(debugSeverity_.getSelectedValue()); });

    debugMessages_.onChange([this]() {
        if (debugMessages_.getSelectedValue() == utilgl::debug::Mode::DebugSynchronous) {
            breakOnMessage_.setVisible(true);
        } else {
            breakOnMessage_.setVisible(false);
        }
        utilgl::handleOpenGLDebugModeChange(debugMessages_.getSelectedValue(),
                                            debugSeverity_.getSelectedValue());
    });

    load();
}

}  // namespace inviwo
