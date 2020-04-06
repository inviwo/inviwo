/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2020 Inviwo Foundation
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
#include <inviwo/core/common/inviwo.h>

#include <modules/qtwidgets/ordinalbasewidget.h>
#include <modules/qtwidgets/numberlineedit.h>
#include <modules/qtwidgets/qstringhelper.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QWidget>
#include <warn/pop>

namespace inviwo {

class IVW_MODULE_QTWIDGETS_API BaseOrdinalEditorWidget : public QWidget {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    BaseOrdinalEditorWidget(bool intMode);
    virtual ~BaseOrdinalEditorWidget();

protected:
    virtual double transformValueToEditor() = 0;
    virtual double transformMinValueToEditor() = 0;
    virtual double transformMaxValueToEditor() = 0;
    virtual double transformIncrementToEditor() = 0;
    virtual int transformIncrementToEditorDecimals() = 0;

    virtual void newEditorValue(double) = 0;

    void applyInit();
    void applyValue();
    void applyMinValue();
    void applyMaxValue();
    void applyIncrement();

    NumberLineEdit* editor_;
    ConstraintBehavior minCB_;
    ConstraintBehavior maxCB_;

signals:
    void valueChanged();

private:
    void updateFromEditor();
    void updateEditor();
};

template <typename T>
class OrdinalEditorWidget final : public BaseOrdinalEditorWidget, public OrdinalBaseWidget<T> {
public:
    OrdinalEditorWidget();
    virtual ~OrdinalEditorWidget() = default;

    // Implements OrdinalBaseWidget
    virtual T getValue() const override;
    virtual void setValue(T value) override;
    virtual void initValue(T value) override;
    virtual void setMinValue(T minValue, ConstraintBehavior cb) override;
    virtual void setMaxValue(T maxValue, ConstraintBehavior cb) override;
    virtual void setIncrement(T increment) override;

protected:
    virtual double transformValueToEditor() override;
    virtual double transformMinValueToEditor() override;
    virtual double transformMaxValueToEditor() override;
    virtual double transformIncrementToEditor() override;
    virtual int transformIncrementToEditorDecimals() override;
    virtual void newEditorValue(double val) override;

    T value_;
    T minValue_;
    T maxValue_;
    T increment_;
};

template <typename T>
inline OrdinalEditorWidget<T>::OrdinalEditorWidget()
    : BaseOrdinalEditorWidget(!std::is_floating_point_v<T>)
    , value_{1}
    , minValue_{0}
    , maxValue_{2}
    , increment_{1} {}

template <typename T>
double OrdinalEditorWidget<T>::transformValueToEditor() {
    return static_cast<double>(value_);
}
template <typename T>
double OrdinalEditorWidget<T>::transformMinValueToEditor() {
    return static_cast<double>(minValue_);
}
template <typename T>
double OrdinalEditorWidget<T>::transformMaxValueToEditor() {
    return static_cast<double>(maxValue_);
}
template <typename T>
double OrdinalEditorWidget<T>::transformIncrementToEditor() {
    return static_cast<double>(increment_);
}
template <typename T>
int OrdinalEditorWidget<T>::transformIncrementToEditorDecimals() {
    return utilqt::decimals<T>(static_cast<double>(increment_));
}
template <typename T>
void OrdinalEditorWidget<T>::newEditorValue(double val) {
    value_ = static_cast<T>(val);
}

template <typename T>
T OrdinalEditorWidget<T>::getValue() const {
    return value_;
}
template <typename T>
void OrdinalEditorWidget<T>::setValue(T value) {
    if (value != value_) {
        value_ = value;
        applyValue();
    }
}
template <typename T>
void OrdinalEditorWidget<T>::initValue(T value) {
    value_ = value;
    applyInit();
}
template <typename T>
void OrdinalEditorWidget<T>::setMinValue(T minValue, ConstraintBehavior cb) {
    if (minValue_ != minValue || minCB_ != cb) {
        minValue_ = minValue;
        minCB_ = cb;
        applyMinValue();
    }
}
template <typename T>
void OrdinalEditorWidget<T>::setMaxValue(T maxValue, ConstraintBehavior cb) {
    if (maxValue_ != maxValue || maxCB_ != cb) {
        maxValue_ = maxValue;
        maxCB_ = cb;
        applyMaxValue();
    }
}
template <typename T>
void OrdinalEditorWidget<T>::setIncrement(T increment) {
    if (increment_ != increment) {
        increment_ = increment;
        applyIncrement();
    }
}

}  // namespace inviwo
