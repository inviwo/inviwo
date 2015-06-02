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

#include <QSlider>
#include <QHBoxLayout>
#include <math.h>
#include <sstream>
#include <QLocale>

namespace inviwo {

class IVW_QTWIDGETS_API BaseSliderWidgetQt : public QWidget {
    Q_OBJECT
public:
    BaseSliderWidgetQt();
    virtual ~BaseSliderWidgetQt();

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

    static const int SLIDER_MAX = 10000;
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

    virtual T getValue();
    virtual void setValue(T value);
    virtual void initValue(T value);
    virtual void setMinValue(T minValue);
    virtual void setMaxValue(T maxValue);
    virtual void setRange(T minValue, T maxValue);
    virtual void setIncrement(T increment);

protected:
    // Define the transforms
    virtual T sliderToRepr(int val) = 0;
    virtual int reprToSlider(T val) = 0;
    virtual T spinnerToRepr(double val) = 0;
    virtual double reprToSpinner(T val) = 0;

    // Has default implementations using above transformations.
    virtual double transformValueToSpinner();
    virtual int transformValueToSlider();

    virtual double transformMinValueToSpinner();
    virtual int transformMinValueToSlider();

    virtual double transformMaxValueToSpinner();
    virtual int transformMaxValueToSlider();

    virtual double transformIncrementToSpinner();
    virtual int transformIncrementToSpinnerDecimals();
    virtual int transformIncrementToSlider();

    virtual void newSliderValue(int val);
    virtual void newSpinnerValue(double val);

    T value_;
    T minValue_;
    T maxValue_;
    T increment_;
};

// Default case for fractional numbers
template <typename T>
class SliderWidgetQt : public TemplateSliderWidget<T> {
public:
    SliderWidgetQt() : TemplateSliderWidget<T>() {}
    virtual ~SliderWidgetQt() {}

protected:  
    // Defines the transform
    virtual T sliderToRepr(int val) {
        return this->minValue_ + (static_cast<T>(val) * (this->maxValue_ - this->minValue_) / static_cast<T>(SLIDER_MAX));
    }
    virtual int reprToSlider(T val) {
        return static_cast<int>((val - this->minValue_) / (this->maxValue_ - this->minValue_) * SLIDER_MAX);
    } 
    virtual T spinnerToRepr(double val) {
        return static_cast<T>(val);
    }
    virtual double reprToSpinner(T val) {
        return static_cast<double>(val);
    }

    // Custom min and max for the slider
    virtual int transformMinValueToSlider() {
        return 0;
    }
    virtual int transformMaxValueToSlider() {
        return SLIDER_MAX;
    };

private:
    static const int SLIDER_MAX = 10000;
};

// Specialization for integer types
template <>
class SliderWidgetQt<int> : public TemplateSliderWidget<int>{
public:
    SliderWidgetQt() : TemplateSliderWidget<int>() {}
    virtual ~SliderWidgetQt() {}

protected:
    // Defines the transform
    virtual int sliderToRepr(int val) {
        return val;
    }
    virtual int reprToSlider(int val) {
        return val;
    }
    virtual int spinnerToRepr(double val) {
        return static_cast<int>(val);
    }
    virtual double reprToSpinner(int val) {
        return static_cast<double>(val);
    }
    virtual int transformIncrementToSpinnerDecimals() {
        return 0;
    }
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

