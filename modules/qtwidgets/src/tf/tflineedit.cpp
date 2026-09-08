/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2026 Inviwo Foundation
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

#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/glmvec.h>
#include <modules/qtwidgets/properties/doublevaluedragspinbox.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <QSignalBlocker>
#include <QSizePolicy>
#include <QHBoxLayout>

#include <glm/vec2.hpp>

class QLayout;

namespace inviwo {

TFLineEdit::TFLineEdit(QWidget* parent) : QWidget(parent) {
    numberWidget_.setClearable(true);
    numberWidget_.setPlaceholder("<->");
    numberWidget_.initValueOptional(std::nullopt);
    numberWidget_.setEnabled(false);

    connect(&numberWidget_, &BaseNumberWidget::valueChanged, this, [this]() {
        if (auto v = value()) {
            emit valueChanged(*v);
        }
    });

    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred));

    // "steal" layout from spinbox and add it to this widget instead
    auto* layout = new QHBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(&numberWidget_);
    setLayout(layout);
}

QSize TFLineEdit::sizeHint() const { return {18, 18}; }

void TFLineEdit::setValidRange(const dvec2& range, double inc) {
    const QSignalBlocker block{numberWidget_};
    validRange_ = range;
    numberWidget_.setMinValue(range.x, ConstraintBehavior::Immutable);
    numberWidget_.setMaxValue(range.y, ConstraintBehavior::Immutable);

    numberWidget_.setIncrement(inc);
}

dvec2 TFLineEdit::getValidRange() const { return validRange_; }

void TFLineEdit::setValueMapping(bool enable, const dvec2& range, double inc) {
    const QSignalBlocker block{numberWidget_};

    const auto prevValue = value();

    valueMappingEnabled_ = enable;
    valueRange_ = range;
    if (valueMappingEnabled_) {
        setValidRange(range, inc);
    } else {
        setValidRange(
            dvec2{std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max()}, inc);
    }

    // update text
    setValue(prevValue, numberWidget_.isEnabled() && !prevValue.has_value());
}

void TFLineEdit::setValue(std::optional<double> value, bool ambiguous) {
    const QSignalBlocker block{numberWidget_};

    numberWidget_.setEnabled(value.has_value() || ambiguous);
    numberWidget_.initValueOptional(value.transform([this](double v) {
        if (valueMappingEnabled_) {
            v = v * (valueRange_.y - valueRange_.x) + valueRange_.x;
        }
        return v;
    }));
}

std::optional<double> TFLineEdit::value() const {
    auto value = numberWidget_.getValueOptional();
    if (valueMappingEnabled_ && value) {
        // renormalize value to [0,1]
        value = (*value - valueRange_.x) / (valueRange_.y - valueRange_.x);
    }
    return value;
}

}  // namespace inviwo
