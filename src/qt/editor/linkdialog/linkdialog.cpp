/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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
#include <inviwo/core/links/linkconditions.h>
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

LinkDialog::LinkDialog(Processor* src, Processor* dest, QWidget* parent)
    : InviwoDockWidget("Edit Property Links", parent)
    , src_(src)
    , dest_(dest) {

    initDialogLayout();
    //Network is required to add property links created in dialog (or remove )
    linkDialogScene_->setNetwork(InviwoApplication::getPtr()->getProcessorNetwork());
    linkDialogScene_->setExpandProperties(false);
    linkDialogScene_->initScene(src_, dest_);
}

LinkDialog::~LinkDialog() {}

void LinkDialog::initDialogLayout() {
    setFloating(true);

    setObjectName("LinkDialogWidget");
    setAllowedAreas(Qt::NoDockWidgetArea);
    QFrame* frame = new QFrame();

    QSize rSize(linkDialogWidth, linkDialogHeight + 100);
    setFixedWidth(rSize.width());
    QVBoxLayout* mainLayout = new QVBoxLayout(frame);
    linkDialogView_ = new LinkDialogGraphicsView(this);
    linkDialogScene_ = new LinkDialogGraphicsScene(this);
    linkDialogView_->setDialogScene(linkDialogScene_);
    linkDialogView_->fitInView(linkDialogView_->rect());
    linkDialogView_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mainLayout->addWidget(linkDialogView_);
    QHBoxLayout* commonButtonLayout = new QHBoxLayout;

    connect(linkDialogScene_, SIGNAL(closeDialog()), this, SLOT(clickedOkayButton()));

    // smart link button
    QHBoxLayout* smartLinkPushButtonLayout = new QHBoxLayout;
    smartLinkPushButtonLayout->setAlignment(Qt::AlignLeft);

    // smart link button
    smartLinkPushButton_ = new QPushButton("SmartLink", this);
    connect(smartLinkPushButton_, SIGNAL(clicked()), this, SLOT(clickedSmartLinkPushButton()));
    smartLinkPushButtonLayout->addWidget(smartLinkPushButton_, 10);

    // checkable combo box
    std::vector<std::string> options;
    options.push_back(SimpleCondition::conditionName());
    options.push_back(PartiallyMatchingIdCondition::conditionName());
    smartLinkOptions_ = new CheckableQComboBox(this, "AutoLink Filter", options);
    smartLinkPushButtonLayout->addWidget(smartLinkOptions_, 20);

    // delete button
    deleteAllLinkPushButton_ = new QPushButton("Delete All", this);
    connect(deleteAllLinkPushButton_, SIGNAL(clicked()), this,
            SLOT(clickedDeleteAllLinksPushButton()));
    smartLinkPushButtonLayout->addWidget(deleteAllLinkPushButton_, 10);

    // expand composite
    expandCompositeButton_ = new QPushButton("Expand All Properties", this);
    expandCompositeButton_->setChecked(false);
    smartLinkPushButtonLayout->addWidget(expandCompositeButton_, 30);
    connect(expandCompositeButton_, SIGNAL(clicked()), this, SLOT(expandCompositeProperties()));
    commonButtonLayout->addLayout(smartLinkPushButtonLayout);

    // okay cancel button
    QHBoxLayout* okayCancelButtonLayout = new QHBoxLayout;
    okayCancelButtonLayout->setAlignment(Qt::AlignRight);
    okayCancelbuttonBox_ =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect(okayCancelbuttonBox_, SIGNAL(accepted()), this, SLOT(clickedOkayButton()));
    connect(okayCancelbuttonBox_, SIGNAL(rejected()), this, SLOT(clickedCancelButton()));
    okayCancelButtonLayout->addWidget(okayCancelbuttonBox_);
    commonButtonLayout->addLayout(okayCancelButtonLayout);
    mainLayout->addLayout(commonButtonLayout);
    setWidget(frame);
}

void LinkDialog::clickedOkayButton() {
    hide();
    eventLoop_.quit();
}

void LinkDialog::updateProcessorLinks() {}

void LinkDialog::clickedCancelButton() {
    linkDialogScene_->removeCurrentPropertyLinks();
    updateProcessorLinks();
    hide();
    eventLoop_.quit();
}

void LinkDialog::closeEvent(QCloseEvent* event) {
   eventLoop_.quit();
}

void LinkDialog::clickedSmartLinkPushButton() {
    std::vector<Property*> srcProperties = src_->getProperties();
    std::vector<Property*> dstProperties = dest_->getProperties();
    int selectedTypes = (int) NoLinkCondition;
    std::vector<std::string> selectedConditons = smartLinkOptions_->getCheckedItems();

    for (size_t i=0; i<selectedConditons.size(); i++) {
        if (selectedConditons[i] == SimpleCondition::conditionName())
            selectedTypes|=SimpleCondition::conditionType();

        if (selectedConditons[i] == PartiallyMatchingIdCondition::conditionName())
            selectedTypes|=PartiallyMatchingIdCondition::conditionType();
    }

    for (size_t i=0; i<srcProperties.size(); i++) {
        for (size_t j=0; j<dstProperties.size(); j++) {

            bool linkSubProperties = linkDialogScene_->isPropertyExpanded(srcProperties[i]) ||
                                     linkDialogScene_->isPropertyExpanded(dstProperties[j]);

            if (linkSubProperties) {
                if (AutoLinker::canLink(srcProperties[i], dstProperties[j], (LinkingConditions) selectedTypes)) {
                    CompositeProperty* compSrc = IS_COMPOSITE_PROPERTY(srcProperties[i]);
                    CompositeProperty* compDst = IS_COMPOSITE_PROPERTY(dstProperties[j]);
                    if ( compSrc && compDst) {
                        //If composite property then try to link sub-properties only
                        std::vector<Property*> s = compSrc->getProperties();
                        std::vector<Property*> d = compDst->getProperties();
                        for (size_t ii=0; ii<s.size(); ii++) {
                            for (size_t jj=0; jj<d.size(); jj++) {
                                if (AutoLinker::canLink(s[ii], d[jj], (LinkingConditions) selectedTypes)) {
                                    linkDialogScene_->addPropertyLink(s[ii], d[jj], true);
                                }
                            }
                        }
                    }
                }
            } else {
                if (AutoLinker::canLink(srcProperties[i], dstProperties[j], (LinkingConditions) selectedTypes))
                    linkDialogScene_->addPropertyLink(srcProperties[i], dstProperties[j], true);
            }
        }
    }
}

void LinkDialog::clickedDeleteAllLinksPushButton() {
    linkDialogScene_->removeAllPropertyLinks();
}

void LinkDialog::expandCompositeProperties() {
    linkDialogScene_->setExpandProperties(true);
    initDialog(src_, dest_);
}

void LinkDialog::initDialog(Processor* src, Processor* dest) {
    linkDialogScene_->clearSceneRepresentations();
    QSize rSize(linkDialogWidth, linkDialogHeight);
    //linkDialogView_->setSceneRect(0,0,rSize.width(), rSize.height()*5);
    linkDialogView_->fitInView(linkDialogView_->rect());
    linkDialogView_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    src_ = src;
    dest_ = dest;
    linkDialogScene_->initScene(src_, dest_);
}

int LinkDialog::exec() {
    eventLoop_.exit();
    show();
    //connect(this, SIGNAL(destroy()), &eventLoop_, SLOT(quit()));
    return eventLoop_.exec();
}

QSize LinkDialog::sizeHint() const {
    QSize size = layout()->sizeHint();
    size.setHeight(linkDialogHeight);
    return size;
}

//////////////////////////////////////////////////////////////////////////

CheckableQComboBox::CheckableQComboBox(QWidget *parent , std::string widgetName, std::vector<std::string> options) : QComboBox(parent),widgetName_(widgetName) {
    setEditable(true);
    lineEdit()->setReadOnly(true);
    stdandardModel_ = new QStandardItemModel(static_cast<int>(options.size()),1);

    for (size_t i=0; i<options.size(); i++) {
        QStandardItem* item = new QStandardItem(QString(options[i].c_str()));
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        item->setData(Qt::Checked, Qt::CheckStateRole);
        stdandardModel_->setItem(static_cast<int>(i), 0, item);
        standardItems_.push_back(item);
    }

    setModel(stdandardModel_);
    lineEdit()->setText(QString(widgetName_.c_str()));
    connect(stdandardModel_, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(onSmartLinkOptionChecked(const QModelIndex&,
            const QModelIndex&)));
}

CheckableQComboBox::~CheckableQComboBox(){
    delete stdandardModel_;
}

bool CheckableQComboBox::isItemChecked(int i) {
    if (i>(int)standardItems_.size())
        return false;

    QStandardItem* item = standardItems_[i];

    if (item->checkState() == Qt::Checked)
        return true;

    return false;
}

std::vector<std::string> CheckableQComboBox::getCheckedItems() {
    std::vector<std::string> checkedItemString;

    for (size_t i=0; i<standardItems_.size(); i++)
        if (isItemChecked(static_cast<int>(i)))
            checkedItemString.push_back(standardItems_[i]->text().toLocal8Bit().constData());

    return checkedItemString;
}

void CheckableQComboBox::onSmartLinkOptionChecked(const QModelIndex& tl, const QModelIndex& br) {
    if (isItemChecked(tl.row())) {
        //do some maintenance stuff here if required
    }

    lineEdit()->setText(QString(widgetName_.c_str()));
}

} //namespace