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

#ifndef IVW_TEXTEDITORWIDGETQT_H
#define IVW_TEXTEDITORWIDGETQT_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>

//QT includes
#include <warn/push>
#include <warn/ignore/all>
#include <QWidget>
#include <warn/pop>

//Property includes
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/property.h>
#include "propertywidgetqt.h"
#include <inviwo/qt/widgets/editablelabelqt.h>


class QFile;
class QTextEdit;
class QToolBar;
class QToolButton;
class QCheckBox;

namespace inviwo {
class SyntaxHighligther;
class FilePropertyWidgetQt;
class TextEditorWidgetQt;
class HtmlEditorWidgetQt;
class StringPropertyWidgetQt;

class IVW_QTWIDGETS_API ModifiedWidget : public QWidget {

    Q_OBJECT

public:
    ModifiedWidget();
    bool saveDialog();
    void setParent(TextEditorWidgetQt*);

    QFile* file_;
    QTextEdit* textEditor_;
    TextEditorWidgetQt* mainParentWidget_;
    QToolBar* toolBar_;
    QToolButton* reDoButton_;
    QToolButton* reLoadButton_;
    QToolButton* saveButton_;
    QToolButton* unDoButton_;
    std::string tmpPropertyValue_;

    SyntaxHighligther* getSyntaxHighligther();

    void generateWidget();

public slots:
    void textHasChanged();

protected:
    void closeEvent(QCloseEvent*);

private:
    SyntaxHighligther* syntaxHighligther_;
};

class IVW_QTWIDGETS_API TextEditorWidgetQt : public PropertyWidgetQt {

    Q_OBJECT

public:

    TextEditorWidgetQt(Property* property);
    virtual ~TextEditorWidgetQt();
    void updateFromProperty();
    SyntaxHighligther* getSyntaxHighligther();
public:
    bool saveDialog();


private:
    QToolButton* btnEdit_;
    FilePropertyWidgetQt* fileWidget_;
    QFile* file_;
    ModifiedWidget* textEditorWidget_;
    HtmlEditorWidgetQt* htmlEditorWidget_;
    StringPropertyWidgetQt* stringWidget_;
    std::string tmpPropertyValue_;
    QCheckBox* checkBox_;
    EditableLabelQt* label_;

    void generateWidget();

public slots:
    void loadFile();
    void loadString();
    void editFile();
    void editString();
    void setPropertyValue();
    bool writeToFile();
    bool writeToString();
};


}//namespace

#endif //IVW_TEXTEDITORWIDGETQT_H