/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#include <inviwo/core/properties/constraintbehavior.h>              // for ConstraintBehavior
#include <inviwo/core/properties/property.h>                        // for Property
#include <inviwo/core/util/glmcomp.h>                               // for glmcomp
#include <inviwo/core/util/glmutils.h>                              // for extent, flat_extent
#include <inviwo/core/util/glmvec.h>                                // for dvec3
#include <inviwo/core/util/stdextensions.h>                         // for make_array
#include <modules/qtwidgets/editablelabelqt.h>                      // for EditableLabelQt
#include <modules/qtwidgets/ordinaleditorwidget.h>                  // for OrdinalEditorWidget
#include <modules/qtwidgets/properties/ordinalspinboxwidget.h>      // for OrdinalSpinBoxWidget
#include <modules/qtwidgets/properties/propertysettingswidgetqt.h>  // for OrdinalLikePropertySe...
#include <modules/qtwidgets/properties/propertywidgetqt.h>          // for PropertyWidgetQt
#include <modules/qtwidgets/sliderwidgetqt.h>                       // for SliderWidgetQt
#include <modules/qtwidgets/numberwidget.h>

#include <array>        // for array
#include <cmath>        // for sin, atan2, cos, M_PI
#include <cstddef>      // for size_t
#include <limits>       // for numeric_limits
#include <memory>       // for unique_ptr
#include <type_traits>  // for remove_reference_t
#include <utility>      // for swap
#include <vector>       // for vector
#include <numbers>

#include <QAction>      // for QAction
#include <QGridLayout>  // for QGridLayout
#include <QHBoxLayout>  // for QHBoxLayout
#include <QLabel>       // for QLabel
#include <QLayoutItem>  // for QLayoutItem
#include <QSize>        // for QSize
#include <QSizePolicy>  // for QSizePolicy, QSizePol...
#include <QWidget>      // for QWidget
#include <QMenu>        // IWYU pragma: keep

#include <glm/geometric.hpp>  // for length

class QHBoxLayout;
namespace inviwo {
template <typename T>
class OrdinalBaseWidget;
template <typename T>
class OrdinalProperty;
template <typename T>
class OrdinalRefProperty;
}  // namespace inviwo

namespace inviwo {

namespace util {

template <typename T>
T spherical(T val) {
    const dvec3 dval{val};
    const dvec3 res{glm::length(dval),
                    std::atan2(std::sqrt(dval[0] * dval[0] + dval[1] * dval[1]), dval[2]),
                    std::atan2(dval[1], dval[0])};
    return static_cast<T>(res);
}

template <typename T>
T euclidean(T val) {
    const dvec3 dval{val};
    const dvec3 res{dval[0] * std::sin(dval[1]) * std::cos(dval[2]),
                    dval[0] * std::sin(dval[1]) * std::sin(dval[2]), dval[0] * std::cos(dval[1])};
    return static_cast<T>(res);
}

}  // namespace util

enum class OrdinalPropertyWidgetQtSemantics { Default, Spherical, SpinBox, SphericalSpinBox, Text };

template <typename Prop, OrdinalPropertyWidgetQtSemantics Sem>
class OrdinalLikePropertyWidgetQt final : public PropertyWidgetQt {
public:
    using T = typename Prop::value_type;
    using BT = typename util::value_type<T>::type;

    explicit OrdinalLikePropertyWidgetQt(Prop* property);
    virtual ~OrdinalLikePropertyWidgetQt() = default;
    virtual void updateFromProperty() override;
    virtual std::unique_ptr<QMenu> getContextMenu() override;

private:
    // Connected to OrdinalEditorWidget::valueChanged()
    void setPropertyValue(size_t);
    void showSettings();

    Prop* ordinal_;
    EditableLabelQt* label_;
    OrdinalLikePropertySettingsWidgetQt<Prop>* settings_;

    std::vector<OrdinalBaseWidget<BT>*> editors_;
};

template <typename T, OrdinalPropertyWidgetQtSemantics Sem>
using OrdinalPropertyWidgetQt = OrdinalLikePropertyWidgetQt<OrdinalProperty<T>, Sem>;

template <typename T, OrdinalPropertyWidgetQtSemantics Sem>
using OrdinalRefPropertyWidgetQt = OrdinalLikePropertyWidgetQt<OrdinalRefProperty<T>, Sem>;

template <typename Prop, OrdinalPropertyWidgetQtSemantics Sem>
OrdinalLikePropertyWidgetQt<Prop, Sem>::OrdinalLikePropertyWidgetQt(Prop* property)
    : PropertyWidgetQt(property)
    , ordinal_(property)
    , label_{new EditableLabelQt(this, property)}
    , settings_(nullptr) {

    auto* hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(getSpacing());
    hLayout->addWidget(label_);

    auto* centralWidget = new QWidget();
    auto policy = centralWidget->sizePolicy();
    policy.setHorizontalStretch(3);
    centralWidget->setSizePolicy(policy);

    auto* gridLayout = new QGridLayout();
    centralWidget->setLayout(gridLayout);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->setSpacing(0);

    auto factory = [this](size_t row, size_t col) {
        auto editor = [&]() {
            if constexpr (Sem == OrdinalPropertyWidgetQtSemantics::SpinBox) {
                return new OrdinalSpinBoxWidget<BT>();
            } else if constexpr (Sem == OrdinalPropertyWidgetQtSemantics::SphericalSpinBox) {
                auto w = new OrdinalSpinBoxWidget<BT>();
                if (col > 0) w->setWrapping(true);
                return w;
            } else if constexpr (Sem == OrdinalPropertyWidgetQtSemantics::Text) {
                return new NumberWidget<BT>();
            } else if constexpr (Sem == OrdinalPropertyWidgetQtSemantics::Spherical) {
                auto w = new NumberWidget<BT>();
                if (col > 0) w->setWrapping(true);
                return w;
            } else {
                return new NumberWidget<BT>();
            }
        }();

        editors_.push_back(editor);

        connect(editor, &std::remove_reference_t<decltype(*editor)>::valueChanged, this,
                [this, index = col + row * util::extent<T, 0>::value]() {
                    this->setPropertyValue(index);
                });

        auto sp = editor->sizePolicy();
        sp.setHorizontalPolicy(QSizePolicy::Expanding);
        editor->setSizePolicy(sp);

        if constexpr ((Sem == OrdinalPropertyWidgetQtSemantics::Default ||
                       Sem == OrdinalPropertyWidgetQtSemantics::Text) &&
                      util::extent<T, 0>::value > 1 && util::extent<T, 1>::value == 1) {
            constexpr std::array<const char*, 4> labels{"x", "y", "z", "w"};
            editor->setPrefix(labels[col]);
            return editor;
        } else if constexpr (Sem == OrdinalPropertyWidgetQtSemantics::Spherical) {
            constexpr std::array<const char*, 3> sphericalLabels{"r", R"(θ)", R"(φ)"};
            editor->setPrefix(sphericalLabels[col]);
            return editor;
        } else if constexpr (Sem == OrdinalPropertyWidgetQtSemantics::SphericalSpinBox) {
            constexpr std::array<const char*, 3> sphericalLabels{"r", "<html>&theta;</html>",
                                                                 "<html>&phi;</html>"};
            auto* widget = new QWidget(this);
            auto* layout = new QHBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(7);
            widget->setLayout(layout);
            layout->addWidget(new QLabel(sphericalLabels[col], this));
            layout->addWidget(editor);

            widget->setFocusPolicy(editor->focusPolicy());
            widget->setFocusProxy(editor);
            return widget;
        } else {
            return editor;
        }
    };

    for (size_t row = 0; row < util::extent<T, 1>::value; row++) {
        for (size_t col = 0; col < util::extent<T, 0>::value; col++) {
            auto editor = factory(row, col);

            auto layoutCol = col;
            auto layoutRow = row;

            // vectors should be drawn in row major while matrices are column major
            if constexpr (util::extent<T, 1>::value > 1 &&
                          Sem != OrdinalPropertyWidgetQtSemantics::Default) {
                std::swap(layoutCol, layoutRow);
            }
            gridLayout->addWidget(editor, static_cast<int>(layoutRow), static_cast<int>(layoutCol));
        }
    }

    if ((gridLayout->count() > 0) && gridLayout->itemAt(0)->widget()) {
        setFocusPolicy(gridLayout->itemAt(0)->widget()->focusPolicy());
        setFocusProxy(gridLayout->itemAt(0)->widget());
    }

    hLayout->addWidget(centralWidget);

    centralWidget->setMinimumHeight(centralWidget->sizeHint().height());
    auto sp = centralWidget->sizePolicy();
    sp.setVerticalPolicy(QSizePolicy::Fixed);
    centralWidget->setSizePolicy(sp);

    setLayout(hLayout);

    setFixedHeight(sizeHint().height());
    sp = sizePolicy();
    sp.setVerticalPolicy(QSizePolicy::Fixed);
    setSizePolicy(sp);

    updateFromProperty();
}

template <typename Prop, OrdinalPropertyWidgetQtSemantics Sem>
void OrdinalLikePropertyWidgetQt<Prop, Sem>::updateFromProperty() {
    T min = ordinal_->getMinValue();
    T max = ordinal_->getMaxValue();
    T inc = ordinal_->getIncrement();
    T val = ordinal_->get();

    constexpr size_t nelem = util::flat_extent<T>::value;
    auto mincb =
        util::make_array<nelem>([&](auto) { return ordinal_->getMinConstraintBehaviour(); });
    auto maxcb =
        util::make_array<nelem>([&](auto) { return ordinal_->getMaxConstraintBehaviour(); });

    if constexpr (Sem == OrdinalPropertyWidgetQtSemantics::SphericalSpinBox ||
                  Sem == OrdinalPropertyWidgetQtSemantics::Spherical) {
        val = util::spherical(val);
        min = T{std::numeric_limits<BT>::epsilon(), 0, -std::numbers::pi};
        max = T{3 * glm::length(max), std::numbers::pi, std::numbers::pi};
        inc = T{inc.x, std::numbers::pi / 360.0, std::numbers::pi / 360.0};

        mincb[1] = ConstraintBehavior::Immutable;
        mincb[2] = ConstraintBehavior::Immutable;
        maxcb[1] = ConstraintBehavior::Immutable;
        maxcb[2] = ConstraintBehavior::Immutable;
    }

    for (size_t i = 0; i < nelem; i++) {
        editors_[i]->setMinValue(util::glmcomp(min, i), mincb[i]);
        editors_[i]->setMaxValue(util::glmcomp(max, i), maxcb[i]);
        editors_[i]->setIncrement(util::glmcomp(inc, i));
        editors_[i]->initValue(util::glmcomp(val, i));
    }
}

template <typename Prop, OrdinalPropertyWidgetQtSemantics Sem>
void OrdinalLikePropertyWidgetQt<Prop, Sem>::setPropertyValue(size_t editorId) {
    T val = ordinal_->get();

    if constexpr (Sem == OrdinalPropertyWidgetQtSemantics::SphericalSpinBox ||
                  Sem == OrdinalPropertyWidgetQtSemantics::Spherical) {
        val = util::spherical(val);

        if (editorId == 1) {
            // necessary to prevent ambiguous cases at the poles where phi is set to 0 or phi gets
            // flipped
            util::glmcomp(val, 2) = editors_[2]->getValue();
        }
    }

    util::glmcomp(val, editorId) = editors_[editorId]->getValue();

    if constexpr (Sem == OrdinalPropertyWidgetQtSemantics::SphericalSpinBox ||
                  Sem == OrdinalPropertyWidgetQtSemantics::Spherical) {
        val = util::euclidean(val);
    }

    ordinal_->setInitiatingWidget(this);
    ordinal_->set(val);
    ordinal_->clearInitiatingWidget();
}

template <typename Prop, OrdinalPropertyWidgetQtSemantics Sem>
void OrdinalLikePropertyWidgetQt<Prop, Sem>::showSettings() {
    if (!settings_) {
        settings_ = new OrdinalLikePropertySettingsWidgetQt<Prop>(ordinal_, this);
    }
    settings_->showWidget();
}

template <typename Prop, OrdinalPropertyWidgetQtSemantics Sem>
std::unique_ptr<QMenu> OrdinalLikePropertyWidgetQt<Prop, Sem>::getContextMenu() {
    auto menu = PropertyWidgetQt::getContextMenu();

    auto settingsAction = menu->addAction(tr("&Property settings..."));
    settingsAction->setToolTip(
        tr("&Open the property settings dialog to adjust min, max, and increment values"));

    connect(settingsAction, &QAction::triggered, this,
            &OrdinalLikePropertyWidgetQt<Prop, Sem>::showSettings);

    settingsAction->setEnabled(!property_->getReadOnly());

    return menu;
}

}  // namespace inviwo
