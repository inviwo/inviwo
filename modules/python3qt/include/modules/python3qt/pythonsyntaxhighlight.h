/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2021 Inviwo Foundation
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

#include <modules/python3qt/python3qtmoduledefine.h>

#include <inviwo/core/util/settings/settings.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>

#include <modules/qtwidgets/syntaxhighlighter.h>

namespace inviwo {

/**
 * @brief Font and syntax highlight colors for python code
 */
class IVW_MODULE_PYTHON3QT_API PythonSyntaxHighlight : public Settings {
public:
    PythonSyntaxHighlight();

    OptionPropertyString font;
    IntProperty fontSize;

    FloatVec4Property textColor;
    FloatVec4Property backgroundColor;
    FloatVec4Property highLightColor;
    FloatVec4Property keywordColor;
    FloatVec4Property litteralColor;
    FloatVec4Property constantColor;
    FloatVec4Property commentColor;
};

namespace utilqt {

IVW_MODULE_PYTHON3QT_API std::vector<std::shared_ptr<std::function<void()>>>
setPythonSyntaxHighlight(SyntaxHighlighter& sh, PythonSyntaxHighlight& settings);

IVW_MODULE_PYTHON3QT_API std::vector<std::shared_ptr<std::function<void()>>>
setPythonOutputSyntaxHighlight(SyntaxHighlighter& sh, PythonSyntaxHighlight& settings);

}  // namespace utilqt

}  // namespace inviwo
