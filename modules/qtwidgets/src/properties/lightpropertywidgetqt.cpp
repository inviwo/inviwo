/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2023 Inviwo Foundation
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

#include <modules/qtwidgets/properties/lightpropertywidgetqt.h>

#include <inviwo/core/properties/constraintbehavior.h>              // for ConstraintBehavior
#include <inviwo/core/properties/ordinalproperty.h>                 // for FloatVec3Property
#include <inviwo/core/util/glmvec.h>                                // for vec3, dvec3
#include <inviwo/core/util/logcentral.h>                            // for LogCentral, LogWarn
#include <inviwo/core/util/zip.h>                                   // for enumerate, zipIterator
#include <modules/qtwidgets/editablelabelqt.h>                      // for EditableLabelQt
#include <modules/qtwidgets/lightpositionwidgetqt.h>                // for LightPositionWidgetQt
#include <modules/qtwidgets/properties/ordinalspinboxwidget.h>      // for OrdinalSpinBoxWidget
#include <modules/qtwidgets/properties/propertysettingswidgetqt.h>  // for OrdinalLikePropertySe...
#include <modules/qtwidgets/properties/propertywidgetqt.h>          // for PropertyWidgetQt

#include <cstdlib>  // for abs
#include <cmath>    // for abs
#include <sstream>  // for basic_stringbuf<>::in...

#include <QAction>                    // for QAction
#include <QCheckBox>                  // for QCheckBox
#include <QGridLayout>                // for QGridLayout
#include <QHBoxLayout>                // for QHBoxLayout
#include <QLabel>                     // for QLabel
#include <QMenu>                      // for QMenu
#include <QSignalBlocker>             // for QSignalBlocker
#include <QSizePolicy>                // for QSizePolicy
#include <QWidget>                    // for QWidget
#include <Qt>                         // for operator|, AlignTop
#include <fmt/core.h>                 // for format, basic_string_...
#include <glm/geometric.hpp>          // for length
#include <glm/vec3.hpp>               // for vec, operator*, opera...
#include <glm/vector_relational.hpp>  // for any, greaterThan, les...

class QHBoxLayout;

namespace inviwo {

LightPropertyWidgetQt::LightPropertyWidgetQt(FloatVec3Property* property)
    : PropertyWidgetQt(property)
    , property_(property)
    , halfSphere_{new LightPositionWidgetQt()}
    , radius_{new OrdinalSpinBoxWidget<float>()}
    , inFront_{new QCheckBox("In front:", this)}
    , cartesian_{new OrdinalSpinBoxWidget<float>(), new OrdinalSpinBoxWidget<float>(),
                 new OrdinalSpinBoxWidget<float>()}
    , label_{new EditableLabelQt(this, property_)}
    , settings_(nullptr) {

    setFocusPolicy(radius_->focusPolicy());
    setFocusProxy(radius_);

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(7);

    connect(halfSphere_, &LightPositionWidgetQt::positionChanged, this, [this]() {
        const auto pos = static_cast<vec3>(halfSphere_->getPosition());
        property_->setInitiatingWidget(this);
        property_->set(pos);
        property_->clearInitiatingWidget();

        QSignalBlocker s0{cartesian_[0]};
        QSignalBlocker s1{cartesian_[1]};
        QSignalBlocker s2{cartesian_[2]};

        cartesian_[0]->setValue(pos.x);
        cartesian_[1]->setValue(pos.y);
        cartesian_[2]->setValue(pos.z);
    });

    radius_->setIncrement(0.1f);
    connect(radius_, &OrdinalSpinBoxWidget<float>::valueChanged, this, [this]() {
        halfSphere_->setRadius((inFront_->isChecked() ? 1.0 : -1.0) * radius_->getValue());
    });

    connect(inFront_, &QCheckBox::toggled, this, [this](bool front) {
        halfSphere_->setRadius((front ? 1.0 : -1.0) * std::abs(halfSphere_->getRadius()));
    });

    for (auto&& [i, coordinate] : util::enumerate(cartesian_)) {
        connect(coordinate, &OrdinalSpinBoxWidget<float>::valueChanged, this, [this, i = i]() {
            auto newPos = halfSphere_->getPosition();
            newPos[i] = cartesian_[i]->getValue();

            QSignalBlocker lblocker(halfSphere_);
            QSignalBlocker rblocker(radius_);
            QSignalBlocker fblocker(inFront_);

            inFront_->setChecked(newPos.z > 0);

            const auto r = glm::length(newPos);
            radius_->setValue(static_cast<float>(r));

            if (halfSphere_->getPosition() != newPos) {
                halfSphere_->setPosition(newPos);
            }

            property_->setInitiatingWidget(this);
            property_->set(static_cast<vec3>(newPos));
            property_->clearInitiatingWidget();
        });
    }

    // Assuming that minimum value is negative and maximum value is positive
    if (glm::any(glm::greaterThan(property_->getMinValue(), vec3(0.0f)))) {
        LogWarn("Minimum value is assumed to be negative. Widget may produce values out of range.")
    }
    if (glm::any(glm::lessThan(property_->getMaxValue(), vec3(0.0f)))) {
        LogWarn("Maximum value is assumed to be positive. Widget may produce values out of range.")
    }

    radius_->setMinValue(0, ConstraintBehavior::Immutable);
    radius_->setMaxValue(glm::length(property_->getMaxValue()),
                         property_->getMaxConstraintBehaviour());

    QWidget* groupBox = new QWidget(this);
    auto sp = groupBox->sizePolicy();
    sp.setHorizontalStretch(3);
    groupBox->setSizePolicy(sp);

    QGridLayout* layout = new QGridLayout();
    groupBox->setLayout(layout);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(3);
    layout->addWidget(inFront_, 0, 0, 1, 1, Qt::AlignTop | Qt::AlignLeft);
    layout->addWidget(new QLabel("Distance:", this), 1, 0, 1, 1, Qt::AlignBottom);
    layout->addWidget(radius_, 2, 0, 1, 1, Qt::AlignTop);

    layout->addWidget(halfSphere_, 0, 1, 3, 1);

    layout->addWidget(cartesian_[0], 0, 2);
    layout->addWidget(cartesian_[1], 1, 2);
    layout->addWidget(cartesian_[2], 2, 2);

    hLayout->addWidget(label_);
    hLayout->addWidget(groupBox);

    setLayout(hLayout);

    updateFromProperty();
}

LightPropertyWidgetQt::~LightPropertyWidgetQt() = default;

void LightPropertyWidgetQt::updateFromProperty() {
    // Prevent widgets from signaling changes just after setting them
    QSignalBlocker lblocker(halfSphere_);
    QSignalBlocker rblocker(radius_);
    QSignalBlocker fblocker(inFront_);

    const auto newPos = dvec3{property_->get()};

    for (auto&& [i, coordinate] : util::enumerate(cartesian_)) {
        QSignalBlocker block(coordinate);
        coordinate->setMinValue(property_->getMinValue()[i],
                                property_->getMinConstraintBehaviour());
        coordinate->setMaxValue(property_->getMaxValue()[i],
                                property_->getMaxConstraintBehaviour());
        coordinate->initValue(property_->get()[i]);
        coordinate->setIncrement(property_->getIncrement()[i]);
    }
    inFront_->setChecked(newPos.z > 0);

    const auto r = glm::length(newPos);
    radius_->setValue(static_cast<float>(r));

    if (halfSphere_->getPosition() != newPos) {
        halfSphere_->setPosition(newPos);
    }
}

std::unique_ptr<QMenu> LightPropertyWidgetQt::getContextMenu() {
    auto menu = PropertyWidgetQt::getContextMenu();
    auto settingsAction = menu->addAction(tr("&Property settings..."));
    settingsAction->setToolTip(
        tr("&Open the property settings dialog to adjust min bound, start, end, max bound, "
           "minSepration and increment values"));

    connect(settingsAction, &QAction::triggered, this, [this]() {
        if (!settings_) {
            settings_ = new OrdinalLikePropertySettingsWidgetQt<FloatVec3Property>(property_, this);
        }
        settings_->showWidget();
    });
    settingsAction->setEnabled(!property_->getReadOnly());
    return menu;
}

}  // namespace inviwo
