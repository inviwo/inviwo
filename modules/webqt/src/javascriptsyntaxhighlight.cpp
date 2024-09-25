/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <inviwo/webqt/javascriptsyntaxhighlight.h>

#include <inviwo/core/properties/optionproperty.h>   // for OptionPropertyString
#include <inviwo/core/properties/ordinalproperty.h>  // for FloatVec4Property, ordinalColor, Int...
#include <inviwo/core/properties/property.h>         // for Property
#include <inviwo/core/util/settings/settings.h>      // for Settings
#include <modules/qtwidgets/inviwoqtutils.h>         // for toQColor, getDefaultMonoSpaceFontIndex
#include <modules/qtwidgets/syntaxhighlighter.h>     // for SyntaxHighlighter, background, comment

#include <array>        // for array
#include <chrono>       // for literals
#include <string>       // for string
#include <string_view>  // for operator""sv, string_view, basic_str...

#include <QColor>           // for QColor
#include <QTextCharFormat>  // for QTextCharFormat

namespace inviwo {

JavascriptSyntaxHighlight::JavascriptSyntaxHighlight()
    : Settings("Javascript Syntax Highlighting")
    , font("font", "Font", utilqt::getMonoSpaceFonts(), utilqt::getDefaultMonoSpaceFontIndex())
    , fontSize("fontSize", "Size", syntax::fontSize, 1, 72)
    , textColor("text", "Text", util::ordinalColor(syntax::text))
    , backgroundColor("background", "Background", util::ordinalColor(syntax::background))
    , highLightColor("highLight", "HighLight", util::ordinalColor(syntax::highLight))
    , keywordColor("keyword", "Keyword", util::ordinalColor(syntax::keyword))
    , litteralColor("literal", "String Literal", util::ordinalColor(syntax::literal))
    , constantColor("constant", "Constant", util::ordinalColor(syntax::constant))
    , commentColor("comment", "Comment", util::ordinalColor(syntax::comment))
    , typeColor{"type", "type", util::ordinalColor(syntax::type)}
    , functionColor{"function", "Function", util::ordinalColor(syntax::builtinFuncs)}
    , functionCallColor{"functionCall", "Function Call", util::ordinalColor(syntax::builtinFuncs)} {
    addProperties(font, fontSize, textColor, backgroundColor, highLightColor, keywordColor,
                  litteralColor, constantColor, commentColor, typeColor, functionColor,
                  functionCallColor);

    load();
}

namespace utilqt {

namespace {

using namespace std::literals;

static constexpr std::array javascriptKeywords = {
    "function"sv,   "var"sv,    "let"sv,       "const"sv,    "if"sv,          "else"sv,
    "for"sv,        "while"sv,  "return"sv,    "try"sv,      "catch"sv,       "finally"sv,
    "throw"sv,      "new"sv,    "delete"sv,    "typeof"sv,   "instanceof"sv,  "do"sv,
    "switch"sv,     "case"sv,   "break"sv,     "continue"sv, "public"sv,      "private"sv,
    "protected"sv,  "static"sv, "readonly"sv,  "enum"sv,     "interface"sv,   "extends"sv,
    "implements"sv, "export"sv, "import"sv,    "type"sv,     "namespace"sv,   "abstract"sv,
    "as"sv,         "async"sv,  "await"sv,     "class"sv,    "constructor"sv, "get"sv,
    "set"sv,        "null"sv,   "undefined"sv, "true"sv,     "false"sv};

static constexpr std::array typePatterns = {"string"sv,  "number"sv, "boolean"sv,
                                            "any"sv,     "void"sv,   "never"sv,
                                            "unknown"sv, "Object"sv, "Array"sv};

}  // namespace

std::vector<std::shared_ptr<std::function<void()>>> setJavascriptSyntaxHighlight(
    SyntaxHighlighter& sh, JavascriptSyntaxHighlight& settings) {

    QColor bgColor = utilqt::toQColor(settings.backgroundColor);

    QTextCharFormat defaultFormat;
    defaultFormat.setBackground(bgColor);
    defaultFormat.setForeground(utilqt::toQColor(settings.textColor));

    QTextCharFormat keywordformat;
    keywordformat.setBackground(bgColor);
    keywordformat.setForeground(utilqt::toQColor(settings.keywordColor));

    QTextCharFormat constantsformat;
    constantsformat.setBackground(bgColor);
    constantsformat.setForeground(utilqt::toQColor(settings.constantColor));

    QTextCharFormat commentformat;
    commentformat.setBackground(bgColor);
    commentformat.setForeground(utilqt::toQColor(settings.commentColor));

    QTextCharFormat litteralFormat;
    litteralFormat.setBackground(bgColor);
    litteralFormat.setForeground(utilqt::toQColor(settings.litteralColor));

    QTextCharFormat typeFormat;
    typeFormat.setBackground(bgColor);
    typeFormat.setForeground(utilqt::toQColor(settings.typeColor));

    QTextCharFormat functionFormat;
    functionFormat.setBackground(bgColor);
    functionFormat.setForeground(utilqt::toQColor(settings.functionColor));

    QTextCharFormat functionCallFormat;
    functionCallFormat.setBackground(bgColor);
    functionCallFormat.setForeground(utilqt::toQColor(settings.functionCallColor));

    sh.setFont(settings.font);
    sh.setFontSize(settings.fontSize);
    sh.setHighlight(settings.highLightColor);
    sh.setDefaultFormat(defaultFormat);

    sh.addWordBoundaryPattern(keywordformat, javascriptKeywords);

    sh.addWordBoundaryPattern(typeFormat, typePatterns);

    sh.addPattern(constantsformat, "\\b[0-9]*\\.?[0-9]+\\b");

    sh.addPattern(functionFormat, R"(\bfunction\s+\w+\b\()");
    sh.addPattern(functionCallFormat, R"(\b\w+\s*(?=\())");

    sh.addPattern(commentformat, "//[^\n]*");
    sh.addPattern(commentformat, "/\\*.*?\\*/");

    sh.addPattern(litteralFormat, R"(".*?"|'.*?'|`.*?`)");

    sh.update();

    std::vector<std::shared_ptr<std::function<void()>>> callbacks;
    for (auto p : settings) {
        callbacks.emplace_back(p->onChangeScoped([psh = &sh, psettings = &settings]() {
            setJavascriptSyntaxHighlight(*psh, *psettings);
        }));
    }
    return callbacks;
}

}  // namespace utilqt

}  // namespace inviwo
