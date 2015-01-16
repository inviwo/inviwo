/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#ifndef IVW_HTMLEDITORYWIDGETQT_H
#define IVW_HTMLEDITORYWIDGETQT_H

//QT includes
#include <QFile>
#include <QCheckBox>
#include <QGridLayout>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTextStream>
#include <QToolBar>
#include <QToolButton>
#include <QWidget>
#include <QMainWindow>
#include <QDesktopServices>
#include <QUrl>

#include <inviwo/qt/widgets/properties/htmllistwidgetqt.h>

namespace inviwo {

class TextEditorWidgetQt;
class IVW_QTWIDGETS_API HtmlEditorWidgetQt : public QWidget {

    Q_OBJECT

public:
    HtmlEditorWidgetQt();
    bool saveDialog();
    void setParent(TextEditorWidgetQt*);

    QFile* file_;
    QTextEdit* htmlEditor_;
    QTextEdit* htmlOutput_;
    TextEditorWidgetQt* mainParentWidget_;
    QToolBar* toolBar_;
    QToolButton* runButton_;
    QToolButton* saveButton_;
    QToolButton* reLoadButton_;
    std::string tmpPropertyValue_;
    HtmlTreeWidget* htmlTreeWidgetQt_;


    void generateWidget();

public slots:
    void run();

protected:
    void closeEvent(QCloseEvent*);
    /*void showEvent(QShowEvent *);*/
};

}//namespace

#endif //IVW_HTMLEDITORYWIDGETQT_H