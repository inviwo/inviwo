/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2017 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/editablelabelqt.h>
#include <modules/qtwidgets/sliderwidgetqt.h>
#include <modules/qtwidgets/ordinaleditorwidget.h>
#include <modules/qtwidgets/properties/propertysettingswidgetqt.h>
#include <modules/qtwidgets/properties/propertywidgetqt.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/properties/propertyowner.h>

#include <math.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QMenu>
#include <QSignalMapper>
#include <warn/pop>

namespace inviwo {

class Property;

template <typename T>
class PropertyTransformer {
public:
    PropertyTransformer(OrdinalProperty<T>* prop) : property_(prop) {}
    virtual ~PropertyTransformer() = default;
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
    virtual ~IdentityPropertyTransformer() = default;
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
    virtual ~SphericalPropertyTransformer() = default;
};

template <typename T>
class SphericalPropertyTransformer<glm::tvec3<T, glm::defaultp>>
    : public PropertyTransformer<glm::tvec3<T, glm::defaultp>> {
public:
    using V = glm::tvec3<T, glm::defaultp>;

    SphericalPropertyTransformer(OrdinalProperty<V>* prop) : PropertyTransformer<V>(prop) {}
    virtual ~SphericalPropertyTransformer(){};

    virtual V value(V val) {
        return V(static_cast<T>(std::sqrt(
                     static_cast<double>(val[0] * val[0] + val[1] * val[1] + val[2] * val[2]))),
                 arctan(val[2], static_cast<T>(std::sqrt(
                                    static_cast<double>(val[0] * val[0] + val[1] * val[1])))),
                 arctan(val[0], val[1]));
    }
    virtual V min(V val) { return V(std::numeric_limits<T>::epsilon(), 0, -M_PI); }
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
    T arctan(T x, T y) {
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

template <typename T>
class OrdinalPropertyWidgetQt : public PropertyWidgetQt {
public:
    using BT = typename util::value_type<T>::type;
    using SliderVectorTyped = std::vector<OrdinalBaseWidget<BT>*>;

    OrdinalPropertyWidgetQt(OrdinalProperty<T>* property);
    virtual ~OrdinalPropertyWidgetQt() = default;
    virtual void updateFromProperty() override;
    virtual std::unique_ptr<QMenu> getContextMenu() override;

private:
    // Connected to sliderwidget valueChanged()
    void setPropertyValue(int);
    void showSettings();

    OrdinalProperty<T>* ordinalproperty_;
    EditableLabelQt* label_;
    TemplatePropertySettingsWidgetQt<T>* settingsWidget_;

    SliderVectorTyped sliders_;
    std::unique_ptr<PropertyTransformer<T>> transformer_;
};

template <typename T>
OrdinalPropertyWidgetQt<T>::OrdinalPropertyWidgetQt(OrdinalProperty<T>* property)
    : PropertyWidgetQt(property)
    , ordinalproperty_(property)
    , label_{new EditableLabelQt(this, property_)}
    , settingsWidget_(nullptr)
    , transformer_{[&]() -> std::unique_ptr<PropertyTransformer<T>> {
        if (property->getSemantics() == PropertySemantics("Spherical")) {
            return util::make_unique<SphericalPropertyTransformer<T>>(property);
        } else {
            return util::make_unique<IdentityPropertyTransformer<T>>(property);
        }
    }()} {

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(PropertyWidgetQt::spacing);
    hLayout->addWidget(label_);

    const std::array<QString, 3> sphericalChars{QString("r"), QString("<html>&theta;</html>"),
                                                QString("<html>&phi;</html>")};

    QWidget* sliderWidget = new QWidget();
    QSizePolicy sliderPol = sliderWidget->sizePolicy();
    sliderPol.setHorizontalStretch(3);
    sliderWidget->setSizePolicy(sliderPol);

    QGridLayout* vLayout = new QGridLayout();
    sliderWidget->setLayout(vLayout);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);

    auto signalMapperSetPropertyValue = new QSignalMapper(this);

    for (size_t j = 0; j < ordinalproperty_->getDim().y; j++) {
        for (size_t i = 0; i < ordinalproperty_->getDim().x; i++) {
            QWidget* controlWidget;
            if (ordinalproperty_->getDim().y > 1 ||
                ordinalproperty_->getSemantics() == PropertySemantics("Text")) {

                auto editor = new OrdinalEditorWidget<BT>();
                connect(editor, &OrdinalEditorWidget<BT>::valueChanged,
                        signalMapperSetPropertyValue,
                        static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
                signalMapperSetPropertyValue->setMapping(editor, static_cast<int>(i));
                sliders_.push_back(editor);
                controlWidget = editor;
            } else {
                auto editor = new SliderWidgetQt<BT>();
                connect(editor, &SliderWidgetQt<BT>::valueChanged, signalMapperSetPropertyValue,
                        static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
                signalMapperSetPropertyValue->setMapping(editor, static_cast<int>(i));
                sliders_.push_back(editor);
                controlWidget = editor;
            }

            // Optionally add element descriptions
            QWidget* edwidget;
            if (ordinalproperty_->getSemantics() == PropertySemantics("Spherical")) {
                edwidget = new QWidget(this);
                QHBoxLayout* edLayout = new QHBoxLayout();
                edLayout->setContentsMargins(0, 0, 0, 0);
                edLayout->setSpacing(7);
                edwidget->setLayout(edLayout);
                edLayout->addWidget(new QLabel(sphericalChars[i], this));
                edLayout->addWidget(static_cast<QWidget*>(controlWidget));
            } else {
                edwidget = controlWidget;
            }
            vLayout->addWidget(edwidget, static_cast<int>(i), static_cast<int>(j));
        }
    }

    hLayout->addWidget(sliderWidget);

    sliderWidget->setMinimumHeight(sliderWidget->sizeHint().height());
    QSizePolicy sp = sliderWidget->sizePolicy();
    sp.setVerticalPolicy(QSizePolicy::Fixed);
    sliderWidget->setSizePolicy(sp);

    connect(signalMapperSetPropertyValue,
            static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mapped), this,
            &OrdinalPropertyWidgetQt<T>::setPropertyValue);

    setLayout(hLayout);

    setFixedHeight(sizeHint().height());
    sp = sizePolicy();
    sp.setVerticalPolicy(QSizePolicy::Fixed);
    setSizePolicy(sp);

    updateFromProperty();
}

template <typename T>
void OrdinalPropertyWidgetQt<T>::updateFromProperty() {
    T min = transformer_->min(ordinalproperty_->getMinValue());
    T max = transformer_->max(ordinalproperty_->getMaxValue());
    T inc = transformer_->inc(ordinalproperty_->getIncrement());
    T val = transformer_->value(ordinalproperty_->get());

    const size_t nelem = ordinalproperty_->getDim().x * ordinalproperty_->getDim().y;
    for (size_t i = 0; i < nelem; i++) {
        sliders_[i]->setRange(util::glmcomp(min, i), util::glmcomp(max, i));
        sliders_[i]->setIncrement(util::glmcomp(inc, i));
        sliders_[i]->initValue(util::glmcomp(val, i));
    }
}

template <typename T>
void OrdinalPropertyWidgetQt<T>::setPropertyValue(int sliderId) {
    T propValue = transformer_->value(ordinalproperty_->get());

    util::glmcomp(propValue, sliderId) = sliders_[sliderId]->getValue();

    ordinalproperty_->setInitiatingWidget(this);
    ordinalproperty_->set(transformer_->invValue(propValue));
    ordinalproperty_->clearInitiatingWidget();
}

template <typename T>
void OrdinalPropertyWidgetQt<T>::showSettings() {
    if (!settingsWidget_) {
        settingsWidget_ = new TemplatePropertySettingsWidgetQt<T>(ordinalproperty_, this);
    }
    settingsWidget_->showWidget();
}

template <typename T>
std::unique_ptr<QMenu> OrdinalPropertyWidgetQt<T>::getContextMenu() {
    auto menu = PropertyWidgetQt::getContextMenu();

    auto settingsAction = menu->addAction(tr("&Property settings..."));
    settingsAction->setToolTip(
        tr("&Open the property settings dialog to adjust min, max, and increment values"));

    connect(settingsAction, &QAction::triggered, this,
            &OrdinalPropertyWidgetQt<T>::showSettings);

    settingsAction->setEnabled(!property_->getReadOnly());
    settingsAction->setVisible(getApplicationUsageMode() == UsageMode::Development);

    return menu;
}

using FloatPropertyWidgetQt = OrdinalPropertyWidgetQt<float>;
using FloatVec2PropertyWidgetQt = OrdinalPropertyWidgetQt<vec2>;
using FloatVec3PropertyWidgetQt = OrdinalPropertyWidgetQt<vec3>;
using FloatVec4PropertyWidgetQt = OrdinalPropertyWidgetQt<vec4>;

using DoublePropertyWidgetQt = OrdinalPropertyWidgetQt<double>;
using DoubleVec2PropertyWidgetQt = OrdinalPropertyWidgetQt<dvec2>;
using DoubleVec3PropertyWidgetQt = OrdinalPropertyWidgetQt<dvec3>;
using DoubleVec4PropertyWidgetQt = OrdinalPropertyWidgetQt<dvec4>;

using IntPropertyWidgetQt = OrdinalPropertyWidgetQt<int>;
using IntSizeTPropertyWidgetQt = OrdinalPropertyWidgetQt<size_t>;
using IntVec2PropertyWidgetQt = OrdinalPropertyWidgetQt<ivec2>;
using IntVec3PropertyWidgetQt = OrdinalPropertyWidgetQt<ivec3>;
using IntVec4PropertyWidgetQt = OrdinalPropertyWidgetQt<ivec4>;

using IntSize2PropertyWidgetQt = OrdinalPropertyWidgetQt<size2_t>;
using IntSize3PropertyWidgetQt = OrdinalPropertyWidgetQt<size3_t>;
using IntSize4PropertyWidgetQt = OrdinalPropertyWidgetQt<size4_t>;

using FloatMat2PropertyWidgetQt = OrdinalPropertyWidgetQt<mat2>;
using FloatMat3PropertyWidgetQt = OrdinalPropertyWidgetQt<mat3>;
using FloatMat4PropertyWidgetQt = OrdinalPropertyWidgetQt<mat4>;

using DoubleMat2PropertyWidgetQt = OrdinalPropertyWidgetQt<dmat2>;
using DoubleMat3PropertyWidgetQt = OrdinalPropertyWidgetQt<dmat3>;
using DoubleMat4PropertyWidgetQt = OrdinalPropertyWidgetQt<dmat4>;

using Int64PropertyWidgetQt = OrdinalPropertyWidgetQt<glm::i64>;

}  // namespace inviwo

#endif  // IVW_ORDINALPROPERTYWIDGETQT_H