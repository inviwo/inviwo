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

#ifndef IVW_ORDINALSPINBOXWIDGET_H
#define IVW_ORDINALSPINBOXWIDGET_H

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/qtwidgets/ordinalbasewidget.h>
#include <modules/qtwidgets/properties/valuedragspinbox.h>
#include <modules/qtwidgets/properties/doublevaluedragspinbox.h>

#include <modules/qtwidgets/qstringhelper.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QWidget>
#include <QLocale>
#include <warn/pop>

namespace inviwo {

class IVW_MODULE_QTWIDGETS_API BaseOrdinalSpinBoxWidget : public QWidget {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    BaseOrdinalSpinBoxWidget();
    virtual ~BaseOrdinalSpinBoxWidget();

    void setWrapping(bool wrap);
    bool wrapping() const;

protected:
    virtual double transformValueToEditor() = 0;
    virtual void newEditorValue(double) = 0;
    virtual double minimumValue() = 0;
    virtual double maximumValue() = 0;
    virtual double increment() = 0;
    virtual int spinnerDecimals() = 0;

    void applyInit();
    void applyValue();
    void applyRange();
    void applyIncrement();

    DoubleValueDragSpinBox* editor_;

signals:
    void valueChanged();

private:
    void updateFromEditor();
    void updateEditor();
};

template <typename T>
class TemplateOrdinalSpinBoxWidget : public BaseOrdinalSpinBoxWidget, public OrdinalBaseWidget<T> {
public:
    TemplateOrdinalSpinBoxWidget()
        : BaseOrdinalSpinBoxWidget(), value_(0), minValue_(0), maxValue_(0), increment_(0) {}
    virtual ~TemplateOrdinalSpinBoxWidget() = default;

    virtual T getValue() override;
    virtual void setValue(T value) override;
    virtual void initValue(T value) override;
    virtual void setMinValue(T minValue) override;
    virtual void setMaxValue(T maxValue) override;
    virtual void setRange(T minValue, T maxValue) override;
    virtual void setIncrement(T increment) override;

protected:
    // Define the transforms
    virtual T editorToRepr(double val) = 0;
    virtual double reprToEditor(T val) = 0;

    // Has default implementations using above transformations.
    virtual double transformValueToEditor() override;
    virtual void newEditorValue(double) override;
    virtual double minimumValue() override;
    virtual double maximumValue() override;
    virtual double increment() override;
    virtual int spinnerDecimals() override;

    T value_;
    T minValue_;
    T maxValue_;
    T increment_;
};

// Default case for fractional numbers
template <typename T>
class OrdinalSpinBoxWidget : public TemplateOrdinalSpinBoxWidget<T> {
public:
    OrdinalSpinBoxWidget() = default;
    virtual ~OrdinalSpinBoxWidget() = default;

protected:
    // Defines the transform
    virtual T editorToRepr(double val) { return static_cast<T>(val); }
    virtual double reprToEditor(T val) { return static_cast<double>(val); }
};

// Specialization for integer types
template <>
class OrdinalSpinBoxWidget<int> : public TemplateOrdinalSpinBoxWidget<int> {
public:
    OrdinalSpinBoxWidget() : TemplateOrdinalSpinBoxWidget<int>() {}
    virtual ~OrdinalSpinBoxWidget() = default;

protected:
    // Defines the transform
    virtual int editorToRepr(double val) override { return static_cast<int>(val + 0.5); }
    virtual double reprToEditor(int val) override { return static_cast<double>(val); }

    virtual int spinnerDecimals() override { return 0; }
};

template <typename T>
double TemplateOrdinalSpinBoxWidget<T>::transformValueToEditor() {
    return reprToEditor(value_);
}
template <typename T>
void TemplateOrdinalSpinBoxWidget<T>::newEditorValue(double val) {
    value_ = editorToRepr(val);
}
template <typename T>
double TemplateOrdinalSpinBoxWidget<T>::minimumValue() {
    return static_cast<double>(minValue_);
}
template <typename T>
double TemplateOrdinalSpinBoxWidget<T>::maximumValue() {
    return static_cast<double>(maxValue_);
}
template <typename T>
double TemplateOrdinalSpinBoxWidget<T>::increment() {
    return static_cast<double>(increment_);
}
template <typename T>
int TemplateOrdinalSpinBoxWidget<T>::spinnerDecimals() {
    const static QLocale locale;
    double inc = increment();
    std::ostringstream buff;
    utilqt::localizeStream(buff);
    buff << inc;
    const std::string str(buff.str());
    auto periodPosition = str.find(locale.decimalPoint().toLatin1());
    if (periodPosition == std::string::npos) {
        return 0;
    } else {
        return static_cast<int>(str.length() - periodPosition) - 1;
    }
}
template <typename T>
T TemplateOrdinalSpinBoxWidget<T>::getValue() {
    return value_;
}
template <typename T>
void TemplateOrdinalSpinBoxWidget<T>::setValue(T value) {
    if (value >= minValue_ && value <= maxValue_ && value != value_) {
        value_ = value;
        applyValue();
    }
}
template <typename T>
void TemplateOrdinalSpinBoxWidget<T>::initValue(T value) {
    value_ = value;
    applyInit();
}
template <typename T>
void TemplateOrdinalSpinBoxWidget<T>::setMinValue(T minValue) {
    if (minValue_ != minValue) {
        minValue_ = minValue;
        applyRange();
    }
}
template <typename T>
void TemplateOrdinalSpinBoxWidget<T>::setMaxValue(T maxValue) {
    if (maxValue_ != maxValue) {
        maxValue_ = maxValue;
        applyRange();
    }
}
template <typename T>
void TemplateOrdinalSpinBoxWidget<T>::setRange(T minValue, T maxValue) {
    bool changed = false;
    if (minValue_ != minValue) {
        minValue_ = minValue;
        changed = true;
    }
    if (maxValue_ != maxValue) {
        maxValue_ = maxValue;
        changed = true;
    }
    if (changed) {
        applyRange();
    }
}
template <typename T>
void TemplateOrdinalSpinBoxWidget<T>::setIncrement(T increment) {
    if (increment_ != increment) {
        increment_ = increment;
        applyIncrement();
    }
}

}  // namespace inviwo

#endif  // IVW_ORDINALSPINBOXWIDGET_H
