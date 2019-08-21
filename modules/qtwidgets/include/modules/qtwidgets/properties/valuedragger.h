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

#ifndef IVW_VALUEDRAGGER_H
#define IVW_VALUEDRAGGER_H

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/qtwidgets/properties/indicatorwidget.h>
#include <modules/qtwidgets/numberlineedit.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QWidget>
#include <QTimerEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QHoverEvent>
#include <QPainter>
#include <QPainterPath>
#include <QToolTip>
#include <QStyleOption>
#include <warn/pop>

#include <type_traits>
#include <algorithm>

namespace inviwo {

template <typename T>
class ValueDragger : public QWidget {
public:
    explicit ValueDragger(NumberLineEdit *spinBox, QWidget *parent = nullptr);
    virtual ~ValueDragger() = default;

    virtual QSize sizeHint() const override;

    T value() const;

    void setValue(T i);
    void setIncrement(int t);     //!< in percent [0,100]
    void setIncrement(double t);  //!< percentage [0,1]
    void setExponent(double e);

protected:
    virtual void paintEvent(QPaintEvent *e) override;
    virtual void mousePressEvent(QMouseEvent *e) override;
    virtual void mouseReleaseEvent(QMouseEvent *e) override;
    virtual void mouseMoveEvent(QMouseEvent *e) override;

    virtual void timerEvent(QTimerEvent *e) override;

private:
    void reset();

    template <typename U = T, typename = std::enable_if_t<std::is_integral<U>::value>>
    void setValue(double val);

    const int timerInterval_ = 100;
    double defaultIncrement_ = 0.01;
    double exponent_ = 1.2;

    NumberLineEdit *spinBox_;
    std::unique_ptr<IndicatorWidget> indicator_;
    int spinDeltaTimerId_ = -1;
    QPoint clickPos_;
    bool dragging_ = false;
    double delta_ = 0.0;
    double currentValue_ = 0.0;

    double increment_;
};

template <typename T>
ValueDragger<T>::ValueDragger(NumberLineEdit *spinBox, QWidget *parent)
    : QWidget(parent), spinBox_(spinBox), indicator_(std::make_unique<IndicatorWidget>()) {
    indicator_->setVisible(false);
    setObjectName("valueDragger");
    setAttribute(Qt::WA_TranslucentBackground);

    QSizePolicy policy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    policy.setHorizontalStretch(0);
    policy.setVerticalStretch(0);
    setSizePolicy(policy);
}

template <typename T>
QSize ValueDragger<T>::sizeHint() const {
    return utilqt::emToPx(this, QSizeF(0.8, 2));
}

template <typename T>
void ValueDragger<T>::setValue(T i) {
    spinBox_->setValue(i);
}

template <typename T>
T ValueDragger<T>::value() const {
    return static_cast<T>(spinBox_->value());
}

template <typename T>
void ValueDragger<T>::setIncrement(int t) {
    defaultIncrement_ = t / 100.0;
}

template <typename T>
void ValueDragger<T>::setIncrement(double t) {
    defaultIncrement_ = t;
}

template <typename T>
void ValueDragger<T>::setExponent(double e) {
    exponent_ = e;
}

template <typename T>
void ValueDragger<T>::paintEvent(QPaintEvent *) {
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

template <typename T>
void ValueDragger<T>::mousePressEvent(QMouseEvent *e) {
    if ((e->button() == Qt::LeftButton) && !dragging_) {
        reset();

        dragging_ = true;
        clickPos_ = e->pos();
        currentValue_ = value();

        increment_ =
            (defaultIncrement_ * ((value() == 0) ? spinBox_->singleStep() : std::abs(value())));

        // reposition indicator widget at the center of this widget
        indicator_->setBasePoint(mapToGlobal(QPoint(width() / 2, height() / 2)));
        indicator_->show();
    }
    e->accept();
    QWidget::mousePressEvent(e);
}

template <typename T>
void ValueDragger<T>::mouseReleaseEvent(QMouseEvent *e) {
    if (dragging_ && (e->button() == Qt::LeftButton)) {
        reset();
        indicator_->hide();
        QToolTip::hideText();
        e->accept();
    }
    QWidget::mouseReleaseEvent(e);
}

template <typename T>
void ValueDragger<T>::mouseMoveEvent(QMouseEvent *e) {
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

template <typename T>
void ValueDragger<T>::timerEvent(QTimerEvent *e) {
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

        currentValue_ = std::max<double>(std::min<double>(currentValue_, spinBox_->maximum()),
                                         spinBox_->minimum());

        setValue(currentValue_);
    }
    QWidget::timerEvent(e);
}

template <typename T>
void ValueDragger<T>::reset() {
    if (spinDeltaTimerId_ != -1) {
        killTimer(spinDeltaTimerId_);
    }
    spinDeltaTimerId_ = -1;
    delta_ = 0.0;
    dragging_ = false;
}

template <typename T>
template <typename U, typename>
void ValueDragger<T>::setValue(double val) {
    spinBox_->setValue(static_cast<int>(val + 0.5));
}

}  // namespace inviwo

#endif  // IVW_VALUEDRAGGER_H
