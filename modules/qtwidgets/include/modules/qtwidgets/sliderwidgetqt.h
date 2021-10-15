/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2021 Inviwo Foundation
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

#pragma once

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/ordinalbasewidget.h>
#include <inviwo/core/properties/constraintbehavior.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QWidget>
#include <QLocale>
#include <warn/pop>

class QSlider;

namespace inviwo {

class NumberLineEdit;

class IVW_MODULE_QTWIDGETS_API BaseSliderWidgetQt : public QWidget {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    BaseSliderWidgetQt(bool intMode = false);
    virtual ~BaseSliderWidgetQt() = default;

    void setWrapping(bool wrap);
    bool wrapping() const;

protected:
    virtual double transformValueToSpinner() = 0;
    virtual double transformMinValueToSpinner() = 0;
    virtual double transformMaxValueToSpinner() = 0;
    virtual double transformIncrementToSpinner() = 0;

    virtual int transformValueToSlider() = 0;
    virtual int transformMinValueToSlider() = 0;
    virtual int transformMaxValueToSlider() = 0;
    virtual int transformIncrementToSlider() = 0;

    virtual int transformIncrementToSpinnerDecimals() = 0;

    virtual void newSliderValue(int val) = 0;
    virtual void newSpinnerValue(double val) = 0;

    void applyInit();
    void applyValue();
    void applyMinValue();
    void applyMaxValue();
    void applyIncrement();

    static constexpr int sliderMax_ = 10000;

signals:
    void valueChanged();

private:
    void updateFromSlider();
    void updateFromSpinBox();

    /**
     * \brief updates the value of the spin box from the slider value
     */
    void updateSpinBox();
    /**
     * \brief updates the value of the slider from the spin box value
     */
    void updateSlider();

    void updateOutOfBounds();

    virtual bool eventFilter(QObject* watched, QEvent* event) override;

    NumberLineEdit* spinBox_;
    QSlider* slider_;
    double spinnerValue_;
    int sliderValue_;

protected:
    ConstraintBehavior minCB_;
    ConstraintBehavior maxCB_;
};

template <typename T>
class SliderWidgetQt final : public BaseSliderWidgetQt, public OrdinalBaseWidget<T> {
public:
    SliderWidgetQt()
        : BaseSliderWidgetQt(!std::is_floating_point_v<T>)
        , value_(0)
        , minValue_(0)
        , maxValue_(0)
        , increment_(0) {}
    virtual ~SliderWidgetQt() = default;

    // Implements OrdinalBaseWidget<T>
    virtual T getValue() const override;
    virtual void setValue(T value) override;
    virtual void initValue(T value) override;
    virtual void setMinValue(T minValue, ConstraintBehavior cb) override;
    virtual void setMaxValue(T maxValue, ConstraintBehavior cb) override;
    virtual void setIncrement(T increment) override;

protected:
    // Define the transforms
    T sliderToRepr(int val) const;
    int reprToSlider(T val) const;
    T spinnerToRepr(double val) const;
    double reprToSpinner(T val) const;

    virtual double transformValueToSpinner() override;
    virtual double transformMinValueToSpinner() override;
    virtual double transformMaxValueToSpinner() override;
    virtual double transformIncrementToSpinner() override;

    virtual int transformValueToSlider() override;
    virtual int transformMinValueToSlider() override;
    virtual int transformMaxValueToSlider() override;
    virtual int transformIncrementToSlider() override;

    virtual int transformIncrementToSpinnerDecimals() override;

    virtual void newSliderValue(int val) override;
    virtual void newSpinnerValue(double val) override;

    T value_;
    T minValue_;
    T maxValue_;
    T increment_;
};

using IntSliderWidgetQt = SliderWidgetQt<int>;
using FloatSliderWidgetQt = SliderWidgetQt<float>;
using DoubleSliderWidgetQt = SliderWidgetQt<double>;

template <typename T>
T SliderWidgetQt<T>::spinnerToRepr(double val) const {
    return static_cast<T>(val);
}
template <typename T>
double SliderWidgetQt<T>::reprToSpinner(T val) const {
    return static_cast<double>(val);
}
template <typename T>
double SliderWidgetQt<T>::transformValueToSpinner() {
    return reprToSpinner(value_);
}
template <typename T>
double SliderWidgetQt<T>::transformMinValueToSpinner() {
    return reprToSpinner(minValue_);
}
template <typename T>
double SliderWidgetQt<T>::transformMaxValueToSpinner() {
    return reprToSpinner(maxValue_);
}
template <typename T>
double SliderWidgetQt<T>::transformIncrementToSpinner() {
    return reprToSpinner(increment_);
}

template <typename T>
T SliderWidgetQt<T>::sliderToRepr(int val) const {
    if constexpr (std::is_floating_point_v<T>) {
        return this->minValue_ + (static_cast<T>(val) * (this->maxValue_ - this->minValue_) /
                                  static_cast<T>(this->sliderMax_));

    } else {
        return static_cast<T>(val);
    }
}
template <typename T>
int SliderWidgetQt<T>::reprToSlider(T val) const {
    if constexpr (std::is_floating_point_v<T>) {
        if (this->maxValue_ == this->minValue_) return this->sliderMax_ / 2;
        return static_cast<int>((val - this->minValue_) / (this->maxValue_ - this->minValue_) *
                                this->sliderMax_);
    } else {
        return static_cast<int>(val);
    }
}
template <typename T>
int SliderWidgetQt<T>::transformValueToSlider() {
    return reprToSlider(value_);
}
template <typename T>
int SliderWidgetQt<T>::transformMinValueToSlider() {
    if constexpr (std::is_floating_point_v<T>) {
        return 0;
    } else {
        return reprToSlider(minValue_);
    }
}
template <typename T>
int SliderWidgetQt<T>::transformMaxValueToSlider() {
    if constexpr (std::is_floating_point_v<T>) {
        return this->sliderMax_;
    } else {
        return reprToSlider(maxValue_);
    }
}
template <typename T>
int SliderWidgetQt<T>::transformIncrementToSlider() {
    return reprToSlider(increment_);
}

template <typename T>
int SliderWidgetQt<T>::transformIncrementToSpinnerDecimals() {
    return utilqt::decimals<T>(reprToSpinner(increment_));
}

template <typename T>
void SliderWidgetQt<T>::newSpinnerValue(double val) {
    value_ = spinnerToRepr(val);
}

template <typename T>
void SliderWidgetQt<T>::newSliderValue(int val) {
    value_ = sliderToRepr(val);
}

template <typename T>
T SliderWidgetQt<T>::getValue() const {
    return value_;
}

template <typename T>
void SliderWidgetQt<T>::setValue(T value) {
    if (value != value_) {
        value_ = value;
        applyValue();
    }
}

template <typename T>
void SliderWidgetQt<T>::initValue(T value) {
    value_ = value;
    applyInit();
}

template <typename T>
void SliderWidgetQt<T>::setMinValue(T minValue, ConstraintBehavior cb) {
    if (minValue_ != minValue || minCB_ != cb) {
        minValue_ = minValue;
        minCB_ = cb;
        applyMinValue();
    }
}

template <typename T>
void SliderWidgetQt<T>::setMaxValue(T maxValue, ConstraintBehavior cb) {
    if (maxValue_ != maxValue || maxCB_ != cb) {
        maxValue_ = maxValue;
        maxCB_ = cb;
        applyMaxValue();
    }
}

template <typename T>
void SliderWidgetQt<T>::setIncrement(T increment) {
    if (increment_ != increment) {
        increment_ = increment;
        applyIncrement();
    }
}

}  // namespace inviwo
