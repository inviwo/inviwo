/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2025 Inviwo Foundation
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

#include <modules/qtwidgets/properties/valuedragger.h>

#include <modules/qtwidgets/inviwoqtutils.h>               // for emToPx
#include <modules/qtwidgets/numberlineedit.h>              // for NumberLineEdit
#include <modules/qtwidgets/properties/indicatorwidget.h>  // for IndicatorWidget
#include <QColor>                                          // for QColor
#include <QFlags>                                          // for QFlags
#include <QMouseEvent>                                     // for QMouseEvent
#include <QPainter>                                        // for QPainter, QPainter::Antialiasing
#include <QPalette>                                        // for QPalette, QPalette::Active
#include <QPen>                                            // for QPen
#include <QPointF>                                         // for QPointF
#include <QSizeF>                                          // for QSizeF
#include <QSizePolicy>                                     // for QSizePolicy, QSizePolicy::Fixed
#include <QString>                                         // for QString
#include <QStyle>                                          // for QStyle, QStyle::State_Enabled
#include <QStyleOption>                                    // for QStyleOption
#include <QTimerEvent>                                     // for QTimerEvent
#include <QToolTip>                                        // for QToolTip
#include <QTransform>                                      // for QTransform
#include <Qt>                                              // for LeftButton, NoBrush, FlatCap
#include <QPainterPath>                                    // for QPainterPath

#include <algorithm>    // for max, min
#include <cmath>        // for pow, remainder, abs
#include <memory>       // for unique_ptr, make_unique
#include <type_traits>  // for enable_if_t, is_integral

namespace inviwo {

ValueDragger::ValueDragger(NumberLineEdit* spinBox, QWidget* parent)
    : QWidget(parent), spinBox_(spinBox), indicator_(std::make_unique<IndicatorWidget>()) {
    indicator_->setVisible(false);
    setObjectName("valueDragger");
    setAttribute(Qt::WA_TranslucentBackground);

    QSizePolicy policy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    policy.setHorizontalStretch(0);
    policy.setVerticalStretch(0);
    setSizePolicy(policy);
}

ValueDragger::~ValueDragger() = default;

QSize ValueDragger::sizeHint() const { return utilqt::emToPx(this, QSizeF(0.8, 2)); }

void ValueDragger::setIncrement(int t) { defaultIncrement_ = t / 100.0; }

void ValueDragger::setIncrement(double t) { defaultIncrement_ = t; }

void ValueDragger::setExponent(double e) { exponent_ = e; }

void ValueDragger::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    painter.setTransform(QTransform::fromTranslate(width() / 2, height() / 2));

    QStyleOption option;
    option.initFrom(this);

    QPen pen(Qt::NoBrush, 2.0, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
    if (!(option.state & QStyle::State_Enabled)) {
        pen.setColor(option.palette.color(QPalette::Disabled, QPalette::WindowText).darker());
    } else if (option.state & QStyle::State_MouseOver) {
        pen.setColor(option.palette.color(QPalette::Active, QPalette::Highlight));
    } else {
        pen.setColor(option.palette.color(QPalette::Active, QPalette::Text));
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(pen.color());
    painter.drawEllipse(QPointF(0, 0), height() / 8.0, height() / 8.0);

    // draw arrow tips
    auto arrowTop = (height() / 2.0) * 0.8 - 2.0;
    auto arrowBase = 1.5 + (height() / 6.0) * 0.8;
    auto arrowHalfWidth = height() / 4.0 - 2.0;

    QPainterPath path;
    path.moveTo(-arrowHalfWidth, arrowBase);
    path.lineTo(0, arrowTop);
    path.lineTo(arrowHalfWidth, arrowBase);
    path.moveTo(-arrowHalfWidth, -arrowBase);
    path.lineTo(0, -arrowTop);
    path.lineTo(arrowHalfWidth, -arrowBase);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(path);
}

void ValueDragger::mousePressEvent(QMouseEvent* e) {
    if ((e->button() == Qt::LeftButton) && !dragging_) {
        reset();

        dragging_ = true;
        clickPos_ = e->pos();
        currentValue_ = spinBox_->value();

        increment_ = (defaultIncrement_ *
                      ((currentValue_ == 0.0) ? spinBox_->singleStep() : std::abs(currentValue_)));

        // reposition indicator widget at the center of this widget
        indicator_->setBasePoint(mapToGlobal(QPoint(width() / 2, height() / 2)));
        indicator_->show();
    }
    e->accept();
    QWidget::mousePressEvent(e);
}

void ValueDragger::mouseReleaseEvent(QMouseEvent* e) {
    if (dragging_ && (e->button() == Qt::LeftButton)) {
        reset();
        indicator_->hide();
        QToolTip::hideText();
        e->accept();
    }
    QWidget::mouseReleaseEvent(e);
}

void ValueDragger::mouseMoveEvent(QMouseEvent* e) {
    if (dragging_) {
        int mousePosDelta = clickPos_.y() - e->pos().y();
        int sign = (mousePosDelta > 0) ? 1 : (mousePosDelta < 0) ? -1 : 0;

        delta_ = (std::pow(std::abs(mousePosDelta + 1), exponent_) - 1.0) * sign * increment_;

        indicator_->setLength(mousePosDelta);
        if (spinDeltaTimerId_ == -1) {
            spinDeltaTimerId_ = startTimer(timerInterval_);
        }
        e->accept();
    } else {
        QWidget::mouseMoveEvent(e);
    }
}

void ValueDragger::timerEvent(QTimerEvent* e) {
    bool doStep = false;
    if (e->timerId() == spinDeltaTimerId_) {
        QToolTip::showText(mapToGlobal(clickPos_), QString("Δ = %1 / s").arg(delta_));
        doStep = ((delta_ != 0.0) && (currentValue_ >= spinBox_->minimum()) &&
                  (currentValue_ <= spinBox_->maximum()));
    }

    if (doStep) {
        currentValue_ += delta_ * timerInterval_ / 1000;
        if (spinBox_->wrapping()) {
            auto remainder = std::remainder(currentValue_ - spinBox_->minimum(),
                                            spinBox_->maximum() - spinBox_->minimum());
            if (remainder < 0.0) {
                remainder += spinBox_->maximum() - spinBox_->minimum();
            }
            currentValue_ = remainder + spinBox_->minimum();
        }

        currentValue_ = std::clamp<double>(currentValue_, spinBox_->minimum(), spinBox_->maximum());

        setValue(currentValue_);
    }
    QWidget::timerEvent(e);
}

void ValueDragger::reset() {
    if (spinDeltaTimerId_ != -1) {
        killTimer(spinDeltaTimerId_);
    }
    spinDeltaTimerId_ = -1;
    delta_ = 0.0;
    dragging_ = false;
}

void ValueDragger::setValue(double val) {
    spinBox_->setValue(val);
    emit spinBox_->editingFinished();
}

}  // namespace inviwo
