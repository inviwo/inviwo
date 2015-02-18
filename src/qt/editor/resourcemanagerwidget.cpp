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

#include <inviwo/qt/editor/resourcemanagerwidget.h>
#include <inviwo/core/resources/resourcemanager.h>

#include <QStandardItem>
#include <string>

namespace inviwo {

ResourceManagerWidget::ResourceManagerWidget(QWidget* parent) : InviwoDockWidget(tr("Resources"), parent), ResourceManagerObserver() {
    setObjectName("ResourceManagerWidget");
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    ResourceManager::getPtr()->addObserver(this);
    buildLayout();
}

ResourceManagerWidget::~ResourceManagerWidget() {}


void ResourceManagerWidget::buildLayout() {
    // Components needed for layout
    QFrame* frame_ = new QFrame();
    QVBoxLayout* mainLayout = new QVBoxLayout();
    listView_ = new QListView();
    model_ = new QStandardItemModel(listView_);
    listView_->setModel(model_);
    mainLayout->addWidget(listView_);
    frame_->setLayout(mainLayout);
    setWidget(frame_);
}

void ResourceManagerWidget::resourceAdded(const Resource* resource)
{
    QStandardItem* item = new QStandardItem(QString::fromStdString(resource->getIdentifier()));
    item->setEditable(false);
    model_->appendRow(item);
}

void ResourceManagerWidget::resourceRemoved(const Resource* resource)
{
    QList<QStandardItem*> foundItem = model_->findItems(QString::fromStdString(resource->getIdentifier()));

    for (QList<QStandardItem*>::iterator it = foundItem.begin(); it != foundItem.end(); ++it)
        model_->removeRow(model_->indexFromItem(*it).row());
}

void ResourceManagerWidget::keyPressEvent(QKeyEvent* keyEvent)
{
	if (keyEvent->key() == Qt::Key_Delete)
        removeSelectedItems();
}

void ResourceManagerWidget::removeSelectedItems()
{
    listView_->setUpdatesEnabled(false);
    //Destruction of QModelIndexList creates a segmentation error for some reason (Qt 4.8.1)
    //QModelIndexList indexes = listView_->selectionModel()->selectedIndexes();
    std::vector<int> rows;

    for (int i=0; i<model_->rowCount(); i++)
        if (listView_->selectionModel()->isRowSelected(i, QModelIndex())) rows.push_back(i);

    qSort(rows.begin(), rows.end(), qGreater<int>());

    for (size_t i=0; i<rows.size(); i++) {
        std::string resrcIdentifier(model_->item(rows[i])->text().toLocal8Bit().constData());
        ResourceManager::getPtr()->removeResource(resrcIdentifier);
    }

    listView_->setUpdatesEnabled(true);
}



} // namespace