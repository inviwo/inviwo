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

#ifndef IVW_ORDINALMINMAXPROPERTYWIDGETQT_H
#define IVW_ORDINALMINMAXPROPERTYWIDGETQT_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <inviwo/qt/widgets/properties/propertywidgetqt.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/qt/widgets/customdoublespinboxqt.h>
#include <inviwo/qt/widgets/editablelabelqt.h>
#include <inviwo/qt/widgets/rangesliderqt.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/util/stringconversion.h>

namespace inviwo {


template<typename T>
class Transformer {
public:
    inline static T sliderToValue(MinMaxProperty<T>* p, int val) {
        return static_cast<T>(val);
    };
    inline static int valueToSlider(MinMaxProperty<T>* p, T val){
        return static_cast<int>(val);
    }
    inline static int sepToSlider(MinMaxProperty<T>* p, T sep){
        return static_cast<int>(sep);
    }
    
    static T spinboxToValue(MinMaxProperty<T>* p, double val){
        return static_cast<T>(val);
    };
    static double valueToSpinbox(MinMaxProperty<T>* p, T val){
        return static_cast<double>(val);
    }
};

template<>
class Transformer<float> {
public:
    inline static float sliderToValue(MinMaxProperty<float>* p, int val) {
        float steps = (p->getRangeMax() - p->getRangeMin())/p->getIncrement();
        steps = steps > 2048.f ? 2048.f : steps;
        float res = static_cast<float>(val);
        res = (res / steps) * (p->getRangeMax() - p->getRangeMin() ) + p->getRangeMin();
        return res;
    };
    inline static int valueToSlider(MinMaxProperty<float>* p, float val){
        float steps = (p->getRangeMax() - p->getRangeMin())/p->getIncrement();
        steps = steps > 2048.f ? 2048.f : steps;
        float res = (val - p->getRangeMin()) / (p->getRangeMax() - p->getRangeMin() ) * steps;
        return static_cast<int>(res);
    }
    inline static int sepToSlider(MinMaxProperty<float>* p, float sep){
        float steps = (p->getRangeMax() - p->getRangeMin())/p->getIncrement();
        steps = steps > 2048.f ? 2048.f : steps;
        return static_cast<int>(sep / (p->getRangeMax() - p->getRangeMin()) * steps);
    }
    
    static float spinboxToValue(MinMaxProperty<float>* p, double val){
        return static_cast<float>(val);
    };

    static double valueToSpinbox(MinMaxProperty<float>* p, float val){
        return static_cast<double>(val);
    }
};

template<>
class Transformer<double> {
public:
    inline static float sliderToValue(MinMaxProperty<double>* p, int val) {
        double steps = (p->getRangeMax() - p->getRangeMin())/p->getIncrement();
        steps = steps > 2048.0 ? 2048.0 : steps;
        double res = static_cast<float>(val);
        res = (res / steps) * (p->getRangeMax() - p->getRangeMin() ) + p->getRangeMin();
        return res;
    };
    inline static int valueToSlider(MinMaxProperty<double>* p, double val){
        double steps = (p->getRangeMax() - p->getRangeMin())/p->getIncrement();
        steps = steps > 2048.0 ? 2048.0 : steps;
        double res = (val - p->getRangeMin()) / (p->getRangeMax() - p->getRangeMin() ) * steps;
        return static_cast<int>(res);
    }
    inline static int sepToSlider(MinMaxProperty<double>* p, double sep){
        double steps = (p->getRangeMax() - p->getRangeMin())/p->getIncrement();
        steps = steps > 2048. ? 2048. : steps;
        return static_cast<int>(sep / (p->getRangeMax() - p->getRangeMin()) * steps);
    }
    
    static double spinboxToValue(MinMaxProperty<double>* p, double val){
        return val;
    };

    static double valueToSpinbox(MinMaxProperty<double>* p, double val){
        return val;
    }
};


class IVW_QTWIDGETS_API BaseOrdinalMinMaxPropertyWidgetQt : public PropertyWidgetQt {

    Q_OBJECT

public:
    BaseOrdinalMinMaxPropertyWidgetQt(Property* property);
    virtual ~BaseOrdinalMinMaxPropertyWidgetQt();
    
    virtual void updateFromProperty() = 0;
    
public slots:
    
    virtual void updateFromSlider(int valMin, int valMax) = 0;
    virtual void updateFromSpinBoxMin(double val) = 0;
    virtual void updateFromSpinBoxMax(double val) = 0;
    
    void setPropertyDisplayName();

protected:
    void generateWidget();

    RangeSliderQt* slider_;
    CustomDoubleSpinBoxQt* spinBoxMin_;
    CustomDoubleSpinBoxQt* spinBoxMax_;
    EditableLabelQt* label_;
};


template <typename T>
class OrdinalMinMaxPropertyWidgetQt : public BaseOrdinalMinMaxPropertyWidgetQt {

public:
    OrdinalMinMaxPropertyWidgetQt(MinMaxProperty<T>* property);
    virtual ~OrdinalMinMaxPropertyWidgetQt();

    virtual void updateFromProperty();
    virtual void updateFromSlider(int valMin, int valMax);
    virtual void updateFromSpinBoxMin(double val);
    virtual void updateFromSpinBoxMax(double val);

    typedef glm::detail::tvec2<T, glm::defaultp> V;
    
protected:
    virtual int transformIncrementToSpinnerDecimals() {
        double inc = Transformer<T>::valueToSpinbox(minMaxProperty_, minMaxProperty_->getIncrement());
        std::ostringstream buff;
        buff << inc;
        const std::string str(buff.str());
        auto periodPosition = str.find(".");
        if (periodPosition == std::string::npos) 
            return 0;
        else
            return str.length() - periodPosition - 1;
    }
    virtual std::string getToolTipText();
    
    MinMaxProperty<T>* minMaxProperty_;
};

typedef OrdinalMinMaxPropertyWidgetQt<double> DoubleMinMaxPropertyWidgetQt;
typedef OrdinalMinMaxPropertyWidgetQt<float> FloatMinMaxPropertyWidgetQt;
typedef OrdinalMinMaxPropertyWidgetQt<int> IntMinMaxPropertyWidgetQt;


template<typename T>
OrdinalMinMaxPropertyWidgetQt<T>::OrdinalMinMaxPropertyWidgetQt(MinMaxProperty<T>* property)
    : BaseOrdinalMinMaxPropertyWidgetQt(property)
    , minMaxProperty_(property) {
    
    BaseOrdinalMinMaxPropertyWidgetQt::generateWidget();
    updateFromProperty();
}

template<typename T>
OrdinalMinMaxPropertyWidgetQt<T>::~OrdinalMinMaxPropertyWidgetQt() {
}

template<typename T>
void OrdinalMinMaxPropertyWidgetQt<T>::updateFromProperty() {
    V val = minMaxProperty_->get();
    V range = minMaxProperty_->getRange();
    T inc = minMaxProperty_->getIncrement();
    T sep = minMaxProperty_->getMinSeparation();
    
    this->spinBoxMin_->blockSignals(true);
    this->spinBoxMax_->blockSignals(true);
    this->slider_->blockSignals(true);

    this->spinBoxMin_->setDisabled(minMaxProperty_->getReadOnly());
    this->spinBoxMax_->setDisabled(minMaxProperty_->getReadOnly());
    this->slider_->setDisabled(minMaxProperty_->getReadOnly());
    
    this->spinBoxMin_->setRange(
        Transformer<T>::valueToSpinbox(minMaxProperty_, range.x),
        Transformer<T>::valueToSpinbox(minMaxProperty_, range.y - sep)
    );
        
    this->spinBoxMin_->setSingleStep(
        Transformer<T>::valueToSpinbox(minMaxProperty_, inc)
    );
    
    this->spinBoxMin_->setValue(
        Transformer<T>::valueToSpinbox(minMaxProperty_, val.x)
    );
    
    this->spinBoxMax_->setRange(
        Transformer<T>::valueToSpinbox(minMaxProperty_, range.x + sep),
        Transformer<T>::valueToSpinbox(minMaxProperty_, range.y)
    );
        
    this->spinBoxMax_->setSingleStep(
        Transformer<T>::valueToSpinbox(minMaxProperty_, inc)
    );
    
    this->spinBoxMax_->setDecimals(transformIncrementToSpinnerDecimals());
    this->spinBoxMin_->setDecimals(transformIncrementToSpinnerDecimals());

    this->spinBoxMax_->setValue(
        Transformer<T>::valueToSpinbox(minMaxProperty_, val.y)
    );
    
    this->slider_->setRange(
        Transformer<T>::valueToSlider(minMaxProperty_, range.x),
        Transformer<T>::valueToSlider(minMaxProperty_, range.y)
    );
    
    this->slider_->setMinSeparation(
        Transformer<T>::sepToSlider(minMaxProperty_, sep)
    );
    
    this->slider_->setValue(
        Transformer<T>::valueToSlider(minMaxProperty_, val.x),
        Transformer<T>::valueToSlider(minMaxProperty_, val.y)
    );
    
    this->spinBoxMin_->blockSignals(false);
    this->spinBoxMax_->blockSignals(false);
    this->slider_->blockSignals(false);
}

template<typename T>
void OrdinalMinMaxPropertyWidgetQt<T>::updateFromSlider(int valMin, int valMax) {
    T min = Transformer<T>::sliderToValue(minMaxProperty_, valMin);
    T max = Transformer<T>::sliderToValue(minMaxProperty_, valMax);
    
    bool modified = false;
    V range = minMaxProperty_->get();
    
    if (std::abs(min - range.x) > glm::epsilon<T>()) {
        modified = true;
        range.x = min;
        this->spinBoxMin_->blockSignals(true);
        this->spinBoxMin_->setValue(Transformer<T>::valueToSpinbox(minMaxProperty_, min));
        this->spinBoxMin_->blockSignals(false);
    }
    
    if (std::abs(max - range.y) > glm::epsilon<T>()) {
        modified = true;
        range.y = max;
        this->spinBoxMax_->blockSignals(true);
        this->spinBoxMax_->setValue(Transformer<T>::valueToSpinbox(minMaxProperty_, max));
        this->spinBoxMax_->blockSignals(false);
    }
    
    if (modified) {
        minMaxProperty_->setInitiatingWidget(this);
        minMaxProperty_->set(range);
        minMaxProperty_->clearInitiatingWidget();
    }
}

template<typename T>
void OrdinalMinMaxPropertyWidgetQt<T>::updateFromSpinBoxMin(double minVal) {
    T min = Transformer<T>::spinboxToValue(minMaxProperty_, minVal);
    T sep = minMaxProperty_->getMinSeparation();
    V range = minMaxProperty_->get();
    
    if (std::abs(min - range.x) > glm::epsilon<T>()) {
        range.x = min;
        
        if (range.y-range.x < sep) {
            range.y = range.x + sep;
            this->spinBoxMax_->blockSignals(true);
            this->spinBoxMax_->setValue(Transformer<T>::valueToSpinbox(minMaxProperty_, range.y));
            this->spinBoxMax_->blockSignals(false);
        }
        
        this->slider_->blockSignals(true);
        this->slider_->setValue(
            Transformer<T>::valueToSlider(minMaxProperty_, range.x),
            Transformer<T>::valueToSlider(minMaxProperty_, range.y)
        );
        this->slider_->blockSignals(false);
        
        minMaxProperty_->setInitiatingWidget(this);
        minMaxProperty_->set(range);
        minMaxProperty_->clearInitiatingWidget();
    }
}
template<typename T>
void OrdinalMinMaxPropertyWidgetQt<T>::updateFromSpinBoxMax(double maxVal) {
    T max = Transformer<T>::spinboxToValue(minMaxProperty_, maxVal);
    T sep = minMaxProperty_->getMinSeparation();
    V range = minMaxProperty_->get();
    
    if (std::abs(max - range.y) > glm::epsilon<T>()) {
        range.y = max;
        
        if (range.y - range.x < sep) {
            range.x = range.y - sep;
            this->spinBoxMin_->blockSignals(true);
            this->spinBoxMin_->setValue(Transformer<T>::valueToSpinbox(minMaxProperty_, range.x));
            this->spinBoxMin_->blockSignals(false);
        }
        
        this->slider_->blockSignals(true);
        this->slider_->setValue(
            Transformer<T>::valueToSlider(minMaxProperty_, range.x),
            Transformer<T>::valueToSlider(minMaxProperty_, range.y)
        );
        this->slider_->blockSignals(false);
        
        minMaxProperty_->setInitiatingWidget(this);
        minMaxProperty_->set(range);
        minMaxProperty_->clearInitiatingWidget();
    }
}

template <typename T>
std::string OrdinalMinMaxPropertyWidgetQt<T>::getToolTipText() {
    std::stringstream ss;

    ss << this->makeToolTipTop(this->minMaxProperty_->getDisplayName());
    ss << this->makeToolTipTableTop();
    ss << this->makeToolTipRow("Identifier", this->minMaxProperty_->getIdentifier());
    ss << this->makeToolTipRow("Path", joinString(this->minMaxProperty_->getPath(),"."));
    ss << this->makeToolTipRow("Semantics", this->minMaxProperty_->getSemantics().getString());
    ss << this->makeToolTipRow("Validation Level", PropertyOwner::invalidationLevelToString(
                                             this->minMaxProperty_->getInvalidationLevel()));
    ss << this->makeToolTipRow("Minimum", toString(minMaxProperty_->get().x));
    ss << this->makeToolTipRow("Maximum", toString(minMaxProperty_->get().y));
    ss << this->makeToolTipRow("Range min", toString(minMaxProperty_->getRangeMin()));
    ss << this->makeToolTipRow("Range max", toString(minMaxProperty_->getRangeMax()));
    ss << this->makeToolTipRow("Increment", toString(minMaxProperty_->getIncrement()));
    ss << this->makeToolTipRow("Seperation", toString(minMaxProperty_->getMinSeparation()));

    ss << this->makeToolTipTableBottom();
    ss << this->makeToolTipBottom();
    return ss.str();
}



} // namespace

#endif // IVW_ORDINALMINMAXPROPERTYWIDGETQT_H

