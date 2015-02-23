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

#include <inviwo/core/properties/property.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/qt/widgets/properties/collapsiblegroupboxwidgetqt.h>
#include <inviwo/qt/widgets/propertyselectiontreewidget.h>
#include <inviwo/qt/widgets/inviwoapplicationqt.h>
#include <apps/inviwo/inviwomainwindow.h>
#include <QDialogButtonBox>
#include <QMouseEvent>

namespace inviwo {


void PropertySelectionTree::mousePressEvent(QMouseEvent* e) {
    if (e->buttons() & Qt::LeftButton)
        dragStartPosition_ = e->pos();
    else if (e->buttons() & Qt::RightButton) {
        QTreeWidgetItem* selectedItem = itemAt(e->pos());

        if (selectedItem && !selectedItem->parent()) {
            QMenu menu;
            QAction* deleteWorkspace = menu.addAction(tr("Delete"));
            QAction* result = menu.exec(QCursor::pos());

            if (result == deleteWorkspace)
                delete selectedItem;
        }
    }

    QTreeWidget::mousePressEvent(e);
}

void PropertySelectionTree::mouseMoveEvent(QMouseEvent* e) {
    if (e->buttons() & Qt::LeftButton) {
        if ((e->pos() - dragStartPosition_).manhattanLength() < QApplication::startDragDistance())
            return;

        QTreeWidgetItem* selectedProperty = itemAt(dragStartPosition_);

        if (selectedProperty && selectedProperty->parent()) {
            //create drag object
        }
    }
}

PropertySelectionTreeWidget::PropertySelectionTreeWidget() : QWidget() {
    vLayout_ = new QVBoxLayout();
    propertySelectionTree_ = new PropertySelectionTree(this);
    propertySelectionTree_->setHeaderHidden(true);
    propertySelectionTree_->setExpandsOnDoubleClick(false);
    vLayout_->addWidget(propertySelectionTree_);
    setLayout(vLayout_);
}

PropertySelectionTreeWidget::~PropertySelectionTreeWidget() {}

void PropertySelectionTreeWidget::addProcessorNetwork(ProcessorNetwork* processorNetwork, std::string workspaceFileName) {
    std::vector<Processor*> processors = processorNetwork->getProcessors();
    QTreeWidgetItem* worksapceItem = new QTreeWidgetItem(QStringList(QString::fromStdString(workspaceFileName)));

    if (processors.size())
        propertySelectionTree_->addTopLevelItem(worksapceItem);
    else {
        LogWarn("Empty workpace with no processors" << workspaceFileName);
        return;
    }

    for (auto& processor : processors) {
        std::vector<Property*> properties = processor->getProperties();
        std::string processorId = processor->getIdentifier();
        QTreeWidgetItem* processorItem = new QTreeWidgetItem(QStringList(QString::fromStdString(processorId)));
        worksapceItem->addChild(processorItem);

        for (auto& propertie : properties) {
            std::string id = propertie->getIdentifier();
            QTreeWidgetItem* newItem = new QTreeWidgetItem(QStringList(QString::fromStdString(id)));
            processorItem->addChild(newItem);
            newItem->setFlags(newItem->flags() | Qt::ItemIsUserCheckable);
            newItem->setCheckState(0, Qt::Unchecked);
        }

        propertySelectionTree_->addTopLevelItem(processorItem);
        processorItem->sortChildren(0, Qt::AscendingOrder);
    }

    propertySelectionTree_->expandAll();
}

void PropertySelectionTreeWidget::clear() {
    propertySelectionTree_->clear();
}

std::vector<Property*> PropertySelectionTreeWidget::getSelectedProperties(ProcessorNetwork* processorNetwork) {
    std::vector<Property*> selectedProperties;
    int workspaceCount  = propertySelectionTree_->topLevelItemCount();

    for (int i=0; i<workspaceCount; i++) {
        QTreeWidgetItem* wrokspaceItem = propertySelectionTree_->topLevelItem(i);
        int processorCount = wrokspaceItem->childCount();

        for (int j=0; j<processorCount; j++) {
            QTreeWidgetItem* processorItem = wrokspaceItem->child(j);
            int propertyCount = processorItem->childCount();
            QString qproecessorId = processorItem->text(0);
            std::string proecessorId = qproecessorId.toLocal8Bit().constData();

            for (int k=0; k<propertyCount; k++) {
                QTreeWidgetItem* propertyItem = processorItem->child(k);

                if (propertyItem->checkState(0) == Qt::Checked) {
                    QString propertyId = propertyItem->text(0);
                    std::string prop = propertyId.toLocal8Bit().constData();
                    QString workspaceName = wrokspaceItem->text(0);
                    std::string workspace = workspaceName.toLocal8Bit().constData();
                    Processor* processor = processorNetwork->getProcessorByIdentifier(proecessorId);
                    Property* selectedProperty = processor->getPropertyByIdentifier(prop);

                    if (selectedProperty)
                        selectedProperties.push_back(selectedProperty);
                }
            }
        }
    }

    return selectedProperties;
}


PropertySelectionTreeDialog::PropertySelectionTreeDialog(ProcessorNetwork* processorNetwork,
        std::vector<Property*>& selectedProperty,
        QWidget* parent)
    : QDialog(parent), selectedProperties_(selectedProperty),
      processorNetwork_(processorNetwork)
{
    initDialog();
    selectionTree_->addProcessorNetwork(processorNetwork);
}

void PropertySelectionTreeDialog::initDialog() {
    std::string title = std::string("Property Selection");
    setWindowTitle(tr(title.c_str()));
    QSize rSize(384,512);
    setFixedSize(rSize);
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    selectionTree_ = new PropertySelectionTreeWidget();
    mainLayout->addWidget(selectionTree_);
    QHBoxLayout* okayCancelButtonLayout = new QHBoxLayout;
    okayCancelButtonLayout->setAlignment(Qt::AlignRight);
    QDialogButtonBox* okayCancelbuttonBox_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect(okayCancelbuttonBox_, SIGNAL(accepted()), this, SLOT(clickedOkayButton()));
    connect(okayCancelbuttonBox_, SIGNAL(rejected()), this, SLOT(clickedCancelButton()));
    okayCancelButtonLayout->addWidget(okayCancelbuttonBox_);
    mainLayout->addLayout(okayCancelButtonLayout);
}

void PropertySelectionTreeDialog::clickedOkayButton() {
    accept();
    selectedProperties_ = selectionTree_->getSelectedProperties(processorNetwork_);
}

void PropertySelectionTreeDialog::clickedCancelButton() {
    reject();
}



} // namespace