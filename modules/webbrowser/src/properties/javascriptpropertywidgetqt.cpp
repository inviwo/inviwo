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

#include <modules/webbrowser/properties/javascriptpropertywidgetqt.h>

#include <inviwo/core/properties/propertysemantics.h>             // for operator==, PropertySem...
#include <inviwo/core/properties/stringproperty.h>                // for StringProperty
#include <inviwo/core/util/assertion.h>                           // for IVW_ASSERT
#include <modules/qtwidgets/properties/stringpropertywidgetqt.h>  // for StringPropertyWidgetQt
#include <modules/qtwidgets/properties/texteditorwidgetqt.h>      // for TextEditorDockWidget
#include <modules/qtwidgets/syntaxhighlighter.h>

#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/common/inviwoapplication.h>

#include <QRegularExpression>

#include <QTextCharFormat>

namespace inviwo {

namespace {

void javascriptHighlightingRules(SyntaxHighlighter& sh) {

    QColor bgColor = utilqt::toQColor(syntax::background);

    QTextCharFormat defaultFormat;
    defaultFormat.setBackground(bgColor);
    defaultFormat.setForeground(utilqt::toQColor(syntax::text));

    QTextCharFormat keywordformat;
    keywordformat.setBackground(bgColor);
    keywordformat.setForeground(utilqt::toQColor(syntax::keyword));

    QTextCharFormat constantsformat;
    constantsformat.setBackground(bgColor);
    constantsformat.setForeground(utilqt::toQColor(syntax::constant));

    QTextCharFormat commentformat;
    commentformat.setBackground(bgColor);
    commentformat.setForeground(utilqt::toQColor(syntax::comment));

    QTextCharFormat litteralFormat;
    litteralFormat.setBackground(bgColor);
    litteralFormat.setForeground(utilqt::toQColor(syntax::literal));


    sh.clear();

    sh.setFont(settings.font);
    sh.setFontSize(settings.fontSize);
    sh.setHighlight(settings.highLightColor);
    sh.setDefaultFormat(defaultFormat);

    rule.pattern = QRegularExpression(".*");
    rule.format = Default;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("\\b(\\w+)\\s*(?=\\()");
    rule.format = FunctionCall;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("\\bfunction\\s+(\\w+)\\b");
    rule.format = Function;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("\\b[0-9]*\\.?[0-9]+\\b");
    rule.format = Number;
    highlightingRules.append(rule);

    QStringList keywordPatterns = {
        "\\bfunction\\b",    "\\bvar\\b",      "\\blet\\b",        "\\bconst\\b",
        "\\bif\\b",          "\\belse\\b",     "\\bfor\\b",        "\\bwhile\\b",
        "\\breturn\\b",      "\\btry\\b",      "\\bcatch\\b",      "\\bfinally\\b",
        "\\bthrow\\b",       "\\bnew\\b",      "\\bdelete\\b",     "\\btypeof\\b",
        "\\binstanceof\\b",  "\\bdo\\b",       "\\bswitch\\b",     "\\bcase\\b",
        "\\bbreak\\b",       "\\bcontinue\\b", "\\bpublic\\b",     "\\bprivate\\b",
        "\\bprotected\\b",   "\\bstatic\\b",   "\\breadonly\\b",   "\\benum\\b",
        "\\binterface\\b",   "\\bextends\\b",  "\\bimplements\\b", "\\bexport\\b",
        "\\bimport\\b",      "\\btype\\b",     "\\bnamespace\\b",  "\\babstract\\b",
        "\\bas\\b",          "\\basync\\b",    "\\bawait\\b",      "\\bclass\\b",
        "\\bconstructor\\b", "\\bget\\b",      "\\bset\\b",        "\\bnull\\b",
        "\\bundefined\\b",   "\\btrue\\b",     "\\bfalse\\b"};

    for (const QString& pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = Keyword;
        highlightingRules.append(rule);
    }

    QStringList typePatterns = {"\\bstring\\b",  "\\bnumber\\b", "\\bboolean\\b",
                                "\\bany\\b",     "\\bvoid\\b",   "\\bnever\\b",
                                "\\bunknown\\b", "\\bObject\\b", "\\bArray\\b"};

    for (const QString& pattern : typePatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = Type;
        highlightingRules.append(rule);
    }

    rule.pattern = QRegularExpression("\".*?\"|'.*?'|`.*?`");
    rule.format = String;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = Comment;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("/\\*.*?\\*/");
    rule.format = Comment;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("=>");
    rule.format = Arrow;
    highlightingRules.append(rule);
}
}  // namespace

class JavascriptEditorDockWidget : public TextEditorDockWidget {
public:
    /**
     * @brief Create a text editor for @p property
     * @pre Property has to be of type FileProperty or StringProperty
     */
    JavascriptEditorDockWidget(Property* property) : TextEditorDockWidget(property) {
        getSyntaxHighlighter()
    }

private:
    std::vector<std::shared_ptr<std::function<void()>>> callbacks_;
};

JavascriptPropertyWidgetQt::JavascriptPropertyWidgetQt(StringProperty* property)
    : StringPropertyWidgetQt(property) {

    IVW_ASSERT(property->getSemantics() == PropertySemantics("JavascriptEditor"),
               "Wrong semantics");

    addEditor();
}

JavascriptPropertyWidgetQt::~JavascriptPropertyWidgetQt() = default;

void JavascriptPropertyWidgetQt::initEditor() {
    editor_ = std::make_unique<JavascriptEditorDockWidget>(property_);
}

}  // namespace inviwo
