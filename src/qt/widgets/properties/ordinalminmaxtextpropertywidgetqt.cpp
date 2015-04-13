/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <inviwo/qt/widgets/properties/ordinalminmaxtextpropertywidgetqt.h>

namespace inviwo {

BaseOrdinalMinMaxTextPropertyWidgetQt::BaseOrdinalMinMaxTextPropertyWidgetQt(Property* property)
    : PropertyWidgetQt(property) {
}
    
BaseOrdinalMinMaxTextPropertyWidgetQt::~BaseOrdinalMinMaxTextPropertyWidgetQt(){
}

void BaseOrdinalMinMaxTextPropertyWidgetQt::generateWidget() {
    makeEditorWidgets();
    
    QHBoxLayout* hLayout = new QHBoxLayout();
    setSpacingAndMargins(hLayout);
    label_ = new EditableLabelQt(this, property_->getDisplayName());
    hLayout->addWidget(label_);
    
    QHBoxLayout* textLayout = new QHBoxLayout();
    QWidget* textWidget = new QWidget();
    textWidget->setLayout(textLayout);
    textLayout->setContentsMargins(0,0,0,0);
    
    QSizePolicy sp = QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    sp.setHorizontalStretch(3);

    QLabel* minLabel = new QLabel("Min:");
    textLayout->addWidget(minLabel);
    textLayout->addWidget(min_);
    min_->setSizePolicy(sp);

    QLabel* maxLabel = new QLabel("Max:");
    textLayout->addWidget(maxLabel);
    textLayout->addWidget(max_);
    max_->setSizePolicy(sp);
    
    hLayout->addWidget(textWidget);
    setLayout(hLayout);
    
    QSizePolicy textsp = textWidget->sizePolicy();
    textsp.setHorizontalStretch(3);
    textWidget->setSizePolicy(textsp);
    
    connect(label_, SIGNAL(textChanged()),this, SLOT(setPropertyDisplayName()));
    connect(min_, SIGNAL(valueChanged()), this, SLOT(updateFromMin()));
    connect(max_, SIGNAL(valueChanged()), this, SLOT(updateFromMax()));
    
    setFixedHeight(sizeHint().height());
    sp = sizePolicy();
    sp.setVerticalPolicy(QSizePolicy::Fixed);
    setSizePolicy(sp);
}

void BaseOrdinalMinMaxTextPropertyWidgetQt::setPropertyDisplayName() {
    this->property_->setDisplayName(label_->getText());
}

} // namespace

