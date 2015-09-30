/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_STRINGMULTILINEPROPERTYWIDGETQT_H
#define IVW_STRINGMULTILINEPROPERTYWIDGETQT_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <inviwo/qt/widgets/editablelabelqt.h>
#include <inviwo/qt/widgets/properties/propertywidgetqt.h>
#include <inviwo/core/properties/stringproperty.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QFontMetrics>
#include <QPlainTextEdit>
#include <warn/pop>

namespace inviwo {

class MultilineTextEdit;

/*! \class StringMultilinePropertyWidgetQt
    \brief Property widget for string properties which shows the contents spread over
    multiple lines. The height of the text editor is adjusted based on the contents and
    given defaults (2 to 10 lines). Changes are committed when the focus changes or
    CTRL + Return or CTRL + Enter is pressed.
*/
class IVW_QTWIDGETS_API StringMultilinePropertyWidgetQt : public PropertyWidgetQt {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    StringMultilinePropertyWidgetQt(StringProperty *property);

    void updateFromProperty();

public slots:
    void setPropertyValue();

private:
    void generateWidget();

    StringProperty *property_;
    MultilineTextEdit *textEdit_;
    EditableLabelQt *label_;
};

/*! \class MultilineTextEdit
\brief Basic text editor based on QPlainTextEdit for showing strings in multiple lines.
The height is automatically adjusted. The editingFinished signal is emitted when the
widget looses focus or CTRL + Return or CTRL + Enter is pressed.
*/
class IVW_QTWIDGETS_API MultilineTextEdit : public QPlainTextEdit {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    MultilineTextEdit(QWidget *parent = nullptr);
    virtual ~MultilineTextEdit();

    void adjustHeight();
signals:
    void editingFinished();

protected:
    virtual void focusOutEvent(QFocusEvent *e) override;
    virtual void resizeEvent(QResizeEvent *e) override;
    virtual void keyPressEvent(QKeyEvent *e) override;
    virtual void contextMenuEvent(QContextMenuEvent *e) override;

private:
    const int minLineCount_;
    const int maxLineCount_;
    int lineHeight_;

    bool showContextMenu_;
};

}  // namespace

#endif  // IVW_STRINGMULTILINEPROPERTYWIDGETQT_H
