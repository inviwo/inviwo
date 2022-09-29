/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2022 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>              // for IVW_MODULE_QTWIDGETS_API

#include <modules/qtwidgets/editorfileobserver.h>                 // for EditorFileObserver
#include <modules/qtwidgets/properties/propertyeditorwidgetqt.h>  // for PropertyEditorWidgetQt

#include <functional>                                             // for function
#include <memory>                                                 // for shared_ptr
#include <string>                                                 // for string

class QCloseEvent;

namespace inviwo {

class CodeEdit;
class FileProperty;
class Property;
class StringProperty;
class SyntaxHighlighter;

/**
 * @brief Text Editor for a FileProperty or a StringProperty
 */
class IVW_MODULE_QTWIDGETS_API TextEditorDockWidget : public PropertyEditorWidgetQt {
public:
    /**
     * @brief Create a text editor for @p property
     * @pre Property has to be of type FileProperty or StringProperty
     */
    TextEditorDockWidget(Property* property);
    SyntaxHighlighter& getSyntaxHighlighter();
    virtual ~TextEditorDockWidget();
    void updateFromProperty();

protected:
    virtual void closeEvent(QCloseEvent*) override;
    virtual void onSetDisplayName(Property*, const std::string& displayName) override;

    virtual void setReadOnly(bool readonly) override;

    void updateWindowTitle();
    void propertyModified();

    void save();
    void saveToFile(const std::string& filename);

    FileProperty* fileProperty_;
    StringProperty* stringProperty_;
    CodeEdit* editor_;
    std::shared_ptr<std::function<void()>> propertyCallback_;
    utilqt::EditorFileObserver fileObserver_;
};

}  // namespace inviwo
