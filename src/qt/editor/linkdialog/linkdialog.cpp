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

#include <inviwo/qt/editor/linkdialog/linkdialog.h>
#include <inviwo/qt/editor/linkdialog/linkdialoggraphicsitems.h>
#include <inviwo/qt/editor/linkdialog/linkdialogscene.h>
#include <inviwo/qt/editor/linkdialog/linkdialogview.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/links/linkconditions.h>
#include <inviwo/core/common/inviwoapplication.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QSettings>
#include <warn/pop>

namespace inviwo {

LinkDialog::LinkDialog(Processor* src, Processor* dst, QWidget* parent)
    : InviwoDockWidget("Edit Property Links", parent), src_(src), dest_(dst) {
    
    setFloating(true);

    setObjectName("LinkDialogWidget");
    setAllowedAreas(Qt::NoDockWidgetArea);
    setFixedWidth(linkdialog::dialogWidth);
    setMinimumHeight(linkdialog::dialogHeight);

    QFrame* frame = new QFrame();
    setWidget(frame);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(frame);
    linkDialogView_ = new LinkDialogGraphicsView(this);
    linkDialogView_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    linkDialogView_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mainLayout->addWidget(linkDialogView_);
        
    // smart link button
    QHBoxLayout* smartLinkPushButtonLayout = new QHBoxLayout;
    smartLinkPushButtonLayout->setAlignment(Qt::AlignLeft);

    // checkable combo box
    std::vector<std::string> options;
    options.push_back(SimpleCondition::conditionName());
    options.push_back(PartiallyMatchingIdCondition::conditionName());
    smartLinkOptions_ = new CheckableQComboBox(this, "AutoLink Filter", options);
    smartLinkPushButtonLayout->addWidget(smartLinkOptions_, 20);

    // smart link button
    smartLinkPushButton_ = new QPushButton("SmartLink", this);
    connect(smartLinkPushButton_, SIGNAL(clicked()), this, SLOT(clickedSmartLinkPushButton()));
    smartLinkPushButtonLayout->addWidget(smartLinkPushButton_, 10);

    // delete button
    deleteAllLinkPushButton_ = new QPushButton("Delete All", this);
    connect(deleteAllLinkPushButton_, SIGNAL(clicked()), this,
            SLOT(clickedDeleteAllLinksPushButton()));
    smartLinkPushButtonLayout->addWidget(deleteAllLinkPushButton_, 10);

    // expand composite
    expandCompositeButton_ = new QPushButton("Expand/Collapse", this);
    expandCompositeButton_->setChecked(false);
    smartLinkPushButtonLayout->addWidget(expandCompositeButton_, 10);
    connect(expandCompositeButton_, SIGNAL(clicked()), this, SLOT(expandCompositeProperties()));
    mainLayout->addLayout(smartLinkPushButtonLayout);
    

    QSettings settings("Inviwo", "Inviwo");
    settings.beginGroup("linkwindow");
    if (settings.contains("geometry")) {
        restoreGeometry(settings.value("geometry").toByteArray());
    }
    settings.endGroup();
        
    linkDialogScene_ = new LinkDialogGraphicsScene(
        this, InviwoApplication::getPtr()->getProcessorNetwork(), src, dst);
    linkDialogScene_->setSceneRect(0.0, 0.0, linkdialog::dialogWidth, size().height());
    linkDialogView_->setDialogScene(linkDialogScene_);

    connect(linkDialogScene_, SIGNAL(closeDialog()), this, SLOT(closeLinkDialog()));
}

LinkDialog::~LinkDialog() {}


void LinkDialog::closeLinkDialog() {
    hide();

    QSettings settings("Inviwo", "Inviwo");
    settings.beginGroup("linkwindow");
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();

    eventLoop_.quit();
}

void LinkDialog::closeEvent(QCloseEvent* event) { closeLinkDialog(); }

void LinkDialog::clickedSmartLinkPushButton() {
    std::vector<Property*> srcProperties = src_->getProperties();
    std::vector<Property*> dstProperties = dest_->getProperties();
    int selectedTypes = (int)NoLinkCondition;
    std::vector<std::string> selectedConditons = smartLinkOptions_->getCheckedItems();

    for (auto& selectedConditon : selectedConditons) {
        if (selectedConditon == SimpleCondition::conditionName())
            selectedTypes |= SimpleCondition::conditionType();

        if (selectedConditon == PartiallyMatchingIdCondition::conditionName())
            selectedTypes |= PartiallyMatchingIdCondition::conditionType();
    }

    for (auto& srcPropertie : srcProperties) {
        for (auto& dstPropertie : dstProperties) {
            bool linkSubProperties = linkDialogScene_->isPropertyExpanded(srcPropertie) ||
                                     linkDialogScene_->isPropertyExpanded(dstPropertie);

            if (linkSubProperties) {
                if (AutoLinker::canLink(srcPropertie, dstPropertie,
                                        (LinkingConditions)selectedTypes)) {
                    CompositeProperty* compSrc = dynamic_cast<CompositeProperty*>(srcPropertie);
                    CompositeProperty* compDst = dynamic_cast<CompositeProperty*>(dstPropertie);
                    if (compSrc && compDst) {
                        // If composite property then try to link sub-properties only
                        std::vector<Property*> s = compSrc->getProperties();
                        std::vector<Property*> d = compDst->getProperties();
                        for (auto& elem : s) {
                            for (auto& d_jj : d) {
                                if (AutoLinker::canLink(elem, d_jj,
                                                        (LinkingConditions)selectedTypes)) {
                                    linkDialogScene_->addPropertyLink(elem, d_jj, true);
                                }
                            }
                        }
                    }
                }
            } else {
                if (AutoLinker::canLink(srcPropertie, dstPropertie,
                                        (LinkingConditions)selectedTypes))
                    linkDialogScene_->addPropertyLink(srcPropertie, dstPropertie, true);
            }
        }
    }
}

void LinkDialog::clickedDeleteAllLinksPushButton() { linkDialogScene_->removeAllPropertyLinks(); }

void LinkDialog::expandCompositeProperties() { linkDialogScene_->toggleExpand(); }

int LinkDialog::exec() {
    eventLoop_.exit();
    show();
    return eventLoop_.exec();
}

QSize LinkDialog::sizeHint() const {
    QSize size = layout()->sizeHint();
    size.setHeight(linkdialog::dialogHeight);
    return size;
}

CheckableQComboBox::CheckableQComboBox(QWidget* parent, std::string widgetName,
                                       std::vector<std::string> options)
    : QComboBox(parent), widgetName_(widgetName) {
    setEditable(true);
    lineEdit()->setReadOnly(true);
    stdandardModel_ = new QStandardItemModel(static_cast<int>(options.size()), 1);

    for (size_t i = 0; i < options.size(); i++) {
        QStandardItem* item = new QStandardItem(QString(options[i].c_str()));
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        item->setData(Qt::Checked, Qt::CheckStateRole);
        stdandardModel_->setItem(static_cast<int>(i), 0, item);
        standardItems_.push_back(item);
    }

    setModel(stdandardModel_);
    lineEdit()->setText(QString(widgetName_.c_str()));
    connect(stdandardModel_, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this,
            SLOT(onSmartLinkOptionChecked(const QModelIndex&, const QModelIndex&)));
}

CheckableQComboBox::~CheckableQComboBox() { delete stdandardModel_; }

bool CheckableQComboBox::isItemChecked(int i) {
    if (i > (int)standardItems_.size()) return false;

    QStandardItem* item = standardItems_[i];

    if (item->checkState() == Qt::Checked) return true;

    return false;
}

std::vector<std::string> CheckableQComboBox::getCheckedItems() {
    std::vector<std::string> checkedItemString;

    for (size_t i = 0; i < standardItems_.size(); i++)
        if (isItemChecked(static_cast<int>(i)))
            checkedItemString.push_back(standardItems_[i]->text().toLocal8Bit().constData());

    return checkedItemString;
}

void CheckableQComboBox::onSmartLinkOptionChecked(const QModelIndex& tl, const QModelIndex& br) {
    if (isItemChecked(tl.row())) {
        // do some maintenance stuff here if required
    }

    lineEdit()->setText(QString(widgetName_.c_str()));
}

}  // namespace