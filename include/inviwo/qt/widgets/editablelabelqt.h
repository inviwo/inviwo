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

#ifndef EDITABLELABELQT_H
#define EDITABLELABELQT_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <inviwo/qt/widgets/properties/propertywidgetqt.h>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QMenu>
namespace inviwo {

class IVW_QTWIDGETS_API EditableLabelQt: public QWidget {
    Q_OBJECT
public:
    EditableLabelQt(QWidget* parent, std::string text, bool shortenText=true);
    EditableLabelQt(PropertyWidgetQt* parent, std::string text, bool shortenText=true);
    std::string getText() {return text_;};
    void setText(std::string txt);
    void setContextMenu(QMenu* menu) {contextMenu_ = menu;};
    void setShortenText(bool shorten);
    
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    
public slots:
    void edit();
    void finishEditing();
    void showContextMenu(const QPoint& pos);

protected:
    virtual void resizeEvent(QResizeEvent *);

private:
    QLabel* label_;
    QLineEdit* lineEdit_;
    std::string text_;
    void generateWidget();
    PropertyWidgetQt* propertyWidget_;
    QMenu* contextMenu_;
    QAction* renameAction_;
    void mouseDoubleClickEvent(QMouseEvent* e);
    QString shortenText();
    bool shortenText_;

signals:
    void textChanged();
};
}//namespace

#endif //EDITABLELABELQT_H