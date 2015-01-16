/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#include <inviwo/qt/widgets/spinboxrangesliderwidgetqt.h>

namespace inviwo {

SpinBoxRangeSliderQt::SpinBoxRangeSliderQt() {
    QHBoxLayout* hLayout = new QHBoxLayout();
    slider_ = new RangeSliderQt(Qt::Horizontal, this);
    spinBoxMin_ = new QSpinBox(this);
    spinBoxMax_ = new QSpinBox(this);
    spinBoxMin_->setFixedWidth(70);
    spinBoxMax_->setFixedWidth(70);
    setMinMaxRange(0, 255);
    slider_->setMinValue(0);
    slider_->setMaxValue(255);
    spinBoxMin_->setMaximum(100000);
    spinBoxMax_->setMaximum(100000);
    hLayout->addWidget(spinBoxMin_);
    hLayout->addWidget(slider_);
    hLayout->addWidget(spinBoxMax_);
    setLayout(hLayout);
    connect(slider_, SIGNAL(valuesChanged(int,int)), this, SLOT(updateFromSlider(int,int)));
    connect(spinBoxMin_, SIGNAL(editingFinished()), this, SLOT(updateFromSpinBoxMin()));
    connect(spinBoxMax_, SIGNAL(editingFinished()), this, SLOT(updateFromSpinBoxMax()));
    setMinValue(0);
    setMaxValue(255);
    updateFromSlider(0, 255);
}

SpinBoxRangeSliderQt::~SpinBoxRangeSliderQt() {}

void SpinBoxRangeSliderQt::updateFromSlider(int valMin, int valMax) {
    valMin = (valMin >= maxRange_) ? maxRange_ - 1 : valMin;
    valMax = (valMax >= maxRange_) ? maxRange_ : valMax;
    valMin = (valMax <= 0) ? 0 : valMin;
    valMax = (valMax <= 0) ? 1 : valMax;
    valMin = (valMin >= valMax) ? valMax - 1 : valMin;

    if (valMin != spinBoxMin_->value())
        spinBoxMin_->setValue(valMin);

    if (valMax != spinBoxMax_->value())
        spinBoxMax_->setValue(valMax);

    emit valuesChanged();
}

void SpinBoxRangeSliderQt::updateFromSpinBoxMin() {
    int val = spinBoxMin_->value();
    val = (val <= 0) ? 0 : val;
    val = (val >= maxRange_) ? maxRange_ - 1 : val;

    if (val >= slider_->maxValue())
        slider_->setMaxValue(val + 1);

    spinBoxMin_->setValue(val);
    slider_->setMinValue(val);
}

void SpinBoxRangeSliderQt::updateFromSpinBoxMax() {
    int val = spinBoxMax_->value();
    val = (val <= 1) ? 1 : val;
    val = (val >= maxRange_) ? maxRange_ : val;

    if (val <= slider_->minValue())
        slider_->setMinValue(val - 1);

    spinBoxMax_->setValue(val);
    slider_->setMaxValue(val);
}

const int SpinBoxRangeSliderQt::getMinValue() {
    return spinBoxMin_->value();
}

const int SpinBoxRangeSliderQt::getMaxValue() {
    return spinBoxMax_->value();
}

const int SpinBoxRangeSliderQt::getMinRange() {return minRange_;}

const int SpinBoxRangeSliderQt::getMaxRange() {return maxRange_;}

void SpinBoxRangeSliderQt::setMinValue(const int value) {
    slider_->setMinValue(value);
}

void SpinBoxRangeSliderQt::setMaxValue(const int value) {
    slider_->setMaxValue(value);
}

void SpinBoxRangeSliderQt::setMinRange(const int minRange) {
    minRange_ = minRange;
    spinBoxMin_->setMinimum(minRange);
    spinBoxMax_->setMinimum(minRange + 1);
    slider_->setMinRange(minRange);
}

void SpinBoxRangeSliderQt::setMaxRange(const int maxRange) {
    maxRange_ = maxRange;
    slider_->setMaxRange(maxRange);
}

void SpinBoxRangeSliderQt::setMinMaxRange(const int minRange, const int maxRange) {
    blockSignals(true);
    setMinRange(minRange);
    setMaxRange(maxRange);
    blockSignals(false);
}
}//Namespace