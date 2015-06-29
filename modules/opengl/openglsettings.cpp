/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>
#include <modules/opengl/openglmodule.h>
#include <modules/opengl/openglsettings.h>
#include <modules/opengl/glwrap/shadermanager.h>
#include <inviwo/core/util/formatconversion.h>
#include <inviwo/core/util/vectoroperations.h>

namespace inviwo {

OpenGLSettings::OpenGLSettings(OpenGLCapabilities* openglInfo)
    : Settings("OpenGL Settings")
    , shaderReloadingProperty_("shaderReloading", "Automatically reload shaders", true)
    , btnOpenGLInfo_("printOpenGLInfo", "Print OpenGL Info")
    , selectedOpenGLProfile_("selectedOpenGLProfile", "OpenGL Profile")
    , uniformWarnings_("uniformWarnings", "Uniform Warnings")
    , openGlCap_(openglInfo)
    , hasOutputedGLSLVersionOnce_(false) {
    
    selectedOpenGLProfile_.addOption("core", "Core");
    selectedOpenGLProfile_.addOption("compatibility", "Compatibility");
    selectedOpenGLProfile_.setSelectedIdentifier(OpenGLCapabilities::getPreferredProfile());
    selectedOpenGLProfile_.setCurrentStateAsDefault();

    uniformWarnings_.addOption("ignore", "Ignore missing locations", Shader::UniformWarning::Ignore);
    uniformWarnings_.addOption("warn", "Print warning", Shader::UniformWarning::Warn);
    uniformWarnings_.addOption("throw", "Throw error", Shader::UniformWarning::Throw);
    uniformWarnings_.setSelectedIndex(0);
    uniformWarnings_.setCurrentStateAsDefault();

    contextMode_ = OpenGLCapabilities::getPreferredProfile();

    addProperty(shaderReloadingProperty_);
    addProperty(btnOpenGLInfo_);
    addProperty(selectedOpenGLProfile_);
    addProperty(uniformWarnings_);
    
    selectedOpenGLProfile_.onChange(this, &OpenGLSettings::updateProfile);

    if (openGlCap_) {
        btnOpenGLInfo_.onChange(openGlCap_, &OpenGLCapabilities::printDetailedInfo);
    }

    loadFromDisk();
}

OpenGLSettings::~OpenGLSettings() {}

void OpenGLSettings::initialize() {
    if (openGlCap_) {
        if (openGlCap_->getCurrentShaderVersion().getVersion() < 150) {
            selectedOpenGLProfile_.setVisible(false);
        }
    }
}

void OpenGLSettings::deinitialize() {}

void OpenGLSettings::updateProfile() {
    if (openGlCap_) {
        if (openGlCap_->setPreferredProfile(selectedOpenGLProfile_.getSelectedValue(),
                                            !hasOutputedGLSLVersionOnce_) &&
            hasOutputedGLSLVersionOnce_) {
            ShaderManager::getPtr()->rebuildAllShaders();
            if (contextMode_ != selectedOpenGLProfile_.getSelectedValue())
                LogInfoCustom("OpenGLInfo", "Restart application to enable "
                                                << selectedOpenGLProfile_.getSelectedValue()
                                                << " mode.");
        }
        hasOutputedGLSLVersionOnce_ = true;
    }
}

} // namespace
