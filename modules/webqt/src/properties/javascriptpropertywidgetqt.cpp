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

#include <inviwo/webqt/properties/javascriptpropertywidgetqt.h>
#include <inviwo/core/properties/propertysemantics.h>  // for operator==, PropertySem...
#include <inviwo/core/util/assertion.h>                // for IVW_ASSERT
#include <inviwo/core/util/exception.h>
#include <modules/qtwidgets/properties/texteditorwidgetqt.h>  // for TextEditorDockWidget
#include <modules/qtwidgets/syntaxhighlighter.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/webqt/javascriptsyntaxhighlight.h>

#include <QRegularExpression>
#include <QTextCharFormat>

namespace inviwo {

namespace {

class JavascriptEditorDockWidget : public TextEditorDockWidget {
public:
    /**
     * @brief Create a text editor for @p property
     * @pre Property has to be of type FileProperty or StringProperty
     */
    explicit JavascriptEditorDockWidget(Property* property)
        : TextEditorDockWidget(property)
        , callbacks_{utilqt::setJavascriptSyntaxHighlight(
              getSyntaxHighlighter(), *util::getInviwoApplication(property)
                                           ->getSettingsByType<JavascriptSyntaxHighlight>())} {}

private:
    std::vector<std::shared_ptr<std::function<void()>>> callbacks_;
};

}  // namespace

JavascriptPropertyWidgetQt::JavascriptPropertyWidgetQt(StringProperty* property)
    : StringPropertyWidgetQt(property) {

    if (property->getSemantics() != PropertySemantics("JavascriptEditor")) {
        throw Exception(
            IVW_CONTEXT,
            "Invalid semantics for JavascriptPropertyWidgetQt, expected JavascriptEditor, got {}",
            property->getSemantics().getString());
    }

    addEditor();
}

JavascriptPropertyWidgetQt::~JavascriptPropertyWidgetQt() = default;

void JavascriptPropertyWidgetQt::initEditor() {
    editor_ = std::make_unique<JavascriptEditorDockWidget>(property_);
}

JavascriptFilePropertyWidgetQt::JavascriptFilePropertyWidgetQt(FileProperty* property)
    : FilePropertyWidgetQt(property) {

    if (property->getSemantics() != PropertySemantics("JavascriptEditor")) {
        throw Exception(
            IVW_CONTEXT,
            "Invalid semantics for JavascriptPropertyWidgetQt, expected JavascriptEditor, got {}",
            property->getSemantics().getString());
    }

    addEditor();
}

JavascriptFilePropertyWidgetQt::~JavascriptFilePropertyWidgetQt() = default;

void JavascriptFilePropertyWidgetQt::initEditor() {
    editor_ = std::make_unique<JavascriptEditorDockWidget>(property_);
}

}  // namespace inviwo
