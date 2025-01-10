/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2025 Inviwo Foundation
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

#include <modules/openglqt/openglqtmoduledefine.h>  // for IVW_MODULE_OPENGLQT_API

#include <inviwo/core/properties/optionproperty.h>   // for OptionPropertyString
#include <inviwo/core/properties/ordinalproperty.h>  // for FloatVec4Property, IntProperty
#include <inviwo/core/util/settings/settings.h>      // for Settings

#include <functional>  // for function
#include <memory>      // for shared_ptr
#include <vector>      // for vector

namespace inviwo {
class SyntaxHighlighter;

class IVW_MODULE_OPENGLQT_API GLSLSyntaxHighlight : public Settings {
public:
    GLSLSyntaxHighlight();

    OptionPropertyString font;
    IntProperty fontSize;

    FloatVec4Property textColor;
    FloatVec4Property backgroundColor;
    FloatVec4Property highLightColor;
    FloatVec4Property keywordColor;
    FloatVec4Property builtinVarColor;
    FloatVec4Property typeColor;
    FloatVec4Property builtinFuncColor;
    FloatVec4Property commentColor;
    FloatVec4Property preProcessorColor;
    FloatVec4Property litteralColor;
    FloatVec4Property constantColor;
    FloatVec4Property mainColor;
};

namespace utilqt {

IVW_MODULE_OPENGLQT_API std::vector<std::shared_ptr<std::function<void()>>> setGLSLSyntaxHighlight(
    SyntaxHighlighter& sh, GLSLSyntaxHighlight& settings);
}

}  // namespace inviwo
