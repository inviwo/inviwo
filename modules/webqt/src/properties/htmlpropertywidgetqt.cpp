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

#include <inviwo/webqt/properties/htmlpropertywidgetqt.h>

#include <inviwo/core/properties/propertysemantics.h>         // for operator==, PropertySem...
#include <inviwo/core/util/assertion.h>                       // for IVW_ASSERT
#include <inviwo/core/util/exception.h>                       // for Exception
#include <modules/qtwidgets/properties/texteditorwidgetqt.h>  // for TextEditorDockWidget
#include <modules/qtwidgets/syntaxhighlighter.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/webqt/htmlsyntaxhighlight.h>

#include <QRegularExpression>
#include <QTextCharFormat>

namespace inviwo {

namespace {

class HtmlEditorDockWidget : public TextEditorDockWidget {
public:
    /**
     * @brief Create a text editor for @p property
     * @pre Property has to be of type FileProperty or StringProperty
     */
    HtmlEditorDockWidget(Property* property) : TextEditorDockWidget(property) {
        auto app = util::getInviwoApplication(property);
        callbacks_ = utilqt::setHtmlSyntaxHighlight(getSyntaxHighlighter(),
                                                    *app->getSettingsByType<HtmlSyntaxHighlight>());
    }

private:
    std::vector<std::shared_ptr<std::function<void()>>> callbacks_;
};

}  // namespace

HtmlPropertyWidgetQt::HtmlPropertyWidgetQt(StringProperty* property)
    : StringPropertyWidgetQt(property) {

    if (property->getSemantics() != PropertySemantics("HtmlEditor")) {
        throw Exception(IVW_CONTEXT,
                        "Invalid semantics for HtmlPropertyWidgetQt, expected HtmlEditor, got {}",
                        property->getSemantics().getString());
    }

    addEditor();
}

HtmlPropertyWidgetQt::~HtmlPropertyWidgetQt() = default;

void HtmlPropertyWidgetQt::initEditor() {
    editor_ = std::make_unique<HtmlEditorDockWidget>(property_);
}

HtmlFilePropertyWidgetQt::HtmlFilePropertyWidgetQt(FileProperty* property)
    : FilePropertyWidgetQt(property) {

    if (property->getSemantics() != PropertySemantics("HtmlEditor")) {
        throw Exception(IVW_CONTEXT,
                        "Invalid semantics for HtmlPropertyWidgetQt, expected HtmlEditor, got {}",
                        property->getSemantics().getString());
    }

    addEditor();
}

HtmlFilePropertyWidgetQt::~HtmlFilePropertyWidgetQt() = default;

void HtmlFilePropertyWidgetQt::initEditor() {
    editor_ = std::make_unique<HtmlEditorDockWidget>(property_);
}

}  // namespace inviwo
