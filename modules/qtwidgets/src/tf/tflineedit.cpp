/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/qtwidgets/tf/tflineedit.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QDoubleValidator>
#include <QLocale>
#include <QSizePolicy>
#include <QSignalBlocker>
#include <QLayout>
#include <warn/pop>

#include <sstream>
#include <limits>

namespace inviwo {

TFLineEdit::TFLineEdit(QWidget* parent) : QWidget(parent), value_(0.0), ambiguous_(true) {
    spinbox_.setInvalid(ambiguous_);

    connect(&spinbox_, &DoubleValueDragSpinBox::editingFinished, this,
            [this]() { emit valueChanged(value()); });
    connect(&spinbox_,
            static_cast<void (DoubleValueDragSpinBox::*)(double)>(
                &DoubleValueDragSpinBox::valueChanged),
            [this]() { emit valueChanged(value()); });

    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred));

    // "steal" layout from spinbox and add it to this widget instead
    QLayout* layout = spinbox_.layout();
    setLayout(layout);
}

QSize TFLineEdit::sizeHint() const { return QSize(18, 18); }

void TFLineEdit::setValidRange(const dvec2& range, double inc) {
    spinbox_.setMinimum(range.x);
    spinbox_.setMaximum(range.y);

    spinbox_.setSingleStep(inc);
    spinbox_.setDecimals(spinbox_.spinnerDecimals(inc));
}

dvec2 TFLineEdit::getValidRange() const { return dvec2{spinbox_.minimum(), spinbox_.maximum()}; }

void TFLineEdit::setValueMapping(bool enable, const dvec2& range, double inc) {
    QSignalBlocker block{spinbox_};

    valueMappingEnabled_ = enable;
    valueRange_ = range;
    setValidRange(range, inc);

    // update text
    setValue(value_, ambiguous_);
}

void TFLineEdit::setValue(double value, bool ambiguous) {
    ambiguous_ = ambiguous;
    spinbox_.setInvalid(ambiguous);

    if (!ambiguous) {
        if (valueMappingEnabled_) {
            value = value * (valueRange_.y - valueRange_.x) + valueRange_.x;
        }
        spinbox_.setValue(value);
    }
}

double TFLineEdit::value() const {
    auto value = spinbox_.value();
    if (valueMappingEnabled_) {
        // renormalize value to [0,1]
        value = (value - valueRange_.x) / (valueRange_.y - valueRange_.x);
    }

    return value;
}

}  // namespace inviwo
