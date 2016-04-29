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

#include <inviwo/qt/widgets/sliderwidgetqt.h>

#include <limits>
#include <warn/push>
#include <warn/ignore/all>
#include <QStyle>
#include <warn/pop>

namespace inviwo {

BaseSliderWidgetQt::BaseSliderWidgetQt()
    : QWidget(), spinBox_(nullptr), slider_(nullptr), spinnerValue_(0.0), sliderValue_(0) {
    generateWidget();
}

void BaseSliderWidgetQt::generateWidget() {
    QHBoxLayout* hLayout = new QHBoxLayout();
    slider_ = new QSlider();
    slider_->setOrientation(Qt::Horizontal);
    slider_->setPageStep(1);
    slider_->setMaximum(sliderMax_);
    spinBox_ = new CustomDoubleSpinBoxQt();
    spinBox_->setKeyboardTracking(false); // don't emit the valueChanged() signal while typing

    hLayout->addWidget(slider_);
    hLayout->addWidget(spinBox_);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);
    setLayout(hLayout);
    connect(slider_, SIGNAL(valueChanged(int)), this, SLOT(updateFromSlider()));
    connect(spinBox_, SIGNAL(valueChanged(double)), this, SLOT(updateFromSpinBox()));


    QSizePolicy sp = sizePolicy();
    sp.setVerticalPolicy(QSizePolicy::Fixed);
    setSizePolicy(sp);
}

void BaseSliderWidgetQt::applyInit() {
    updateSlider();
    updateSpinBox();
}

void BaseSliderWidgetQt::applyValue() {
    applyInit();
    emit valueChanged();
}
void BaseSliderWidgetQt::applyMinValue() {
    spinBox_->blockSignals(true);
    spinBox_->setMinimum(transformMinValueToSpinner());
    spinBox_->blockSignals(false);
    slider_->blockSignals(true);
    slider_->setMinimum(transformMinValueToSlider());
    slider_->blockSignals(false);
    updateSlider();
}
void BaseSliderWidgetQt::applyMaxValue() {
    spinBox_->blockSignals(true);
    spinBox_->setMaximum(transformMaxValueToSpinner());
    spinBox_->blockSignals(false);
    slider_->blockSignals(true);
    slider_->setMaximum(transformMaxValueToSlider());
    slider_->blockSignals(false);
    updateSlider();
}
void BaseSliderWidgetQt::applyIncrement() {
    spinBox_->blockSignals(true);
    spinBox_->setSingleStep(transformIncrementToSpinner());
    spinBox_->setDecimals(transformIncrementToSpinnerDecimals());
    spinBox_->blockSignals(false);
    slider_->blockSignals(true);
    slider_->setSingleStep(transformIncrementToSlider());
    slider_->blockSignals(false);
}

void BaseSliderWidgetQt::updateFromSlider() {
    int newValue = slider_->value();
    slider_->setStyleSheet(QString());
    if(newValue != sliderValue_) {
        sliderValue_ = newValue;
        newSliderValue(sliderValue_);
        updateSpinBox();
        emit valueChanged();
    }
}

void BaseSliderWidgetQt::updateFromSpinBox() {
    double newValue = spinBox_->value();
    if(fabs(newValue - spinnerValue_) > std::numeric_limits<double>::epsilon()) {
        spinnerValue_ = newValue;
        newSpinnerValue(spinnerValue_);
        updateSlider();
        emit valueChanged();
    }
}

void BaseSliderWidgetQt::updateSpinBox() {
    spinnerValue_ = transformValueToSpinner();
    spinBox_->blockSignals(true);
    spinBox_->setValue(spinnerValue_);
    spinBox_->blockSignals(false);
}

void BaseSliderWidgetQt::updateSlider() {
    sliderValue_ = transformValueToSlider();
    slider_->blockSignals(true);

    bool isOutOfBounds = (slider_->maximum() < sliderValue_ || slider_->minimum() > sliderValue_);
    if (isOutOfBounds != slider_->property("outOfBounds").toBool() ) {
        slider_->setProperty("outOfBounds", isOutOfBounds);
        slider_->style()->unpolish(slider_);
        slider_->style()->polish(slider_);
    }

    slider_->setValue(sliderValue_);

    slider_->blockSignals(false);
}

} // namespace

