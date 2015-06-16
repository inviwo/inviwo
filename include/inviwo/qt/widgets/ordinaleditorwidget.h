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

#ifndef IVW_ORDINALEDITORWIDGET_H
#define IVW_ORDINALEDITORWIDGET_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/qt/widgets/sliderwidgetqt.h>
#include <inviwo/qt/widgets/properties/propertywidgetqt.h>
#include <inviwo/qt/widgets/qstringhelper.h>

namespace inviwo {

class IVW_QTWIDGETS_API BaseOrdinalEditorWidget : public QWidget  {
    Q_OBJECT
public:
    BaseOrdinalEditorWidget();
    virtual ~BaseOrdinalEditorWidget();

protected: 
    virtual QString transformValueToEditor() = 0;
    virtual void newEditorValue(QString) = 0;

    void applyInit();
    void applyValue();

    IvwLineEdit* editor_;

private slots:
    void updateFromEditor();

signals:
    void valueChanged();

private:
    void generateWidget();
    void updateEditor();


};

template <typename T>
class TemplateOrdinalEditorWidget : public BaseOrdinalEditorWidget, public OrdinalBaseWidget<T> {
public:
    TemplateOrdinalEditorWidget() : BaseOrdinalEditorWidget()
        , value_(0)
        , minValue_(0)
        , maxValue_(0)
        , increment_(0)
    {
    }
    virtual ~TemplateOrdinalEditorWidget() {}

    virtual T getValue();
    virtual void setValue(T value);
    virtual void initValue(T value);
    virtual void setMinValue(T minValue);
    virtual void setMaxValue(T maxValue);
    virtual void setRange(T minValue, T maxValue);
    virtual void setIncrement(T increment);

protected:
    // Define the transforms
    virtual T editorToRepr(QString val) = 0;
    virtual QString reprToEditor(T val) = 0;

    // Has default implementations using above transformations.
    virtual QString transformValueToEditor();
    virtual void newEditorValue(QString);
    
    T value_;
    T minValue_;
    T maxValue_;
    T increment_;
};


// Default case for fractional numbers
template <typename T>
class OrdinalEditorWidget : public TemplateOrdinalEditorWidget<T> {
public:
    OrdinalEditorWidget() : TemplateOrdinalEditorWidget<T>() {
        this->editor_->setValidator(new QDoubleValidator(this));
    }
    virtual ~OrdinalEditorWidget() {}

protected:  
    // Defines the transform
    virtual T editorToRepr(QString val) {
        QLocale locale = BaseOrdinalEditorWidget::editor_->locale();
        return static_cast<T>(locale.toDouble(val.remove(QChar(' '))));
    }
    virtual QString reprToEditor(T val) {
        QLocale locale = BaseOrdinalEditorWidget::editor_->locale();
        return QStringHelper<T>::toLocaleString(locale,val);// locale.toString(val);
    }
};

// Specialization for integer types
template <>
class OrdinalEditorWidget<int> : public TemplateOrdinalEditorWidget<int>{
public:
    OrdinalEditorWidget() : TemplateOrdinalEditorWidget<int>() {
        editor_->setValidator(new QIntValidator(this));
    }
    virtual ~OrdinalEditorWidget() {}

protected:
    // Defines the transform
    virtual int editorToRepr(QString val) {
        QLocale locale = BaseOrdinalEditorWidget::editor_->locale();
        return locale.toInt(val.remove(QChar(' ')));
    }
    virtual QString reprToEditor(int val) {
        QLocale locale = BaseOrdinalEditorWidget::editor_->locale();
        return locale.toString(val);
    }
};


template <typename T>
QString inviwo::TemplateOrdinalEditorWidget<T>::transformValueToEditor() {
    return reprToEditor(value_);
}
template <typename T>
void inviwo::TemplateOrdinalEditorWidget<T>::newEditorValue(QString val) {
    value_ = editorToRepr(val);
}
template <typename T>
T TemplateOrdinalEditorWidget<T>::getValue() {
    return value_;
}
template <typename T>
void TemplateOrdinalEditorWidget<T>::setValue(T value) {
    if(value >= minValue_ && value <= maxValue_ && value != value_) {
        value_ = value;
        applyValue();
    }
}
template <typename T>
void TemplateOrdinalEditorWidget<T>::initValue(T value) {
    value_ = value;
    applyInit();
}
template <typename T>
void TemplateOrdinalEditorWidget<T>::setMinValue(T minValue) {
    if(minValue_ != minValue) {
        minValue_ = minValue;
    }
}
template <typename T>
void TemplateOrdinalEditorWidget<T>::setMaxValue(T maxValue) {
    if(maxValue_ != maxValue) {
        maxValue_ = maxValue;
    }
}
template <typename T>
void TemplateOrdinalEditorWidget<T>::setRange(T minValue, T maxValue) {
    setMinValue(minValue);
    setMaxValue(maxValue);
}
template <typename T>
void TemplateOrdinalEditorWidget<T>::setIncrement(T increment) {
    if(increment_ != increment) {
        increment_ = increment;
    }
}



} // namespace

#endif // IVW_ORDINALEDITORWIDGET_H

