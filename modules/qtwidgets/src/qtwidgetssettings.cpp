/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QFontDatabase>
#include <warn/pop>

namespace inviwo {

namespace {
std::vector<std::string> getMonoSpaceFonts() {
    std::vector<std::string> fonts;
    QFontDatabase fontdb;

    for (auto& font : fontdb.families()) {
        if (fontdb.isFixedPitch(font)) {
            fonts.push_back(utilqt::fromQString(font));
        }
    }

    return fonts;
}

size_t getDefaultFontIndex() {
    const auto fonts = getMonoSpaceFonts();
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    auto it = std::find(fonts.begin(), fonts.end(), utilqt::fromQString(fixedFont.family()));
    if (it != fonts.end()) {
        return it - fonts.begin();
    } else {
        return 0;
    }
}

#include <warn/push>
#include <warn/ignore/unused-variable>
const ivec4 ghost_white(248, 248, 240, 255);
const ivec4 light_ghost_white(248, 248, 242, 255);
const ivec4 light_gray(204, 204, 204, 255);
const ivec4 gray(136, 136, 136, 255);
const ivec4 brown_gray(73, 72, 62, 255);
const ivec4 dark_gray(43, 44, 39, 255);

const ivec4 yellow(230, 219, 116, 255);
const ivec4 blue(102, 217, 239, 255);
const ivec4 pink(249, 38, 114, 255);
const ivec4 purple(174, 129, 255, 255);
const ivec4 brown(117, 113, 94, 255);
const ivec4 orange(253, 151, 31, 255);
const ivec4 light_orange(255, 213, 105, 255);
const ivec4 green(166, 226, 46, 255);
const ivec4 sea_green(166, 228, 48, 255);
#include <warn/pop>

}  // namespace

QtWidgetsSettings::QtWidgetsSettings()
    : Settings("Syntax Highlighting")
    , font_("font", "Font", getMonoSpaceFonts(), getDefaultFontIndex())
    , fontSize_("fontSize", "Size", 11, 1, 72)
    , glslSyntax_("glslSyntax", "GLSL Syntax Highlighting")
    , glslTextColor_("glslTextColor", "Text", light_ghost_white, ivec4(0), ivec4(255), ivec4(1),
                     InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , glslBackgroundColor_("glslBackgroundColor", "Background", dark_gray, ivec4(0), ivec4(255),
                           ivec4(1), InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , glslBackgroundHighLightColor_("glslBackgroundHighLightColor", "High Light",
                                    ivec4{33, 34, 29, 255}, ivec4(0), ivec4(255), ivec4(1),
                                    InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , glslQualifierColor_("glslQualifierColor", "Qualifiers", pink, ivec4(0), ivec4(255), ivec4(1),
                          InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , glslBuiltinsColor_("glslBultinsColor", "Builtins", orange, ivec4(0), ivec4(255), ivec4(1),
                         InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , glslTypeColor_("glslTypeColor", "Types", blue, ivec4(0), ivec4(255), ivec4(1),
                     InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , glslGlslBuiltinsColor_("glslGlslBultinsColor", "GLSL Builtins", blue, ivec4(0), ivec4(255),
                             ivec4(1), InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , glslCommentColor_("glslCommentColor", "Comments", gray, ivec4(0), ivec4(255), ivec4(1),
                        InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , glslPreProcessorColor_("glslPreProcessorColor", "Pre Processor", pink, ivec4(0), ivec4(255),
                             ivec4(1), InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , glslConstantsColor_("glslConstantsColor", "Constants", purple, ivec4(0), ivec4(255), ivec4(1),
                          InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , glslVoidMainColor_("glslVoidMainColor", "void main", sea_green, ivec4(0), ivec4(255),
                         ivec4(1), InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , pythonSyntax_("pythonSyntax", "Python Syntax Highlighting")
    , pyBGColor_("pyBGColor", "Background", ivec4(0xb0, 0xb0, 0xbc, 255), ivec4(0), ivec4(255),
                 ivec4(1), InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , pyBGHighLightColor_("pyBGHighLightColor", "High Light", ivec4(0xd0, 0xd0, 0xdc, 255),
                          ivec4(0), ivec4(255), ivec4(1), InvalidationLevel::InvalidOutput,
                          PropertySemantics::Color)
    , pyTextColor_("pyTextColor", "Text", ivec4(0x11, 0x11, 0x11, 255), ivec4(0), ivec4(255),
                   ivec4(1), InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , pyTypeColor_("pyTypeColor", "Types", ivec4(0x14, 0x3C, 0xA6, 255), ivec4(0), ivec4(255),
                   ivec4(1), InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , pyCommentsColor_("pyCommentsColor", "Comments", ivec4(0x00, 0x66, 0x00, 255), ivec4(0),
                       ivec4(255), ivec4(1), InvalidationLevel::InvalidOutput,
                       PropertySemantics::Color) {

    addProperty(font_);
    addProperty(fontSize_);

    addProperty(glslSyntax_);
    glslSyntax_.addProperty(glslBackgroundColor_);
    glslSyntax_.addProperty(glslBackgroundHighLightColor_);
    glslSyntax_.addProperty(glslTextColor_);
    glslSyntax_.addProperty(glslCommentColor_);
    glslSyntax_.addProperty(glslTypeColor_);
    glslSyntax_.addProperty(glslQualifierColor_);
    glslSyntax_.addProperty(glslBuiltinsColor_);
    glslSyntax_.addProperty(glslGlslBuiltinsColor_);
    glslSyntax_.addProperty(glslPreProcessorColor_);
    glslSyntax_.addProperty(glslConstantsColor_);
    glslSyntax_.addProperty(glslVoidMainColor_);

    addProperty(pythonSyntax_);
    pythonSyntax_.addProperty(pyBGColor_);
    pythonSyntax_.addProperty(pyBGHighLightColor_);
    pythonSyntax_.addProperty(pyTextColor_);
    pythonSyntax_.addProperty(pyCommentsColor_);
    pythonSyntax_.addProperty(pyTypeColor_);

    load();
}

}  // namespace inviwo
