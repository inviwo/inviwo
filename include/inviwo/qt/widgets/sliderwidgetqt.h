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

#ifndef IVW_SLIDETWIDGETQT_H
#define IVW_SLIDETWIDGETQT_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <inviwo/qt/widgets/inviwoqtutils.h>
#include <inviwo/qt/widgets/customdoublespinboxqt.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QSlider>
#include <QHBoxLayout>
#include <QLocale>
#include <warn/pop>

#include <math.h>
#include <sstream>

namespace inviwo {

class IVW_QTWIDGETS_API BaseSliderWidgetQt : public QWidget {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    BaseSliderWidgetQt();
    virtual ~BaseSliderWidgetQt() = default;

protected: 
    virtual double transformValueToSpinner() = 0;
    virtual int transformValueToSlider() = 0;

    virtual double transformMinValueToSpinner() = 0;
    virtual int transformMinValueToSlider() = 0;

    virtual double transformMaxValueToSpinner() = 0;
    virtual int transformMaxValueToSlider() = 0;

    virtual double transformIncrementToSpinner() = 0;
    virtual int transformIncrementToSpinnerDecimals() = 0;
    virtual int transformIncrementToSlider() = 0;

    virtual void newSliderValue(int val) = 0;
    virtual void newSpinnerValue(double val) = 0;

    void applyInit();
    void applyValue();
    void applyMinValue();
    void applyMaxValue();
    void applyIncrement();

private slots:
    void updateFromSlider();
    void updateFromSpinBox();

signals:
    void valueChanged();

private:
    void generateWidget();
    /**
    * \brief updates the value of the spin box from the slider value
    */
    void updateSpinBox();
    /**
    * \brief updates the value of the slider from the spin box value
    */
    void updateSlider();

    static const int sliderMax_ = 10000;
    CustomDoubleSpinBoxQt* spinBox_;
    QSlider* slider_;
    double spinnerValue_;
    int sliderValue_;

};

template <typename T>
class OrdinalBaseWidget {
public:
    virtual ~OrdinalBaseWidget() {}

    virtual T getValue() = 0;
    virtual void setValue(T value) = 0;
    virtual void initValue(T value) = 0;
    virtual void setMinValue(T minValue) = 0;
    virtual void setMaxValue(T maxValue) = 0;
    virtual void setRange(T minValue, T maxValue) = 0;
    virtual void setIncrement(T increment) = 0;
};

template <typename T>
class TemplateSliderWidget : public BaseSliderWidgetQt, public OrdinalBaseWidget<T> {
public:
    TemplateSliderWidget() : BaseSliderWidgetQt()
        , value_(0)
        , minValue_(0)
        , maxValue_(0)
        , increment_(0) {}
    virtual ~TemplateSliderWidget() {}

    virtual T getValue() override;
    virtual void setValue(T value) override;
    virtual void initValue(T value) override;
    virtual void setMinValue(T minValue) override;
    virtual void setMaxValue(T maxValue) override;
    virtual void setRange(T minValue, T maxValue) override;
    virtual void setIncrement(T increment) override;

protected:
    // Define the transforms
    virtual T sliderToRepr(int val) = 0;
    virtual int reprToSlider(T val) = 0;
    virtual T spinnerToRepr(double val) = 0;
    virtual double reprToSpinner(T val) = 0;

    // Has default implementations using above transformations.
    virtual double transformValueToSpinner() override;
    virtual int transformValueToSlider() override;

    virtual double transformMinValueToSpinner() override;
    virtual int transformMinValueToSlider() override;

    virtual double transformMaxValueToSpinner() override;
    virtual int transformMaxValueToSlider() override;

    virtual double transformIncrementToSpinner() override;
    virtual int transformIncrementToSpinnerDecimals() override;
    virtual int transformIncrementToSlider() override;

    virtual void newSliderValue(int val) override;
    virtual void newSpinnerValue(double val) override;

    T value_;
    T minValue_;
    T maxValue_;
    T increment_;
};

template <typename T, bool floatingPoint = std::is_floating_point<T>::value>
class SliderWidgetQt {};

// For fractional numbers
template <typename T>
class SliderWidgetQt<T, true> : public TemplateSliderWidget<T> {
public:
    SliderWidgetQt() = default;
    virtual ~SliderWidgetQt() = default;

protected:
    // Defines the transform
    virtual T sliderToRepr(int val) override {
        return this->minValue_ + (static_cast<T>(val) * (this->maxValue_ - this->minValue_) /
                                  static_cast<T>(sliderMax_));
    }
    virtual int reprToSlider(T val) override {
        return static_cast<int>((val - this->minValue_) / (this->maxValue_ - this->minValue_) *
                                sliderMax_);
    }
    virtual T spinnerToRepr(double val) override { return static_cast<T>(val); }
    virtual double reprToSpinner(T val) override { return static_cast<double>(val); }

    // Custom min and max for the slider
    virtual int transformMinValueToSlider() override { return 0; }
    virtual int transformMaxValueToSlider() override { return sliderMax_; };

private:
    static const int sliderMax_ = 10000;
};

// For integer types
template <typename T>
class SliderWidgetQt<T, false> : public TemplateSliderWidget<T> {
public:
    SliderWidgetQt() = default;
    virtual ~SliderWidgetQt() = default;

protected:
    // Defines the transform
    virtual T sliderToRepr(int val) override { return static_cast<T>(val); }
    virtual int reprToSlider(T val) override { return static_cast<int>(val); }
    virtual T spinnerToRepr(double val) override { return static_cast<T>(val); }
    virtual double reprToSpinner(T val) override { return static_cast<double>(val); }
    virtual int transformIncrementToSpinnerDecimals() override { return 0; }
};

typedef SliderWidgetQt<int> IntSliderWidgetQt;
typedef SliderWidgetQt<float> FloatSliderWidgetQt;
typedef SliderWidgetQt<double> DoubleSliderWidgetQt;

template <typename T>
double inviwo::TemplateSliderWidget<T>::transformValueToSpinner() {
    return reprToSpinner(value_);
}

template <typename T>
int inviwo::TemplateSliderWidget<T>::transformValueToSlider() {
    return reprToSlider(value_);
}

template <typename T>
double inviwo::TemplateSliderWidget<T>::transformMinValueToSpinner() {
    return reprToSpinner(minValue_);
}

template <typename T>
int inviwo::TemplateSliderWidget<T>::transformMinValueToSlider() {
    return reprToSlider(minValue_);
}

template <typename T>
double inviwo::TemplateSliderWidget<T>::transformMaxValueToSpinner() {
    return reprToSpinner(maxValue_);
}

template <typename T>
int inviwo::TemplateSliderWidget<T>::transformMaxValueToSlider() {
    return reprToSlider(maxValue_);
}

template <typename T>
double inviwo::TemplateSliderWidget<T>::transformIncrementToSpinner() {
    return reprToSpinner(increment_);
}

template <typename T>
int inviwo::TemplateSliderWidget<T>::transformIncrementToSpinnerDecimals() {
    const static QLocale locale;
    double inc = reprToSpinner(increment_);
    std::ostringstream buff;
    utilqt::localizeStream(buff);
    buff << inc;
    const std::string str(buff.str());
    auto periodPosition = str.find(locale.decimalPoint().toLatin1());
    if (periodPosition == std::string::npos)
        return 0;
    else
        return static_cast<int>(str.length() - periodPosition) - 1;
}

template <typename T>
int inviwo::TemplateSliderWidget<T>::transformIncrementToSlider() {
    return reprToSlider(increment_);
}

template <typename T>
void inviwo::TemplateSliderWidget<T>::newSpinnerValue(double val) {
    value_ = spinnerToRepr(val);
}

template <typename T>
void inviwo::TemplateSliderWidget<T>::newSliderValue(int val) {
    value_ = sliderToRepr(val);
}

template <typename T>
T TemplateSliderWidget<T>::getValue() {
    return value_;
}

template <typename T>
void TemplateSliderWidget<T>::setValue(T value) {
    if(value >= minValue_ && value <= maxValue_ && value != value_) {
        value_ = value;
        applyValue();
    }
}

template <typename T>
void TemplateSliderWidget<T>::initValue(T value) {
    value_ = value;
    applyInit();
}

template <typename T>
void TemplateSliderWidget<T>::setMinValue(T minValue) {
    if(minValue_ != minValue) {
        minValue_ = minValue;
        applyMinValue();
    }
}

template <typename T>
void TemplateSliderWidget<T>::setMaxValue(T maxValue) {
    if(maxValue_ != maxValue) {
        maxValue_ = maxValue;
        applyMaxValue();
    }
}

template <typename T>
void TemplateSliderWidget<T>::setRange(T minValue, T maxValue) {
    setMinValue(minValue);
    setMaxValue(maxValue);
}

template <typename T>
void TemplateSliderWidget<T>::setIncrement(T increment) {
    if(increment_ != increment) {
        increment_ = increment;
        applyIncrement();
    }
}




} // namespace

#endif // IVW_SLIDETWIDGETQT_H

