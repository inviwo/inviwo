/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_ORDINALPROPERTYWIDGETQT_H
#define IVW_ORDINALPROPERTYWIDGETQT_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <inviwo/qt/widgets/inviwoqtutils.h>
#include <inviwo/qt/widgets/editablelabelqt.h>
#include <inviwo/qt/widgets/sliderwidgetqt.h>
#include <inviwo/qt/widgets/ordinaleditorwidget.h>
#include <inviwo/qt/widgets/properties/propertysettingswidgetqt.h>
#include <inviwo/qt/widgets/properties/propertywidgetqt.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/tooltiphelper.h>
#include <inviwo/core/properties/propertyowner.h>

#include <math.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QMenu>
#include <QSignalMapper>
#include <warn/pop>

namespace inviwo {

class Property;

/**
 *  The Widget should work for FloatProperty and FloatVec(2|3|4)Property
 */
class IVW_QTWIDGETS_API BaseOrdinalPropertyWidgetQt : public PropertyWidgetQt {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>

public:
    BaseOrdinalPropertyWidgetQt(Property* property);
    virtual ~BaseOrdinalPropertyWidgetQt();
    virtual void updateFromProperty() = 0;
    virtual QMenu* getContextMenu();

    using SliderVector = std::vector<QWidget*>;

public slots:
    virtual void setPropertyValue(int sliderId) = 0;
    virtual void setAsMin() = 0;
    virtual void setAsMax() = 0;
    virtual void showSettings() = 0;
    virtual void showContextMenu(const QPoint& pos);

    void showContextMenuSlider(int sliderId);

protected:
    virtual SliderVector makeSliders(QWidget* widget) = 0;
    void generateWidget();
    SliderVector sliderWidgets_;
    int sliderId_;
    PropertySettingsWidgetQt* settingsWidget_;

private:
    EditableLabelQt* label_;
    QMenu* contextMenu_;

    QSignalMapper* signalMapperSetPropertyValue_;
    QSignalMapper* signalMapperContextMenu_;

    QAction* settingsAction_;
    QAction* minAction_;
    QAction* maxAction_;
    void generatesSettingsWidget();
};

template <typename BT, typename T>
class TemplateOrdinalPropertyWidgetQt : public BaseOrdinalPropertyWidgetQt {
public:
    TemplateOrdinalPropertyWidgetQt(OrdinalProperty<T>* property)
        : BaseOrdinalPropertyWidgetQt(property), ordinalproperty_(property) {}

    virtual ~TemplateOrdinalPropertyWidgetQt() {};

    virtual void updateFromProperty() = 0;
protected:

    virtual SliderVector makeSliders(QWidget* widget) {

        std::vector<QString> sphericalChars;
        sphericalChars.push_back(QString("r"));
        sphericalChars.push_back(QString("<html>&theta;</html>"));
        sphericalChars.push_back(QString("<html>&phi;</html>"));

        QSizePolicy sliderPol = widget->sizePolicy();
        sliderPol.setHorizontalStretch(3);
        widget->setSizePolicy(sliderPol);

        QGridLayout* vLayout = new QGridLayout();
        widget->setLayout(vLayout);
        vLayout->setContentsMargins(0, 0, 0, 0);
        vLayout->setSpacing(0);

        SliderVector sliders;
        QWidget* controlWidget;
        for (size_t j = 0; j < ordinalproperty_->getDim().y; j++) {
            for (size_t i = 0; i < ordinalproperty_->getDim().x; i++) {
                if (ordinalproperty_->getDim().y > 1 ||
                    ordinalproperty_->getSemantics() == PropertySemantics("Text")) {
                    controlWidget = new OrdinalEditorWidget<BT>();
                } else {
                    controlWidget = new SliderWidgetQt<BT>();
                }
                sliders.push_back(controlWidget);
                
                // Optionally add element descriptions
                QWidget* widget;
                if(ordinalproperty_->getSemantics() == PropertySemantics("Spherical")) {
                    
                    widget = new QWidget(this);
                    QHBoxLayout* hLayout = new QHBoxLayout();
                    hLayout->setContentsMargins(0, 0, 0, 0);
                    hLayout->setSpacing(7);
                    widget->setLayout(hLayout);
                    hLayout->addWidget(new QLabel(sphericalChars[i], this));
                    hLayout->addWidget(controlWidget);
                }else{
                    widget = controlWidget;
                }
                vLayout->addWidget(widget, static_cast<int>(i), static_cast<int>(j));
            }
        }
        return sliders;
    }

    virtual void showSettings() {
        if (!this->settingsWidget_) {
            this->settingsWidget_ =
                new TemplatePropertySettingsWidgetQt<BT, T>(ordinalproperty_, this);
        }
        this->settingsWidget_->showWidget();
    }

    virtual void setPropertyValue(int sliderId) = 0;
    virtual void setAsMin() = 0;
    virtual void setAsMax() = 0;

    OrdinalProperty<T>* ordinalproperty_;
};

template <typename T>
class PropertyTransformer {
public:
    PropertyTransformer(OrdinalProperty<T>* prop) : property_(prop) {}
    virtual ~PropertyTransformer() {};
    virtual T value(T val) = 0;
    virtual T min(T val) = 0;
    virtual T max(T val) = 0;
    virtual T inc(T val) = 0;

    virtual T invValue(T val) = 0;
    virtual T invMin(T val) = 0;
    virtual T invMax(T val) = 0;
    virtual T invInc(T val) = 0;

protected:
    OrdinalProperty<T>* property_;
};

template <typename T>
class IdentityPropertyTransformer : public PropertyTransformer<T> {
public:
    IdentityPropertyTransformer(OrdinalProperty<T>* prop) : PropertyTransformer<T>(prop) {}
    virtual ~IdentityPropertyTransformer() {};
    virtual T value(T val) { return val; }
    virtual T min(T val) { return val; }
    virtual T max(T val) { return val; }
    virtual T inc(T val) { return val; }

    virtual T invValue(T val) { return val; }
    virtual T invMin(T val) { return val; }
    virtual T invMax(T val) { return val; }
    virtual T invInc(T val) { return val; }
};

template <typename T>
class SphericalPropertyTransformer : public IdentityPropertyTransformer<T> {
public:
    SphericalPropertyTransformer(OrdinalProperty<T>* prop) : IdentityPropertyTransformer<T>(prop) {}
    virtual ~SphericalPropertyTransformer() {};
};

template <typename T>
class SphericalPropertyTransformer<glm::detail::tvec3<T, glm::defaultp> >
    : public PropertyTransformer<glm::detail::tvec3<T, glm::defaultp> > {
public:
    typedef glm::detail::tvec3<T, glm::defaultp> V;

    SphericalPropertyTransformer(OrdinalProperty<V>* prop) : PropertyTransformer<V>(prop) {}
    virtual ~SphericalPropertyTransformer() {};

    virtual V value(V val) {
        return V(
            static_cast<T>(std::sqrt(static_cast<double>(val[0] * val[0] + val[1] * val[1] + val[2] * val[2]))),
            arctan(val[2], static_cast<T>(std::sqrt(static_cast<double>(val[0] * val[0] + val[1] * val[1])))),
            arctan(val[0], val[1]));
    }
    virtual V min(V val) { return V(std::numeric_limits < T >::epsilon() , 0, -M_PI); }
    virtual V max(V val) {
        return V(
            3 * std::sqrt(static_cast<double>(val[0] * val[0] + val[1] * val[1] + val[2] * val[2])),
            M_PI, M_PI);
    }
    virtual V inc(V val) { return V(0.01, 0.01, 0.01); }

    virtual V invValue(V val) {
        return V(
            val[0] * std::sin(static_cast<double>(val[1])) * std::cos(static_cast<double>(val[2])),
            val[0] * std::sin(static_cast<double>(val[1])) * std::sin(static_cast<double>(val[2])),
            val[0] * std::cos(static_cast<double>(val[1])));
    }
    virtual V invMin(V val) { return this->property_->getMinValue(); }
    virtual V invMax(V val) { return this->property_->getMaxValue(); }
    virtual V invInc(V val) { return this->property_->getIncrement(); }

private:
    inline T arctan(T x, T y) {
        if (x == 0) {
            return static_cast<T>(M_PI_2);
        } else if (x < 0 && y > 0) {
            return static_cast<T>(std::atan(static_cast<double>(y / x)) + M_PI);
        } else if (x < 0 && y < 0) {
            return static_cast<T>(std::atan(static_cast<double>(y / x)) - M_PI);
        } else {
            return static_cast<T>(std::atan(static_cast<double>(y / x)));
        }
    }
};

template <typename BT, typename T>
class OrdinalPropertyWidgetQt : public TemplateOrdinalPropertyWidgetQt<BT, T> {
public:
    using SliderVectorTyped = std::vector<OrdinalBaseWidget<BT>*>;

    OrdinalPropertyWidgetQt(OrdinalProperty<T>* property)
        : TemplateOrdinalPropertyWidgetQt<BT, T>(property) {
        if (property->getSemantics() == PropertySemantics("Spherical")) {
            transformer_ = new SphericalPropertyTransformer<T>(property);
        } else {
            transformer_ = new IdentityPropertyTransformer<T>(property);
        }
        BaseOrdinalPropertyWidgetQt::generateWidget();

        const size_t nelem =
            this->ordinalproperty_->getDim().x * this->ordinalproperty_->getDim().y;
        for (size_t i = 0; i < nelem; i++) {
            auto widget = dynamic_cast<OrdinalBaseWidget<BT>*>(this->sliderWidgets_[i]);
            sliders_.push_back(widget);
        }

        updateFromProperty();
    }
    virtual ~OrdinalPropertyWidgetQt() { delete transformer_; }
    virtual void updateFromProperty() override;

    virtual std::string getToolTipText() override;

protected:
    // Connected to sliderwidget valueChanged()
    virtual void setPropertyValue(int) override;
    virtual void setAsMin() override;
    virtual void setAsMax() override;

    PropertyTransformer<T>* transformer_;
    SliderVectorTyped sliders_;
};

template <typename BT, typename T>
std::string OrdinalPropertyWidgetQt<BT, T>::getToolTipText() {
     ToolTipHelper t(this->ordinalproperty_->getDisplayName());
     t.tableTop();

     t.row("Identifier", this->ordinalproperty_->getIdentifier());
     t.row("Path", joinString(this->ordinalproperty_->getPath(), "."));
     t.row("Semantics", this->ordinalproperty_->getSemantics().getString());
     t.row("Validation Level",
           PropertyOwner::invalidationLevelToString(this->ordinalproperty_->getInvalidationLevel()));
     t.tableBottom();

     T min = transformer_->min(this->ordinalproperty_->getMinValue());
     T max = transformer_->max(this->ordinalproperty_->getMaxValue());
     T inc = transformer_->inc(this->ordinalproperty_->getIncrement());
     T val = transformer_->value(this->ordinalproperty_->get());

     t.tableTop();
     auto header = {"Value ", " Min ", " Max ", " Inc "};
     t.row("#", header, true);
     size_t size = this->ordinalproperty_->getDim().x * this->ordinalproperty_->getDim().y;
     for (size_t i = 0; i < size; i++) {
         auto row = {util::glmcomp(val, i), util::glmcomp(min, i), util::glmcomp(max, i),
                     util::glmcomp(inc, i)};
         t.row(i, row);
     }
     t.tableBottom();

     return t;
}

template <typename BT, typename T>
void OrdinalPropertyWidgetQt<BT, T>::updateFromProperty() {
    T min = transformer_->min(this->ordinalproperty_->getMinValue());
    T max = transformer_->max(this->ordinalproperty_->getMaxValue());
    T inc = transformer_->inc(this->ordinalproperty_->getIncrement());
    T val = transformer_->value(this->ordinalproperty_->get());

    const size_t nelem = this->ordinalproperty_->getDim().x * this->ordinalproperty_->getDim().y;
    for (size_t i = 0; i < nelem; i++) {
        sliders_[i]->setRange(util::glmcomp(min, i), util::glmcomp(max, i));
        sliders_[i]->setIncrement(util::glmcomp(inc, i));
        sliders_[i]->initValue(util::glmcomp(val, i));
    }
}

template <typename BT, typename T>
void OrdinalPropertyWidgetQt<BT, T>::setPropertyValue(int sliderId) {
    T propValue = transformer_->value(this->ordinalproperty_->get());

    util::glmcomp(propValue, sliderId) = sliders_[sliderId]->getValue();

    this->ordinalproperty_->setInitiatingWidget(this);
    this->ordinalproperty_->set(transformer_->invValue(propValue));
    this->ordinalproperty_->clearInitiatingWidget();
}

template <typename BT, typename T>
void OrdinalPropertyWidgetQt<BT, T>::setAsMin() {
    if (this->sliderId_ >= 0 && this->sliderId_ < this->sliderWidgets_.size()) {

        T propValue = transformer_->min(this->ordinalproperty_->getMinValue());        
        util::glmcomp(propValue, this->sliderId_) =  sliders_[this->sliderId_]->getValue();

        this->ordinalproperty_->setInitiatingWidget(this);
        this->ordinalproperty_->setMinValue(transformer_->invMin(propValue));
        this->ordinalproperty_->clearInitiatingWidget();

        sliders_[this->sliderId_]->setMinValue(util::glmcomp(propValue, this->sliderId_));
    }
}

template <typename BT, typename T>
void OrdinalPropertyWidgetQt<BT, T>::setAsMax() {
    if (this->sliderId_ >= 0 && this->sliderId_ < this->sliderWidgets_.size()) {

        T propValue = transformer_->max(this->ordinalproperty_->getMaxValue());
        util::glmcomp(propValue, this->sliderId_) =  sliders_[this->sliderId_]->getValue();

        this->ordinalproperty_->setInitiatingWidget(this);
        this->ordinalproperty_->setMaxValue(transformer_->invMax(propValue));
        this->ordinalproperty_->clearInitiatingWidget();

        sliders_[this->sliderId_]->setMaxValue(util::glmcomp(propValue, this->sliderId_));
    }
}

typedef OrdinalPropertyWidgetQt<float, float> FloatPropertyWidgetQt;
typedef OrdinalPropertyWidgetQt<float, vec2> FloatVec2PropertyWidgetQt;
typedef OrdinalPropertyWidgetQt<float, vec3> FloatVec3PropertyWidgetQt;
typedef OrdinalPropertyWidgetQt<float, vec4> FloatVec4PropertyWidgetQt;

typedef OrdinalPropertyWidgetQt<double, double> DoublePropertyWidgetQt;
typedef OrdinalPropertyWidgetQt<double, dvec2> DoubleVec2PropertyWidgetQt;
typedef OrdinalPropertyWidgetQt<double, dvec3> DoubleVec3PropertyWidgetQt;
typedef OrdinalPropertyWidgetQt<double, dvec4> DoubleVec4PropertyWidgetQt;

typedef OrdinalPropertyWidgetQt<int, int> IntPropertyWidgetQt;
typedef OrdinalPropertyWidgetQt<int, ivec2> IntVec2PropertyWidgetQt;
typedef OrdinalPropertyWidgetQt<int, ivec3> IntVec3PropertyWidgetQt;
typedef OrdinalPropertyWidgetQt<int, ivec4> IntVec4PropertyWidgetQt;

typedef OrdinalPropertyWidgetQt<size_t, size2_t> IntSize2PropertyWidgetQt;
typedef OrdinalPropertyWidgetQt<size_t, size3_t> IntSize3PropertyWidgetQt;
typedef OrdinalPropertyWidgetQt<size_t, size4_t> IntSize4PropertyWidgetQt;

typedef OrdinalPropertyWidgetQt<float, mat2> FloatMat2PropertyWidgetQt;
typedef OrdinalPropertyWidgetQt<float, mat3> FloatMat3PropertyWidgetQt;
typedef OrdinalPropertyWidgetQt<float, mat4> FloatMat4PropertyWidgetQt;

typedef OrdinalPropertyWidgetQt<double, dmat2> DoubleMat2PropertyWidgetQt;
typedef OrdinalPropertyWidgetQt<double, dmat3> DoubleMat3PropertyWidgetQt;
typedef OrdinalPropertyWidgetQt<double, dmat4> DoubleMat4PropertyWidgetQt;

typedef OrdinalPropertyWidgetQt<glm::i64, glm::i64> Int64PropertyWidgetQt;
}  // namespace

#endif  // IVW_ORDINALPROPERTYWIDGETQT_H