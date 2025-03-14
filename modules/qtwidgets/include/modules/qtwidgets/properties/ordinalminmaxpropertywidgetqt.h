/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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

#include <inviwo/core/properties/minmaxproperty.h>                  // IWYU pragma: keep
#include <inviwo/core/properties/property.h>                        // for Property
#include <inviwo/core/util/glm.h>                                   // for almostEqual
#include <inviwo/core/util/glmutils.h>                              // for is_floating_point
#include <modules/qtwidgets/editablelabelqt.h>                      // for EditableLabelQt
#include <modules/qtwidgets/inviwoqtutils.h>                        // for emToPx, decimals
#include <modules/qtwidgets/numberwidget.h>                         // for NumberWidget
#include <modules/qtwidgets/properties/propertysettingswidgetqt.h>  // for MinMaxPropertySetting...
#include <modules/qtwidgets/properties/propertywidgetqt.h>          // for PropertyWidgetQt
#include <modules/qtwidgets/rangesliderqt.h>                        // for RangeSliderQt

#include <cstddef>      // for size_t
#include <memory>       // for unique_ptr
#include <type_traits>  // for is_integral, enable_if

#include <QAction>          // for QAction
#include <QDoubleSpinBox>   // for QDoubleSpinBox
#include <QHBoxLayout>      // for QHBoxLayout
#include <QSignalBlocker>   // for QSignalBlocker
#include <QSizePolicy>      // for QSizePolicy
#include <QSplitterHandle>  // for QSplitterHandle
#include <QWidget>          // for QWidget
#include <Qt>               // for Horizontal
#include <QMenu>            // IWYU pragma: keep

#include <glm/detail/qualifier.hpp>    // for defaultp, tvec2
#include <glm/gtc/type_precision.hpp>  // for i64

class QHBoxLayout;

namespace inviwo {

template <typename T, class Enable = void>
class Transformer {
public:
    static T sliderToValue(MinMaxProperty<T>*, int val) { return static_cast<T>(val); };
    static int valueToSlider(MinMaxProperty<T>*, T val) { return static_cast<int>(val); }
    static int sepToSlider(MinMaxProperty<T>*, T sep) { return static_cast<int>(sep); }
};

template <typename T>
class Transformer<T, typename std::enable_if<util::is_floating_point<T>::value>::type> {
public:
    static T sliderToValue(MinMaxProperty<T>* p, int val) {
        return (static_cast<T>(val) / steps()) * (p->getRangeMax() - p->getRangeMin()) +
               p->getRangeMin();
    }
    static int valueToSlider(MinMaxProperty<T>* p, T val) {
        return static_cast<int>((val - p->getRangeMin()) / (p->getRangeMax() - p->getRangeMin()) *
                                steps());
    }
    static int sepToSlider(MinMaxProperty<T>* p, T sep) {
        return static_cast<int>(sep / (p->getRangeMax() - p->getRangeMin()) * steps());
    }
    static T steps() { return 10000; }
};

template <typename T>
class OrdinalMinMaxPropertyWidgetQt : public PropertyWidgetQt {
public:
    OrdinalMinMaxPropertyWidgetQt(MinMaxProperty<T>* property);
    virtual ~OrdinalMinMaxPropertyWidgetQt() = default;

    virtual void updateFromProperty() override;
    virtual std::unique_ptr<QMenu> getContextMenu() override;

    using V = glm::tvec2<T, glm::defaultp>;

private:
    void updateFromSlider(int valMin, int valMax);
    void updateFromMin();
    void updateFromMax();
    int transformIncrementToSpinnerDecimals();
    void showSettings();

    MinMaxPropertySettingsWidgetQt<T>* settings_;
    RangeSliderQt* slider_;
    NumberWidget<T>* min_;
    NumberWidget<T>* max_;
    EditableLabelQt* label_;
    MinMaxProperty<T>* minMaxProperty_;
};

using DoubleMinMaxPropertyWidgetQt = OrdinalMinMaxPropertyWidgetQt<double>;
using FloatMinMaxPropertyWidgetQt = OrdinalMinMaxPropertyWidgetQt<float>;
using IntSizeTMinMaxPropertyWidgetQt = OrdinalMinMaxPropertyWidgetQt<size_t>;
using Int64MinMaxPropertyWidgetQt = OrdinalMinMaxPropertyWidgetQt<glm::i64>;
using IntMinMaxPropertyWidgetQt = OrdinalMinMaxPropertyWidgetQt<int>;

template <typename T>
OrdinalMinMaxPropertyWidgetQt<T>::OrdinalMinMaxPropertyWidgetQt(MinMaxProperty<T>* property)
    : PropertyWidgetQt(property)
    , settings_(nullptr)
    , slider_(new RangeSliderQt(Qt::Horizontal, this))
    , min_(new NumberWidget<T>{NumberWidgetConfig{
          .interaction = NumberWidgetConfig::Interaction::NoDragging,
          .barVisible = false,
      }})
    , max_(new NumberWidget<T>{NumberWidgetConfig{
          .interaction = NumberWidgetConfig::Interaction::NoDragging,
          .barVisible = false,
      }})
    , label_(new EditableLabelQt(this, property_))
    , minMaxProperty_(property) {

    QHBoxLayout* hLayout = new QHBoxLayout();
    setSpacingAndMargins(hLayout);
    hLayout->addWidget(label_);

    slider_->handle(0)->setFocusProxy(min_);
    slider_->handle(1)->setFocusProxy(max_);

    setFocusPolicy(min_->focusPolicy());
    setFocusProxy(min_);

    QHBoxLayout* hSliderLayout = new QHBoxLayout();
    QWidget* sliderWidget = new QWidget();
    sliderWidget->setLayout(hSliderLayout);
    hSliderLayout->setContentsMargins(0, 0, 0, 0);

    min_->setFixedWidth(utilqt::emToPx(this, 4.6));
    hSliderLayout->addWidget(min_);

    hSliderLayout->addWidget(slider_);

    max_->setFixedWidth(utilqt::emToPx(this, 4.6));
    hSliderLayout->addWidget(max_);

    hLayout->addWidget(sliderWidget);
    setLayout(hLayout);

    QSizePolicy slidersPol = sliderWidget->sizePolicy();
    slidersPol.setHorizontalStretch(3);
    sliderWidget->setSizePolicy(slidersPol);

    connect(slider_, &RangeSliderQt::valuesChanged, this,
            &OrdinalMinMaxPropertyWidgetQt<T>::updateFromSlider);
    connect(min_, &NumberWidget<T>::valueChanged, this,
            &OrdinalMinMaxPropertyWidgetQt<T>::updateFromMin);
    connect(max_, &NumberWidget<T>::valueChanged, this,
            &OrdinalMinMaxPropertyWidgetQt<T>::updateFromMax);

    updateFromProperty();
}

template <typename T>
void OrdinalMinMaxPropertyWidgetQt<T>::updateFromProperty() {
    const V val = minMaxProperty_->get();
    const V range = minMaxProperty_->getRange();
    const T inc = minMaxProperty_->getIncrement();
    const T sep = minMaxProperty_->getMinSeparation();

    QSignalBlocker minBlock(min_);
    QSignalBlocker maxBlock(max_);
    QSignalBlocker slideBlock(slider_);

    min_->setMinValue(range.x, ConstraintBehavior::Editable);
    min_->setMaxValue(range.y - sep, ConstraintBehavior::Editable);

    max_->setMinValue(range.x + sep, ConstraintBehavior::Editable);
    max_->setMaxValue(range.y, ConstraintBehavior::Editable);

    min_->initValue(val.x);
    max_->initValue(val.y);

    min_->setIncrement(inc);
    max_->setIncrement(inc);

    using F = Transformer<T>;
    const auto p = minMaxProperty_;

    slider_->setRange(F::valueToSlider(p, range.x), F::valueToSlider(p, range.y));
    slider_->setMinSeparation(F::sepToSlider(p, sep));
    slider_->setValue(F::valueToSlider(p, val.x), F::valueToSlider(p, val.y));
}

template <typename T>
void OrdinalMinMaxPropertyWidgetQt<T>::updateFromSlider(int valMin, int valMax) {
    const T min = Transformer<T>::sliderToValue(minMaxProperty_, valMin);
    const T max = Transformer<T>::sliderToValue(minMaxProperty_, valMax);

    const V newRange = minMaxProperty_->clamp(V{min, max});

    bool modified = false;
    V range = minMaxProperty_->get();

    if (!util::almostEqual(newRange.x, range.x)) {
        modified = true;
        range.x = newRange.x;
        QSignalBlocker minBlock(min_);
        min_->setValue(newRange.x);
    }

    if (!util::almostEqual(newRange.y, range.y)) {
        modified = true;
        range.y = newRange.y;
        QSignalBlocker maxBlock(max_);
        max_->setValue(newRange.y);
    }

    if (modified) {
        minMaxProperty_->setInitiatingWidget(this);
        minMaxProperty_->set(range);
        minMaxProperty_->clearInitiatingWidget();
    }
}

template <typename T>
void OrdinalMinMaxPropertyWidgetQt<T>::updateFromMin() {
    const T min = min_->getValue();
    const T sep = minMaxProperty_->getMinSeparation();
    V range = minMaxProperty_->get();

    if (!util::almostEqual(min, range.x)) {
        range.x = min;

        const QSignalBlocker slideBlock(slider_);
        if (range.y - range.x < sep) {
            range.y = range.x + sep;
            const QSignalBlocker maxBlock(max_);
            max_->setValue(range.y);
            slider_->setValue(Transformer<T>::valueToSlider(minMaxProperty_, range.x),
                              Transformer<T>::valueToSlider(minMaxProperty_, range.y));
        } else {
            slider_->setMinValue(Transformer<T>::valueToSlider(minMaxProperty_, range.x));
        }

        minMaxProperty_->setInitiatingWidget(this);
        minMaxProperty_->set(range);
        minMaxProperty_->clearInitiatingWidget();
    }
}

template <typename T>
void OrdinalMinMaxPropertyWidgetQt<T>::updateFromMax() {
    const T max = max_->getValue();
    const T sep = minMaxProperty_->getMinSeparation();
    V range = minMaxProperty_->get();

    if (!util::almostEqual(max, range.y)) {
        range.y = max;

        const QSignalBlocker slideBlock(slider_);
        if (range.y - range.x < sep) {
            range.x = range.y - sep;
            const QSignalBlocker minBlock(min_);
            min_->setValue(range.x);
            slider_->setValue(Transformer<T>::valueToSlider(minMaxProperty_, range.x),
                              Transformer<T>::valueToSlider(minMaxProperty_, range.y));
        } else {
            slider_->setMaxValue(Transformer<T>::valueToSlider(minMaxProperty_, range.y));
        }

        minMaxProperty_->setInitiatingWidget(this);
        minMaxProperty_->set(range);
        minMaxProperty_->clearInitiatingWidget();
    }
}

template <typename T>
std::unique_ptr<QMenu> OrdinalMinMaxPropertyWidgetQt<T>::getContextMenu() {
    auto menu = PropertyWidgetQt::getContextMenu();
    auto settingsAction = menu->addAction(tr("&Property settings..."));
    settingsAction->setToolTip(
        tr("&Open the property settings dialog to adjust min bound, start, end, max bound, "
           "minSepration and increment values"));

    connect(settingsAction, &QAction::triggered, this,
            &OrdinalMinMaxPropertyWidgetQt<T>::showSettings);
    settingsAction->setEnabled(!property_->getReadOnly());
    return menu;
}

template <typename T>
int OrdinalMinMaxPropertyWidgetQt<T>::transformIncrementToSpinnerDecimals() {
    return utilqt::decimals(minMaxProperty_->getIncrement());
}

template <typename T>
void OrdinalMinMaxPropertyWidgetQt<T>::showSettings() {
    if (!settings_) {
        settings_ = new MinMaxPropertySettingsWidgetQt<T>(minMaxProperty_, this);
    }
    settings_->showWidget();
}

}  // namespace inviwo
