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

#include <inviwo/core/metadata/metadata.h>                          // for BoolMetaData
#include <inviwo/core/properties/constraintbehavior.h>              // for ConstraintBehavior
#include <inviwo/core/properties/property.h>                        // for Property
#include <inviwo/core/properties/minmaxproperty.h>                  // IWYU pragma: keep
#include <inviwo/core/util/glm.h>                                   // for almostEqual
#include <modules/qtwidgets/editablelabelqt.h>                      // for EditableLabelQt
#include <modules/qtwidgets/numberwidget.h>                         // for NumberWidget
#include <modules/qtwidgets/properties/propertysettingswidgetqt.h>  // for MinMaxPropertySetting...
#include <modules/qtwidgets/properties/propertywidgetqt.h>          // for PropertyWidgetQt

#include <cstddef>  // for size_t
#include <memory>   // for unique_ptr

#include <QAction>         // for QAction
#include <QHBoxLayout>     // for QHBoxLayout
#include <QLabel>          // for QLabel
#include <QSignalBlocker>  // for QSignalBlocker
#include <QSizePolicy>     // for QSizePolicy, QSizePol...
#include <QWidget>         // for QWidget
#include <QMenu>           // IWYU pragma: keep

#include <glm/detail/qualifier.hpp>    // for defaultp, tvec2
#include <glm/gtc/type_precision.hpp>  // for i64

class QHBoxLayout;

namespace inviwo {

template <typename BT, typename T>
class OrdinalMinMaxTextPropertyWidgetQt : public PropertyWidgetQt {

public:
    OrdinalMinMaxTextPropertyWidgetQt(MinMaxProperty<T>* property);
    virtual ~OrdinalMinMaxTextPropertyWidgetQt();

    virtual void updateFromProperty() override;
    virtual std::unique_ptr<QMenu> getContextMenu() override;

    using V = glm::tvec2<T, glm::defaultp>;

protected:
    void updateFromMin();
    void updateFromMax();
    void showSettings();

    MinMaxPropertySettingsWidgetQt<T>* settings_;
    EditableLabelQt* label_;
    MinMaxProperty<T>* minMaxProperty_;
    NumberWidget<T>* min_;
    NumberWidget<T>* max_;
};

using DoubleMinMaxTextPropertyWidgetQt = OrdinalMinMaxTextPropertyWidgetQt<double, double>;
using FloatMinMaxTextPropertyWidgetQt = OrdinalMinMaxTextPropertyWidgetQt<double, float>;
using IntMinMaxTextPropertyWidgetQt = OrdinalMinMaxTextPropertyWidgetQt<int, int>;
using IntSizeTMinMaxTextPropertyWidgetQt = OrdinalMinMaxTextPropertyWidgetQt<int, size_t>;
using Int64MinMaxTextPropertyWidgetQt = OrdinalMinMaxTextPropertyWidgetQt<int, glm::i64>;

template <typename BT, typename T>
OrdinalMinMaxTextPropertyWidgetQt<BT, T>::OrdinalMinMaxTextPropertyWidgetQt(
    MinMaxProperty<T>* property)
    : PropertyWidgetQt(property)
    , settings_(nullptr)
    , label_(new EditableLabelQt(this, property_))
    , minMaxProperty_(property)
    , min_{new NumberWidget<T>{NumberWidgetConfig{
          .interaction = NumberWidgetConfig::Interaction::NoDragging,
          .barVisible = false,
      }}}
    , max_{new NumberWidget<T>{NumberWidgetConfig{
          .interaction = NumberWidgetConfig::Interaction::NoDragging,
          .barVisible = false,
      }}} {

    setFocusPolicy(min_->focusPolicy());
    setFocusProxy(min_);

    QHBoxLayout* hLayout = new QHBoxLayout();
    setSpacingAndMargins(hLayout);
    hLayout->addWidget(label_);

    QHBoxLayout* textLayout = new QHBoxLayout();
    QWidget* textWidget = new QWidget();
    textWidget->setLayout(textLayout);
    textLayout->setContentsMargins(0, 0, 0, 0);

    QSizePolicy sp = QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    sp.setHorizontalStretch(3);

    textLayout->addWidget(new QLabel("Min:"));
    textLayout->addWidget(min_);
    min_->setSizePolicy(sp);

    textLayout->addWidget(new QLabel("Max:"));
    textLayout->addWidget(max_);
    max_->setSizePolicy(sp);

    hLayout->addWidget(textWidget);
    setLayout(hLayout);

    QSizePolicy textsp = textWidget->sizePolicy();
    textsp.setHorizontalStretch(3);
    textWidget->setSizePolicy(textsp);

    connect(min_, &NumberWidget<T>::valueChanged, this,
            &OrdinalMinMaxTextPropertyWidgetQt<BT, T>::updateFromMin);
    connect(max_, &NumberWidget<T>::valueChanged, this,
            &OrdinalMinMaxTextPropertyWidgetQt<BT, T>::updateFromMax);

    setFixedHeight(sizeHint().height());
    sp = sizePolicy();
    sp.setVerticalPolicy(QSizePolicy::Fixed);
    setSizePolicy(sp);

    updateFromProperty();
}

template <typename BT, typename T>
OrdinalMinMaxTextPropertyWidgetQt<BT, T>::~OrdinalMinMaxTextPropertyWidgetQt() = default;

template <typename BT, typename T>
void OrdinalMinMaxTextPropertyWidgetQt<BT, T>::updateFromProperty() {
    const V val = minMaxProperty_->get();
    const V range = minMaxProperty_->getRange();
    const T inc = minMaxProperty_->getIncrement();
    const T sep = minMaxProperty_->getMinSeparation();

    QSignalBlocker minBlock(min_);
    QSignalBlocker maxBlock(max_);

    min_->setMinValue(range.x, ConstraintBehavior::Editable);
    min_->setMaxValue(range.y - sep, ConstraintBehavior::Editable);

    max_->setMinValue(range.x + sep, ConstraintBehavior::Editable);
    max_->setMaxValue(range.y, ConstraintBehavior::Editable);

    min_->initValue(val.x);
    max_->initValue(val.y);

    min_->setIncrement(inc);
    max_->setIncrement(inc);
}

template <typename BT, typename T>
void OrdinalMinMaxTextPropertyWidgetQt<BT, T>::updateFromMin() {
    QSignalBlocker minBlock(min_);

    const T min = min_->getValue();
    V value = minMaxProperty_->get();

    // check for modification of range start
    if (!util::almostEqual(min, value.x)) {
        const T sep = minMaxProperty_->getMinSeparation();
        V range = minMaxProperty_->getRange();

        value.x = min;

        // adjust range end position if range start is larger than range end - minSep
        if (value.x > value.y - sep) {
            // offset value.y
            value.y = min + sep;
        }
        if ((value.x < range.x) || (value.y > range.y)) {
            // range adjustment necessary
            if (minMaxProperty_->template getMetaData<BoolMetaData>("autoAdjustRanges", false)) {
                // adjust ranges to fit values
                range.x = glm::min(range.x, value.x);
                range.y = glm::max(range.y, value.y);
            } else {
                // clamp values to range
                value.x = glm::clamp(value.x, range.x, range.y - sep);
                value.y = glm::clamp(value.y, range.x + sep, range.y);
            }
        }

        minMaxProperty_->setInitiatingWidget(this);
        minMaxProperty_->set(value, range, minMaxProperty_->getIncrement(), sep);
        minMaxProperty_->clearInitiatingWidget();

        {
            QSignalBlocker maxBlock(max_);

            const V newVal(minMaxProperty_->get());
            min_->setValue(newVal.x);
            max_->setValue(newVal.y);
        }
    }
}

template <typename BT, typename T>
void OrdinalMinMaxTextPropertyWidgetQt<BT, T>::updateFromMax() {
    QSignalBlocker maxBlock(max_);

    const T max = max_->getValue();
    V value = minMaxProperty_->get();

    // check for modification of range start
    if (!util::almostEqual(max, value.y)) {
        const T sep = minMaxProperty_->getMinSeparation();
        V range = minMaxProperty_->getRange();

        value.y = max;

        // adjust range start position if range end is smaller than range start + minSep
        if (value.y < value.x + sep) {
            // offset value.x
            value.x = max - sep;
        }
        if ((value.x < range.x) || (value.y > range.y)) {
            // range adjustment necessary
            if (minMaxProperty_->template getMetaData<BoolMetaData>("autoAdjustRanges", false)) {
                // adjust ranges to fit values
                range.x = glm::min(range.x, value.x);
                range.y = glm::max(range.y, value.y);
            } else {
                // clamp values to range
                value.x = glm::clamp(value.x, range.x, range.y - sep);
                value.y = glm::clamp(value.y, range.x + sep, range.y);
            }
        }

        minMaxProperty_->setInitiatingWidget(this);
        minMaxProperty_->set(value, range, minMaxProperty_->getIncrement(), sep);
        minMaxProperty_->clearInitiatingWidget();

        {
            QSignalBlocker minBlock(min_);

            const V newVal(minMaxProperty_->get());
            min_->setValue(newVal.x);
            max_->setValue(newVal.y);
        }
    }
}

template <typename BT, typename T>
std::unique_ptr<QMenu> OrdinalMinMaxTextPropertyWidgetQt<BT, T>::getContextMenu() {
    auto menu = PropertyWidgetQt::getContextMenu();

    auto rangeAutoAdjustAction = menu->addAction(tr("&Auto adjust ranges"));
    rangeAutoAdjustAction->setCheckable(true);
    rangeAutoAdjustAction->setChecked(
        minMaxProperty_->template getMetaData<BoolMetaData>("autoAdjustRanges", false));

    connect(rangeAutoAdjustAction, &QAction::toggled, [&](bool toggled) {
        // adjust metadata of the property
        if (toggled) {
            minMaxProperty_->template setMetaData<BoolMetaData>("autoAdjustRanges", true);
        } else {
            // remove metadata entry
            minMaxProperty_->template unsetMetaData<BoolMetaData>("autoAdjustRanges");
        }
    });

    auto settingsAction = menu->addAction(tr("&Property settings..."));
    settingsAction->setToolTip(
        tr("&Open the property settings dialog to adjust min bound, start, end, max bound, "
           "minSepration and increment values"));

    connect(settingsAction, &QAction::triggered, this,
            &OrdinalMinMaxTextPropertyWidgetQt<BT, T>::showSettings);
    settingsAction->setEnabled(!property_->getReadOnly());
    return menu;
}

template <typename BT, typename T>
void OrdinalMinMaxTextPropertyWidgetQt<BT, T>::showSettings() {
    if (!settings_) {
        settings_ = new MinMaxPropertySettingsWidgetQt<T>(minMaxProperty_, this);
    }
    settings_->showWidget();
}

}  // namespace inviwo
