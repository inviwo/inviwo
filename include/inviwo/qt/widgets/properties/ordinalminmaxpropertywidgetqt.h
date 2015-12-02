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
#include <inviwo/qt/widgets/inviwoqtutils.h>
#include <inviwo/qt/widgets/properties/propertywidgetqt.h>
#include <inviwo/qt/widgets/properties/propertysettingswidgetqt.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/qt/widgets/customdoublespinboxqt.h>
#include <inviwo/qt/widgets/editablelabelqt.h>
#include <inviwo/qt/widgets/rangesliderqt.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/tooltiphelper.h>

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
        double res = static_cast<double>(val);
        res = (res / steps) * (p->getRangeMax() - p->getRangeMin() ) + p->getRangeMin();
        return static_cast<float>(res);
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
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>

public:
    BaseOrdinalMinMaxPropertyWidgetQt(Property* property);
    virtual ~BaseOrdinalMinMaxPropertyWidgetQt();
    virtual void updateFromProperty() = 0;
    virtual QMenu* getContextMenu();
    
public slots:
    virtual void updateFromSlider(int valMin, int valMax) = 0;
    virtual void updateFromSpinBoxMin(double val) = 0;
    virtual void updateFromSpinBoxMax(double val) = 0;
    virtual void showSettings() = 0;
    virtual void showContextMenu(const QPoint& pos);

protected:
    void generateWidget();
    PropertySettingsWidgetQt* settingsWidget_;

    RangeSliderQt* slider_;
    CustomDoubleSpinBoxQt* spinBoxMin_;
    CustomDoubleSpinBoxQt* spinBoxMax_;
    EditableLabelQt* label_;

    QMenu* contextMenu_;
    QAction* settingsAction_;
    void generatesSettingsWidget();
};


template <typename BT, typename T>
class OrdinalMinMaxPropertyWidgetQt : public BaseOrdinalMinMaxPropertyWidgetQt {

public:
    OrdinalMinMaxPropertyWidgetQt(MinMaxProperty<T>* property);
    virtual ~OrdinalMinMaxPropertyWidgetQt();

    virtual void updateFromProperty() override;
    virtual void updateFromSlider(int valMin, int valMax) override;
    virtual void updateFromSpinBoxMin(double val) override;
    virtual void updateFromSpinBoxMax(double val) override;

    typedef glm::detail::tvec2<T, glm::defaultp> V;
    
protected:
    virtual int transformIncrementToSpinnerDecimals() {
        const static QLocale locale;
        double inc = Transformer<T>::valueToSpinbox(minMaxProperty_, minMaxProperty_->getIncrement());
        std::ostringstream buff;
        utilqt::localizeStream(buff);
        buff << inc;
        const std::string str(buff.str());
        auto periodPosition = str.find(locale.decimalPoint().toLatin1());
        if (periodPosition == std::string::npos) 
            return 0;
        else
            return static_cast<int>(str.length() - periodPosition - 1);
    }
    virtual std::string getToolTipText() override;

    virtual void showSettings() override {
        if (!this->settingsWidget_) {
            this->settingsWidget_ =
                new TemplateMinMaxPropertySettingsWidgetQt<BT, T>(minMaxProperty_, this);
        }
        this->settingsWidget_->showWidget();
    }
    
    MinMaxProperty<T>* minMaxProperty_;
};

typedef OrdinalMinMaxPropertyWidgetQt<double, double> DoubleMinMaxPropertyWidgetQt;
typedef OrdinalMinMaxPropertyWidgetQt<float, float> FloatMinMaxPropertyWidgetQt;
typedef OrdinalMinMaxPropertyWidgetQt<int, int> IntMinMaxPropertyWidgetQt;


template <typename BT, typename T>
OrdinalMinMaxPropertyWidgetQt<BT, T>::OrdinalMinMaxPropertyWidgetQt(MinMaxProperty<T>* property)
    : BaseOrdinalMinMaxPropertyWidgetQt(property)
    , minMaxProperty_(property) {
    
    BaseOrdinalMinMaxPropertyWidgetQt::generateWidget();
    updateFromProperty();
}

template <typename BT, typename T>
OrdinalMinMaxPropertyWidgetQt<BT, T>::~OrdinalMinMaxPropertyWidgetQt() {
}

template<typename BT, typename T>
void OrdinalMinMaxPropertyWidgetQt<BT, T>::updateFromProperty() {
    V val = minMaxProperty_->get();
    V range = minMaxProperty_->getRange();
    T inc = minMaxProperty_->getIncrement();
    T sep = minMaxProperty_->getMinSeparation();
    
    this->spinBoxMin_->blockSignals(true);
    this->spinBoxMax_->blockSignals(true);
    this->slider_->blockSignals(true);
    
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

template<typename BT, typename T>
void OrdinalMinMaxPropertyWidgetQt<BT, T>::updateFromSlider(int valMin, int valMax) {
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

template<typename BT, typename T>
void OrdinalMinMaxPropertyWidgetQt<BT, T>::updateFromSpinBoxMin(double minVal) {
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

template<typename BT, typename T>
void OrdinalMinMaxPropertyWidgetQt<BT, T>::updateFromSpinBoxMax(double maxVal) {
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

template<typename BT, typename T>
std::string OrdinalMinMaxPropertyWidgetQt<BT, T>::getToolTipText() {
    ToolTipHelper t(this->minMaxProperty_->getDisplayName());

    t.tableTop();
    t.row("Identifier", this->minMaxProperty_->getIdentifier());
    t.row("Path", joinString(this->minMaxProperty_->getPath(), "."));
    t.row("Semantics", this->minMaxProperty_->getSemantics().getString());
    t.row("Validation Level",
          PropertyOwner::invalidationLevelToString(this->minMaxProperty_->getInvalidationLevel()));
    t.row("Minimum Bound", minMaxProperty_->getRangeMin());
    t.row("Start", minMaxProperty_->get().x);
    t.row("End", minMaxProperty_->get().y);
    t.row("Maximum Bound", minMaxProperty_->getRangeMax());
    t.row("Increment", minMaxProperty_->getIncrement());
    t.row("Separation", minMaxProperty_->getMinSeparation());

    t.tableBottom();

    return t;
}

} // namespace

#endif // IVW_ORDINALMINMAXPROPERTYWIDGETQT_H

