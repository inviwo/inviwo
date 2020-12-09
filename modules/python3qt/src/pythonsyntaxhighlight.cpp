/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <modules/python3qt/pythonsyntaxhighlight.h>

#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QFontDatabase>
#include <warn/pop>

#include <array>
#include <string_view>

using namespace std::literals;

namespace inviwo {

namespace {}  // namespace

PythonSyntaxHighlight::PythonSyntaxHighlight()
    : Settings("Python Syntax Highlighting")
    , font("font", "Font", utilqt::getMonoSpaceFonts(), utilqt::getDefaultFontIndex())
    , fontSize("fontSize", "Size", syntax::fontSize, 1, 72)
    , textColor("text", "Text", util::ordinalColor(syntax::text))
    , backgroundColor("background", "Background", util::ordinalColor(syntax::background))
    , highLightColor("highLight", "HighLight", util::ordinalColor(syntax::highLight))
    , keywordColor("type", "Type", util::ordinalColor(syntax::keyword))
    , litteralColor("literal", "String Literal", util::ordinalColor(syntax::literal))
    , constantColor("constant", "Constant", util::ordinalColor(syntax::constant))
    , commentColor("comment", "Comment", util::ordinalColor(syntax::comment)) {
    addProperties(font, fontSize, textColor, backgroundColor, highLightColor, keywordColor,
                  litteralColor, constantColor, commentColor);

    load();
}

namespace {

constexpr const std::array pythonKeywords = {
    "and"sv,   "as"sv,     "assert"sv, "break"sv, "class"sv,   "continue"sv, "def"sv,  "del"sv,
    "elif"sv,  "else"sv,   "except"sv, "exec"sv,  "finally"sv, "for"sv,      "from"sv, "global"sv,
    "if"sv,    "import"sv, "in"sv,     "is"sv,    "lambda"sv,  "not"sv,      "or"sv,   "pass"sv,
    "print"sv, "raise"sv,  "return"sv, "try"sv,   "while"sv,   "with"sv,     "yield"sv};

}  // namespace

std::vector<std::shared_ptr<std::function<void()>>> utilqt::setPythonSyntaxHighlight(
    SyntaxHighligther& sh, PythonSyntaxHighlight& settings) {

    QColor bgColor = utilqt::toQColor(settings.backgroundColor);

    QTextCharFormat defaultFormat;
    QColor textColor = utilqt::toQColor(settings.textColor);
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

    sh.clear();

    sh.setFont(settings.font);
    sh.setFontSize(settings.fontSize);
    sh.setHighlight(settings.highLightColor);
    sh.setDefaultFormat(defaultFormat);

    sh.addWordBoundaryPattern(keywordformat, pythonKeywords);
    sh.addPattern(constantsformat, "\\b([0-9]+\\.)?[0-9]+([eE][+-]?[0-9]+)?");

    sh.addPattern(litteralFormat, R"("([^"\\]|\\.)*")");
    sh.addPattern(litteralFormat, R"('([^'\\]|\\.)*')");

    sh.addPattern(commentformat, "#.*$");
    sh.addMultBlockPattern(commentformat, R"(""")"sv, R"(""")"sv);
    sh.addMultBlockPattern(commentformat, R"(''')"sv, R"(''')"sv);

    sh.update();

    std::vector<std::shared_ptr<std::function<void()>>> callbacks;
    for (auto p : settings) {
        callbacks.emplace_back(p->onChangeScoped(
            [psh = &sh, psettings = &settings]() { setPythonSyntaxHighlight(*psh, *psettings); }));
    }
    return callbacks;
}

std::vector<std::shared_ptr<std::function<void()>>> utilqt::setPythonOutputSyntaxHighlight(
    SyntaxHighligther& sh, PythonSyntaxHighlight& settings) {

    QColor bgColor = utilqt::toQColor(settings.backgroundColor);

    QTextCharFormat defaultFormat;
    defaultFormat.setBackground(bgColor);
    defaultFormat.setForeground(utilqt::toQColor(settings.textColor));

    QTextCharFormat constantsformat;
    constantsformat.setBackground(bgColor);
    constantsformat.setForeground(utilqt::toQColor(settings.constantColor));

    sh.clear();

    sh.setFont(settings.font);
    sh.setFontSize(settings.fontSize);
    sh.setHighlight(settings.highLightColor);
    sh.setDefaultFormat(defaultFormat);

    sh.addPattern(constantsformat, "\\b([0-9]+\\.)?[0-9]+([eE][+-]?[0-9]+)?");

    sh.update();

    std::vector<std::shared_ptr<std::function<void()>>> callbacks;
    for (auto p : settings) {
        callbacks.emplace_back(p->onChangeScoped([psh = &sh, psettings = &settings]() {
            setPythonOutputSyntaxHighlight(*psh, *psettings);
        }));
    }
    return callbacks;
}

}  // namespace inviwo
