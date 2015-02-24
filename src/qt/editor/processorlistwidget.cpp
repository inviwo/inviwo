/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QMimeData>
#include <QHeaderView>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/qt/editor/processorlistwidget.h>
#include <inviwo/qt/editor/helpwidget.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>

namespace inviwo {

const int ProcessorTree::IDENTIFIER_ROLE = Qt::UserRole+1;
    
void ProcessorTree::mousePressEvent(QMouseEvent* e) {
    if (e->buttons() & Qt::LeftButton)
        dragStartPosition_ = e->pos();

    QTreeWidget::mousePressEvent(e);
}

void ProcessorTree::mouseMoveEvent(QMouseEvent* e) {
    if (e->buttons() & Qt::LeftButton) {
        if ((e->pos() - dragStartPosition_).manhattanLength() < QApplication::startDragDistance())
            return;

        QTreeWidgetItem* selectedProcessor = itemAt(dragStartPosition_);

        if (selectedProcessor && selectedProcessor->parent())
            new ProcessorDragObject(this, selectedProcessor->data(0, IDENTIFIER_ROLE).toString());
    }
}

ProcessorTree::ProcessorTree(QWidget* parent) 
    : QTreeWidget(parent) {
}

ProcessorTreeWidget::ProcessorTreeWidget(QWidget* parent, HelpWidget* helpWidget) 
    : InviwoDockWidget(tr("Processors"), parent) 
    , helpWidget_(helpWidget) {
    setObjectName("ProcessorTreeWidget");
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    QFrame* frame = new QFrame();
    QVBoxLayout* vLayout = new QVBoxLayout(frame);
    vLayout->setSpacing(7);
    vLayout->setContentsMargins(7, 7, 7, 7);
    lineEdit_ = new QLineEdit(frame);
    lineEdit_->setPlaceholderText("Filter processor list...");
    connect(lineEdit_, SIGNAL(textChanged(const QString&)), this, SLOT(addProcessorsToTree()));
    vLayout->addWidget(lineEdit_);
    QHBoxLayout* listViewLayout = new QHBoxLayout();
    listViewLayout->addWidget(new QLabel("Group processors by",frame));
    listView_ = new QComboBox(frame);
    listView_->addItem("Alphabet");
    listView_->addItem("Category");
    listView_->addItem("Code State");
    listView_->addItem("Module");
    listView_->setCurrentIndex(1);
    connect(listView_, SIGNAL(currentIndexChanged(int)), this, SLOT(addProcessorsToTree()));
    listView_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    listViewLayout->addWidget(listView_);
    vLayout->addLayout(listViewLayout);

    iconStable_ = QIcon(":/icons/processor_stable.png");
    iconExperimental_ = QIcon(":/icons/processor_experimental.png");
    iconBroken_ = QIcon(":/icons/processor_broken.png");
    
    processorTree_ = new ProcessorTree(this);
    processorTree_->setHeaderHidden(true);
    processorTree_->setColumnCount(2);
    processorTree_->setIndentation(10);
    processorTree_->setAnimated(true);
    processorTree_->header()->setStretchLastSection(false);
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    processorTree_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    processorTree_->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    #else
    processorTree_->header()->setResizeMode(0, QHeaderView::Stretch);
    processorTree_->header()->setResizeMode(1, QHeaderView::Fixed);
    #endif
    processorTree_->header()->setDefaultSectionSize(40);
    
    connect(processorTree_, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
            this, SLOT(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));

    addProcessorsToTree();
    vLayout->addWidget(processorTree_);
    frame->setLayout(vLayout);
    setWidget(frame);
}

ProcessorTreeWidget::~ProcessorTreeWidget() {}

bool ProcessorTreeWidget::processorFits(ProcessorFactoryObject* processor, const QString& filter) {
    return (QString::fromStdString(processor->getDisplayName()).contains(filter, Qt::CaseInsensitive) ||
            QString::fromStdString(processor->getClassIdentifier()).contains(filter, Qt::CaseInsensitive) ||
            QString::fromStdString(processor->getTags().getString()).contains(filter, Qt::CaseInsensitive));
}

const QIcon* ProcessorTreeWidget::getCodeStateIcon(CodeState state) const {
    switch (state) {
        case CODE_STATE_STABLE:
            return &iconStable_;

        case CODE_STATE_BROKEN:
            return &iconBroken_;

        case CODE_STATE_EXPERIMENTAL:
        default:
            return &iconExperimental_;
    }
}

std::string ProcessorTreeWidget::getCodeStateString(CodeState state) const {
    switch (state) {
        case CODE_STATE_STABLE:
            return "Stable";
        case CODE_STATE_BROKEN:
            return "Broken";
        case CODE_STATE_EXPERIMENTAL:
            return "Experimental";
        default:
            return "Unknown";
    }
}

QTreeWidgetItem* ProcessorTreeWidget::addToplevelItemTo(QString title) {
    QTreeWidgetItem* newItem = new QTreeWidgetItem(QStringList(title));
    
    processorTree_->addTopLevelItem(newItem);
    processorTree_->setFirstItemColumnSpanned(newItem, true);
    
    return newItem;
}
    
QTreeWidgetItem* ProcessorTreeWidget::addProcessorItemTo(QTreeWidgetItem* item,
                                                         ProcessorFactoryObject* processor,
                                                         std::string moduleId) {
    QTreeWidgetItem* newItem = new QTreeWidgetItem();
    newItem->setIcon(0, *getCodeStateIcon(processor->getCodeState()));
    newItem->setText(0, QString::fromStdString(processor->getDisplayName()));
    newItem->setText(1, QString::fromStdString(processor->getTags().getString() + " "));
    newItem->setTextAlignment(1, Qt::AlignRight);
    newItem->setData(0, ProcessorTree::IDENTIFIER_ROLE,
                     QString::fromStdString(processor->getClassIdentifier()));

    QString str(QString("<html><head/><body>\
        <b style='color:white;'>%1</b>\
        <table border='0' cellspacing='0' cellpadding='0' style='border-color:white;white-space:pre;'>\
        <tr><td style='color:#bbb;padding-right:8px;'>Module:</td><td style='color:#d06060;'><nobr>%5</nobr></td>\
        <tr><td style='color:#bbb;padding-right:8px;'>Identifier:</td><td><nobr>%2</nobr></td></tr>\
        <tr><td style='color:#bbb;padding-right:8px;'>Category:</td><td><nobr>%3</nobr></td></tr>\
        <tr><td style='color:#bbb;padding-right:8px;'>Code State:</td><td><nobr>%4</nobr></td></tr>\
        <tr><td style='color:#bbb;padding-right:8px;'>Tags:</td><td><nobr>%6</nobr></td></tr>\
        </tr></table></body></html>")
        .arg(processor->getDisplayName().c_str())
        .arg(processor->getClassIdentifier().c_str())
        .arg(processor->getCategory().c_str())
        .arg(getCodeStateString(processor->getCodeState()).c_str())
        .arg(moduleId.c_str())
        .arg(processor->getTags().getString().c_str())
        );

    newItem->setToolTip(0, str);

    QFont font = newItem->font(1);
    font.setWeight(QFont::Bold);
    newItem->setFont(1, font);
    
    item->addChild(newItem);

    if (processor->getTags().tags_.size() == 0) {
        processorTree_->setFirstItemColumnSpanned(newItem, true);
    }

    return newItem;
}
    

void ProcessorTreeWidget::addProcessorsToTree() {
    processorTree_->clear();
    // add processors from all modules to the list
    InviwoApplication* inviwoApp = InviwoApplication::getPtr();

    if (listView_->currentIndex() == 2) {
        addToplevelItemTo("Stable Processors");
        addToplevelItemTo("Experimental Processors");
        addToplevelItemTo("Broken Processors");
    }

    for (auto& elem : inviwoApp->getModules()) {
        std::vector<ProcessorFactoryObject*> curProcessorList = elem->getProcessors();

        QList<QTreeWidgetItem*> items;
        for (auto& processor : curProcessorList) {
            if (lineEdit_->text().isEmpty() || processorFits(processor, lineEdit_->text())) {
                std::string categoryName;

                switch (listView_->currentIndex()) {
                    case 0:  // By Alphabet
                        categoryName = processor->getDisplayName().substr(0, 1);
                        break;
                    case 1:  // By Category
                        categoryName = processor->getCategory();
                        break;
                    case 2:  // By Code State
                        categoryName = getCodeStateString(processor->getCodeState());
                        break;
                    case 3:  // By Module
                        categoryName = elem->getIdentifier();
                        break;
                    default:
                        categoryName = "Unkonwn";
                }

                QString category = QString::fromStdString(categoryName);
                items = processorTree_->findItems(category, Qt::MatchFixedString, 0);

                if (items.empty()) items.push_back(addToplevelItemTo(category));
                addProcessorItemTo(items[0], processor, elem->getIdentifier());
            }
        }
    }

    // Apply sorting
    switch (listView_->currentIndex()) {
        case 2: {  // By Code State
            int i = 0;
            while (i < processorTree_->topLevelItemCount()) {
                QTreeWidgetItem* item = processorTree_->topLevelItem(i);
                if (item->childCount() == 0) {
                    delete processorTree_->takeTopLevelItem(i);
                } else {
                    item->sortChildren(0, Qt::AscendingOrder);
                    i++;
                }
            }
            break;
        }
        default:
            processorTree_->sortItems(0, Qt::AscendingOrder);
            break;
    }

    processorTree_->expandAll();
    processorTree_->resizeColumnToContents(1);
}

void ProcessorTreeWidget::currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous) {
    if(!current) return;
    std::string classname = current->data(0, ProcessorTree::IDENTIFIER_ROLE).toString().toUtf8().constData();
    if (!classname.empty()) helpWidget_->showDocForClassName(classname);
}

static QString mimeType = "inviwo/ProcessorDragObject";

ProcessorDragObject::ProcessorDragObject(QWidget* source, const QString className) : QDrag(source) {
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

bool ProcessorDragObject::canDecode(const QMimeData* mimeData) {
    if (mimeData->hasFormat(mimeType)) return true;
    else return false;
}

bool ProcessorDragObject::decode(const QMimeData* mimeData, QString& className) {
    QByteArray byteData = mimeData->data(mimeType);

    if (byteData.isEmpty())
        return false;

    QDataStream ds(&byteData, QIODevice::ReadOnly);
    ds >> className;
    return true;
}

} // namespace