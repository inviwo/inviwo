/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>  // for IVW_MODULE_QTWIDGETS_API

#include <modules/qtwidgets/properties/propertywidgetqt.h>  // for PropertyWidgetQt

#include <QObject>         // for Q_OBJECT, signals
#include <QPlainTextEdit>  // for QPlainTextEdit

class QContextMenuEvent;
class QFocusEvent;
class QKeyEvent;
class QResizeEvent;
class QWidget;
namespace inviwo {
class EditableLabelQt;
class StringProperty;
}  // namespace inviwo

namespace inviwo {

class MultilineTextEdit;
class MultiFileProperty;

/*! \class StringMultilinePropertyWidgetQt
    \brief Property widget for string properties which shows the contents spread over
    multiple lines. The height of the text editor is adjusted based on the contents and
    given defaults (2 to 10 lines). Changes are committed when the focus changes or
    CTRL + Return or CTRL + Enter is pressed.
*/
class IVW_MODULE_QTWIDGETS_API StringMultilinePropertyWidgetQt : public PropertyWidgetQt {
public:
    StringMultilinePropertyWidgetQt(StringProperty* property);

    void updateFromProperty();
    void setPropertyValue();

private:
    StringProperty* property_;
    MultilineTextEdit* textEdit_;
    EditableLabelQt* label_;
};

class IVW_MODULE_QTWIDGETS_API MultiFileStringPropertyWidgetQt : public PropertyWidgetQt {
public:
    MultiFileStringPropertyWidgetQt(MultiFileProperty* property);

    void updateFromProperty();
    void setPropertyValue();

private:
    MultiFileProperty* property_;
    MultilineTextEdit* textEdit_;
    EditableLabelQt* label_;
};

/*! \class MultilineTextEdit
\brief Basic text editor based on QPlainTextEdit for showing strings in multiple lines.
The height is automatically adjusted. The editingFinished signal is emitted when the
widget looses focus or CTRL + Return or CTRL + Enter is pressed.
*/
class IVW_MODULE_QTWIDGETS_API MultilineTextEdit : public QPlainTextEdit {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    MultilineTextEdit(QWidget* parent = nullptr);
    virtual ~MultilineTextEdit();

    void adjustHeight();
signals:
    void editingFinished();

protected:
    virtual void focusOutEvent(QFocusEvent* e) override;
    virtual void resizeEvent(QResizeEvent* e) override;
    virtual void keyPressEvent(QKeyEvent* e) override;
    virtual void contextMenuEvent(QContextMenuEvent* e) override;

private:
    const int minLineCount_;
    const int maxLineCount_;
    int lineHeight_;
    bool showContextMenu_;
};

}  // namespace inviwo
