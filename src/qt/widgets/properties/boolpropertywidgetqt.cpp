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

#include <inviwo/qt/widgets/properties/boolpropertywidgetqt.h>

namespace inviwo {

BoolPropertyWidgetQt::BoolPropertyWidgetQt(BoolProperty* property) 
    : PropertyWidgetQt(property)
    , property_(property) {

    generateWidget();
    updateFromProperty();
}

void BoolPropertyWidgetQt::generateWidget() {
    QHBoxLayout* hLayout = new QHBoxLayout();
    setSpacingAndMargins(hLayout);

    label_ = new EditableLabelQt(this, property_->getDisplayName(), false);
    connect(label_, SIGNAL(textChanged()), this, SLOT(setPropertyDisplayName()));
    hLayout->addWidget(label_);

    checkBox_ = new QCheckBox();
    checkBox_->setEnabled(!property_->getReadOnly());
    checkBox_->setFixedSize(QSize(15,15));
   
    connect(checkBox_, SIGNAL(clicked()), this, SLOT(setPropertyValue()));

    hLayout->addWidget(checkBox_);
    setLayout(hLayout);
}

void BoolPropertyWidgetQt::setPropertyValue() {
    property_->set(checkBox_->isChecked());
}

void BoolPropertyWidgetQt::updateFromProperty() {
    checkBox_->setChecked(property_->get());
    checkBox_->setDisabled(property_->getReadOnly());
}

void BoolPropertyWidgetQt::setPropertyDisplayName() {
    property_->setDisplayName(label_->getText());
}

} // namespace
