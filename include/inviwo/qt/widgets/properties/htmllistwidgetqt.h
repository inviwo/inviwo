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

#ifndef IVW_HTMLLISTWIDGETQT_H
#define IVW_HTMLLISTWIDGETQT_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QTreeWidget>
#include <QDrag>
#include <warn/pop>

namespace inviwo {

class ProcessorFactoryObject;

class IVW_QTWIDGETS_API HtmlTree : public QTreeWidget {

public:
    HtmlTree(QWidget* parent) : QTreeWidget(parent) {}
    ~HtmlTree() {}

protected:
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);

private:
    QPoint dragStartPosition_;
};

class IVW_QTWIDGETS_API HtmlTreeWidget : public QWidget {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    HtmlTreeWidget(QWidget* parent=0);
    ~HtmlTreeWidget();

private:
    HtmlTree* processorTree_;
    QPoint dragStartPosition_;

    bool processorFits(ProcessorFactoryObject* processor, const QString& filter);

private slots:
    void addTagsToTree(const QString& text="");
};

class IVW_QTWIDGETS_API HtmlDragObject : public QDrag {
public:
    HtmlDragObject(QWidget* source, const QString className);

    static bool canDecode(const QMimeData* mimeData);
    static bool decode(const QMimeData* mimeData, QString& className);
};

} // namespace

#endif // IVW_HTMLLISTWIDGETQT_H