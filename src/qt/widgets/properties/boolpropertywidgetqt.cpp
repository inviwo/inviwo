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

#include <inviwo/qt/widgets/properties/boolpropertywidgetqt.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QCheckBox>
#include <QLineEdit>
#include <QRegExp>
#include <QRegExpValidator>
#include <warn/pop>


namespace inviwo {

BoolPropertyWidgetQt::BoolPropertyWidgetQt(BoolProperty* property) 
    : PropertyWidgetQt(property)
    , property_(property) {

    generateWidget();
    updateFromProperty();
}

void BoolPropertyWidgetQt::generateWidget() {
    label_ = new EditableLabelQt(this, property_, false);

    lineEdit_ = new QLineEdit();
    // adjust size policy of line edit
    QSizePolicy sizePolicy(lineEdit_->sizePolicy());
    sizePolicy.setHorizontalStretch(3);
    sizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    lineEdit_->setSizePolicy(sizePolicy);

    lineEdit_->setEnabled(!property_->getReadOnly());
    // set up a validator accepting "true"/1 and "false"/0
    lineEdit_->setValidator(new QRegExpValidator(QRegExp("true|false|1|0"), lineEdit_));

    auto setPropertyValueFromString = [=]() {
        QString str(lineEdit_->text());
        property_->set(str == "true" || str == "1");
        // update checkbox
        checkBox_->setChecked(property_->get());
    };
    connect(lineEdit_, &QLineEdit::editingFinished, setPropertyValueFromString);

    checkBox_ = new QCheckBox();
    checkBox_->setEnabled(!property_->getReadOnly());
    checkBox_->setFixedSize(QSize(15, 15));

    auto setPropertyValueFromCheckbox = [=](bool checked) {
        property_->set(checked);
        // update text representation in line edit
        lineEdit_->setText(checked ? "true" : "false");
    };
    connect(checkBox_, &QCheckBox::toggled, setPropertyValueFromCheckbox);

    // create widget layout 
    QHBoxLayout* hLayout = new QHBoxLayout();
    setSpacingAndMargins(hLayout);
    hLayout->addWidget(label_);
    hLayout->addWidget(checkBox_);
    hLayout->addWidget(lineEdit_);
    setLayout(hLayout);

    // change visibility of checkbox and line edit depending on semantics
    bool textSemantics = (property_->getSemantics() == PropertySemantics::Text);
    lineEdit_->setVisible(textSemantics);
    checkBox_->setVisible(!textSemantics);
}

void BoolPropertyWidgetQt::updateFromProperty() {
    checkBox_->setChecked(property_->get());
    lineEdit_->setText(property_->get() ? "true" : "false");
}

} // namespace inviwo
