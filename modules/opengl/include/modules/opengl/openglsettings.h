/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2018 Inviwo Foundation
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

#ifndef IVW_OPENGLSETTINGS_H
#define IVW_OPENGLSETTINGS_H

#include <modules/opengl/openglmoduledefine.h>
#include <inviwo/core/util/settings/settings.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <modules/opengl/openglcapabilities.h>
#include <modules/opengl/debugmessages.h>
#include <modules/opengl/shader/shader.h>

namespace inviwo {

class IVW_MODULE_OPENGL_API OpenGLSettings : public Settings {

public:
    OpenGLSettings();

    BoolProperty shaderReloadingProperty_;
    ButtonProperty btnOpenGLInfo_;
    OptionPropertyString selectedOpenGLProfile_;
    TemplateOptionProperty<Shader::UniformWarning> uniformWarnings_;
    TemplateOptionProperty<Shader::OnError> shaderObjectErrors_;

    TemplateOptionProperty<utilgl::debug::Mode> debugMessages_;
    TemplateOptionProperty<utilgl::debug::Severity> debugSeverity_;
    TemplateOptionProperty<utilgl::debug::BreakLevel> breakOnMessage_;
};

}  // namespace inviwo

#endif  // IVW_OPENGLSETTINGS_H
