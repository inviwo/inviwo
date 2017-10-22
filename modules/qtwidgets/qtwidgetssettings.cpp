/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetssettings.h>

namespace inviwo {

QtWidgetsSettings::QtWidgetsSettings()
    : Settings("Syntax Highlighting")
    , glslSyntax_("glslSyntax", "GLSL Syntax Highlighting")
    , glslTextColor_("glslTextColor", "Text", ivec4(0xAA, 0xAA, 0xAA, 255), ivec4(0, 0, 0, 1),
        ivec4(255, 255, 255, 1), ivec4(1, 1, 1, 1), InvalidationLevel::InvalidOutput,
        PropertySemantics::Color)
    , glslBackgroundColor_("glslBackgroundColor", "Background", ivec4(0x4D, 0x4D, 0x4D, 255),
        ivec4(0, 0, 0, 1), ivec4(255, 255, 255, 1), ivec4(1, 1, 1, 1),
        InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , glslQualifierColor_("glslQualifierColor", "Qualifiers", ivec4(0x7D, 0xB4, 0xDF, 255),
        ivec4(0, 0, 0, 1), ivec4(255, 255, 255, 1), ivec4(1, 1, 1, 1),
        InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , glslBuiltinsColor_("glslBultinsColor", "Builtins", ivec4(0x1F, 0xF0, 0x7F, 255),
        ivec4(0, 0, 0, 1), ivec4(255, 255, 255, 1), ivec4(1, 1, 1, 1),
        InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , glslTypeColor_("glslTypeColor", "Types", ivec4(0x56, 0x9C, 0xD6, 255), ivec4(0, 0, 0, 1),
        ivec4(255, 255, 255, 1), ivec4(1, 1, 1, 1), InvalidationLevel::InvalidOutput,
        PropertySemantics::Color)
    , glslGlslBuiltinsColor_("glslGlslBultinsColor", "GLSL Builtins", ivec4(0xFF, 0x80, 0x00, 255),
        ivec4(0, 0, 0, 1), ivec4(255, 255, 255, 1), ivec4(1, 1, 1, 1),
        InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , glslCommentColor_("glslCommentColor", "Comments", ivec4(0x60, 0x8B, 0x4E, 255),
        ivec4(0, 0, 0, 1), ivec4(255, 255, 255, 1), ivec4(1, 1, 1, 1),
        InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , glslPreProcessorColor_("glslPreProcessorColor", "Pre Processor", ivec4(0x9B, 0x9B, 0x9B, 255),
        ivec4(0, 0, 0, 1), ivec4(255, 255, 255, 1), ivec4(1, 1, 1, 1),
        InvalidationLevel::InvalidOutput, PropertySemantics::Color)

    , pythonSyntax_("pythonSyntax_", "Python Syntax Highlighting")
    , pyFontSize_("pyFontSize_", "Font Size", 11, 1, 72)
    , pyBGColor_("pyBGColor", "Background", ivec4(0xb0, 0xb0, 0xbc, 255), ivec4(0, 0, 0, 1),
        ivec4(255, 255, 255, 1), ivec4(1, 1, 1, 1), InvalidationLevel::InvalidOutput,
        PropertySemantics::Color)
    , pyTextColor_("pyTextColor", "Text", ivec4(0x11, 0x11, 0x11, 255), ivec4(0, 0, 0, 1),
        ivec4(255, 255, 255, 1), ivec4(1, 1, 1, 1), InvalidationLevel::InvalidOutput,
        PropertySemantics::Color)
    , pyTypeColor_("pyTypeColor", "Types", ivec4(0x14, 0x3C, 0xA6, 255), ivec4(0, 0, 0, 1),
        ivec4(255, 255, 255, 1), ivec4(1, 1, 1, 1), InvalidationLevel::InvalidOutput,
        PropertySemantics::Color)
    , pyCommentsColor_("pyCommentsColor", "Comments", ivec4(0x00, 0x66, 0x00, 255),
        ivec4(0, 0, 0, 1), ivec4(255, 255, 255, 1), ivec4(1, 1, 1, 1),
        InvalidationLevel::InvalidOutput, PropertySemantics::Color) {
    addProperty(pythonSyntax_);
    addProperty(glslSyntax_);

    glslSyntax_.addProperty(glslBackgroundColor_);
    glslSyntax_.addProperty(glslTextColor_);
    glslSyntax_.addProperty(glslCommentColor_);
    glslSyntax_.addProperty(glslTypeColor_);
    glslSyntax_.addProperty(glslQualifierColor_);
    glslSyntax_.addProperty(glslBuiltinsColor_);
    glslSyntax_.addProperty(glslGlslBuiltinsColor_);
    glslSyntax_.addProperty(glslPreProcessorColor_);

    pythonSyntax_.addProperty(pyFontSize_);
    pythonSyntax_.addProperty(pyBGColor_);
    pythonSyntax_.addProperty(pyTextColor_);
    pythonSyntax_.addProperty(pyCommentsColor_);
    pythonSyntax_.addProperty(pyTypeColor_);
}

} // namespace

