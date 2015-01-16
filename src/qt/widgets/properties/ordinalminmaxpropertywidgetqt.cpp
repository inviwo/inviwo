/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#include <inviwo/qt/widgets/properties/ordinalminmaxpropertywidgetqt.h>

namespace inviwo {

BaseOrdinalMinMaxPropertyWidgetQt::BaseOrdinalMinMaxPropertyWidgetQt(Property* property)
    : PropertyWidgetQt(property) {
}
    
BaseOrdinalMinMaxPropertyWidgetQt::~BaseOrdinalMinMaxPropertyWidgetQt(){
}

void BaseOrdinalMinMaxPropertyWidgetQt::generateWidget() {
    QHBoxLayout* hLayout = new QHBoxLayout();
    setSpacingAndMargins(hLayout);
    label_ = new EditableLabelQt(this, property_->getDisplayName());
    hLayout->addWidget(label_);
    
    QHBoxLayout* hSliderLayout = new QHBoxLayout();
    QWidget* sliderWidget = new QWidget();
    sliderWidget->setLayout(hSliderLayout);
    hSliderLayout->setContentsMargins(0,0,0,0);
    
    spinBoxMin_ = new CustomDoubleSpinBoxQt(this);
    spinBoxMin_->setKeyboardTracking(false); // don't emit the valueChanged() signal while typing
    spinBoxMin_->setFixedWidth(50);
    hSliderLayout->addWidget(spinBoxMin_);
    
    slider_ = new RangeSliderQt(Qt::Horizontal, this);
    hSliderLayout->addWidget(slider_);
    
    spinBoxMax_ = new CustomDoubleSpinBoxQt(this);
    spinBoxMax_->setKeyboardTracking(false); // don't emit the valueChanged() signal while typing
    spinBoxMax_->setFixedWidth(50);
    hSliderLayout->addWidget(spinBoxMax_);
    
    hLayout->addWidget(sliderWidget);
    setLayout(hLayout);
    
    QSizePolicy slidersPol = sliderWidget->sizePolicy();
    slidersPol.setHorizontalStretch(3);
    sliderWidget->setSizePolicy(slidersPol);
    
    connect(label_, SIGNAL(textChanged()),this, SLOT(setPropertyDisplayName()));
    
    connect(slider_, SIGNAL(valuesChanged(int,int)), this, SLOT(updateFromSlider(int,int)));
    connect(spinBoxMin_, SIGNAL(valueChanged(double)), this, SLOT(updateFromSpinBoxMin(double)));
    connect(spinBoxMax_, SIGNAL(valueChanged(double)), this, SLOT(updateFromSpinBoxMax(double)));
}

void BaseOrdinalMinMaxPropertyWidgetQt::setPropertyDisplayName() {
    this->property_->setDisplayName(label_->getText());
}


} // namespace

