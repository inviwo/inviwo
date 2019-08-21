/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <modules/qtwidgets/sliderwidgetqt.h>
#include <modules/qtwidgets/numberlineedit.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <limits>
#include <cmath>

#include <warn/push>
#include <warn/ignore/all>
#include <QStyle>
#include <QSlider>
#include <QHBoxLayout>
#include <QSignalBlocker>
#include <QMouseEvent>
#include <warn/pop>

namespace inviwo {

namespace {

class Slider : public QSlider {
    using QSlider::QSlider;

    virtual void wheelEvent(QWheelEvent *e) override {
        if (hasFocus()) QSlider::wheelEvent(e);
    }
};

}  // namespace

BaseSliderWidgetQt::BaseSliderWidgetQt(bool intMode)
    : QWidget()
    , spinBox_(new NumberLineEdit(intMode))
    , slider_(new Slider())
    , spinnerValue_(0.0)
    , sliderValue_(0) {

    QHBoxLayout *hLayout = new QHBoxLayout();

    slider_->setOrientation(Qt::Horizontal);
    slider_->setPageStep(1);
    slider_->setMaximum(sliderMax_);
    slider_->installEventFilter(this);
    slider_->setFocusPolicy(Qt::ClickFocus);

    setFocusPolicy(spinBox_->focusPolicy());
    setFocusProxy(spinBox_);
    slider_->setFocusProxy(spinBox_);

    spinBox_->setKeyboardTracking(false);  // don't emit the valueChanged() signal while typing

    hLayout->addWidget(slider_);
    hLayout->addWidget(spinBox_);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(utilqt::refSpacePx(this));
    hLayout->setStretch(0, 3);
    hLayout->setStretch(1, 1);

    setLayout(hLayout);
    connect(slider_, &QSlider::valueChanged, this, &BaseSliderWidgetQt::updateFromSlider);
    connect(spinBox_, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &BaseSliderWidgetQt::updateFromSpinBox);

    QSizePolicy sp = sizePolicy();
    sp.setVerticalPolicy(QSizePolicy::Fixed);
    setSizePolicy(sp);
}

void BaseSliderWidgetQt::setWrapping(bool wrap) { spinBox_->setWrapping(wrap); }

bool BaseSliderWidgetQt::wrapping() const { return spinBox_->wrapping(); }

void BaseSliderWidgetQt::applyInit() {
    updateSlider();
    updateSpinBox();
}

void BaseSliderWidgetQt::applyValue() {
    applyInit();
    emit valueChanged();
}
void BaseSliderWidgetQt::applyMinValue() {
    QSignalBlocker spinBlock(spinBox_);
    QSignalBlocker slideBlock(slider_);

    spinBox_->setMinimum(transformMinValueToSpinner());
    slider_->setMinimum(transformMinValueToSlider());
    updateSlider();
}
void BaseSliderWidgetQt::applyMaxValue() {
    QSignalBlocker spinBlock(spinBox_);
    QSignalBlocker slideBlock(slider_);

    spinBox_->setMaximum(transformMaxValueToSpinner());
    slider_->setMaximum(transformMaxValueToSlider());
    updateSlider();
}
void BaseSliderWidgetQt::applyIncrement() {
    QSignalBlocker spinBlock(spinBox_);
    QSignalBlocker slideBlock(slider_);

    spinBox_->setSingleStep(transformIncrementToSpinner());
    spinBox_->setDecimals(transformIncrementToSpinnerDecimals());
    slider_->setSingleStep(transformIncrementToSlider());
}

void BaseSliderWidgetQt::updateFromSlider() {
    int newValue = slider_->value();
    if (newValue != sliderValue_) {
        sliderValue_ = newValue;
        newSliderValue(sliderValue_);
        updateSpinBox();
        emit valueChanged();
    }
}

void BaseSliderWidgetQt::updateFromSpinBox() {
    double newValue = spinBox_->value();
    if (fabs(newValue - spinnerValue_) > std::numeric_limits<double>::epsilon()) {
        spinnerValue_ = newValue;
        newSpinnerValue(spinnerValue_);
        updateSlider();
        emit valueChanged();
    }
}

void BaseSliderWidgetQt::updateSpinBox() {
    QSignalBlocker spinBlock(spinBox_);

    spinnerValue_ = transformValueToSpinner();
    spinBox_->setValue(spinnerValue_);
}

void BaseSliderWidgetQt::updateSlider() {
    QSignalBlocker slideBlock(slider_);

    sliderValue_ = transformValueToSlider();
    bool isOutOfBounds = (slider_->maximum() < sliderValue_ || slider_->minimum() > sliderValue_);
    if (isOutOfBounds != slider_->property("outOfBounds").toBool()) {
        slider_->setProperty("outOfBounds", isOutOfBounds);
        slider_->style()->unpolish(slider_);
        slider_->style()->polish(slider_);
    }

    slider_->setValue(sliderValue_);
}

bool BaseSliderWidgetQt::eventFilter(QObject *watched, QEvent *event) {
    if (watched == slider_ && event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton && !slider_->isSliderDown() && isEnabled()) {
            auto newPos = slider_->minimum() + static_cast<double>(me->pos().x()) *
                                                   (slider_->maximum() - slider_->minimum()) /
                                                   slider_->width();
            slider_->setValue(static_cast<int>(newPos));
            me->accept();
            return true;
        }
    }

    return QObject::eventFilter(watched, event);
}

}  // namespace inviwo
