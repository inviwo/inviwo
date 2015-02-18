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

#include <QApplication>
#include <QLayout>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QWidget>
#include <QMimeData>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/processors/processorfactoryobject.h>
#include <inviwo/qt/widgets/properties/htmllistwidgetqt.h>

namespace inviwo {

void HtmlTree::mousePressEvent(QMouseEvent* e) {
    if (e->buttons() & Qt::LeftButton)
        dragStartPosition_ = e->pos();

    QTreeWidget::mousePressEvent(e);
}

void HtmlTree::mouseMoveEvent(QMouseEvent* e) {
    if (e->buttons() & Qt::LeftButton) {
        if ((e->pos() - dragStartPosition_).manhattanLength() < QApplication::startDragDistance())
            return;

        QTreeWidgetItem* selectedProcessor = itemAt(dragStartPosition_);

        if (selectedProcessor)
            new HtmlDragObject(this, selectedProcessor->text(0));
    }
}

HtmlTreeWidget::HtmlTreeWidget(QWidget* parent): QWidget(parent) {
    QVBoxLayout* vLayout = new QVBoxLayout();
    QLineEdit* lineEdit = new QLineEdit(this);
    lineEdit->setPlaceholderText("Filter Html object list...");
    connect(lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(addTagsToTree(const QString&)));
    vLayout->addWidget(lineEdit);
    processorTree_ = new HtmlTree(this);
    processorTree_->setHeaderHidden(true);
    addTagsToTree();
    processorTree_->expandAll();
    vLayout->addWidget(processorTree_);
    setLayout(vLayout);
}

HtmlTreeWidget::~HtmlTreeWidget() {}

void HtmlTreeWidget::addTagsToTree(const QString& text) {
    processorTree_->clear();
    //Add tag items
    processorTree_->sortItems(0, Qt::AscendingOrder);
}

static QString mimeType = "inviwo/HtmlDragObject";

HtmlDragObject::HtmlDragObject(QWidget* source, const QString className) : QDrag(source) {
    QByteArray byteData;
    {
        QDataStream ds(&byteData, QIODevice::WriteOnly);
        ds << className;
    }
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(mimeType, byteData);
    mimeData->setData("text/plain", className.toLatin1().data());
    setMimeData(mimeData);
    start(Qt::MoveAction);
}

bool HtmlDragObject::canDecode(const QMimeData* mimeData) {
    if (mimeData->hasFormat(mimeType)) return true;
    else return false;
}

bool HtmlDragObject::decode(const QMimeData* mimeData, QString& className) {
    QByteArray byteData = mimeData->data(mimeType);

    if (byteData.isEmpty())
        return false;

    QDataStream ds(&byteData, QIODevice::ReadOnly);
    ds >> className;
    return true;
}

} // namespace