/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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
#include <modules/qtwidgets/properties/compositepropertywidgetqt.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QHBoxLayout>
#include <QtCore/qmath.h>
#include <QSpinBox>
#include <QSignalBlocker>
#include <warn/pop>

namespace inviwo {

LightPropertyWidgetQt::LightPropertyWidgetQt(FloatVec3Property* property)
    : PropertyWidgetQt(property)
    , property_(property)
    , lightWidget_{new LightPositionWidgetQt()}
    , radiusSpinBox_{new CustomDoubleSpinBoxQt(this)}
    , label_{new EditableLabelQt(this, property_)} {

    setFocusPolicy(radiusSpinBox_->focusPolicy());
    setFocusProxy(radiusSpinBox_);

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(7);

    connect(lightWidget_, &LightPositionWidgetQt::positionChanged, this,
            &LightPropertyWidgetQt::onPositionLightWidgetChanged);

    radiusSpinBox_->setSingleStep(0.1);
    // don't emit the valueChanged() signal while typing
    radiusSpinBox_->setKeyboardTracking(false);
    connect(
        radiusSpinBox_,
        static_cast<void (CustomDoubleSpinBoxQt::*)(double)>(&CustomDoubleSpinBoxQt::valueChanged),
        this, &LightPropertyWidgetQt::onRadiusSpinBoxChanged);

    // Assuming that minimum value is negative and maximum value is positive
    if (glm::any(glm::greaterThan(property_->getMinValue(), vec3(0.0f)))) {
        LogWarn("Minimum value is assumed to be negative. Widget may produce values out of range.")
    }
    if (glm::any(glm::lessThan(property_->getMaxValue(), vec3(0.0f)))) {
        LogWarn("Maximum value is assumed to be positive. Widget may produce values out of range.")
    }

    vec3 maxVal = glm::abs(property_->getMaxValue());
    radiusSpinBox_->setMinimum(-glm::length(maxVal));
    radiusSpinBox_->setMaximum(glm::length(maxVal));

    QWidget* groupBox = new QWidget(this);
    QGridLayout* layout = new QGridLayout();
    groupBox->setLayout(layout);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(3);
    layout->addWidget(new QLabel("Direction", this), 0, 0);
    layout->addWidget(lightWidget_, 0, 1);
    layout->addWidget(new QLabel("Distance", this), 1, 0);
    layout->addWidget(radiusSpinBox_, 1, 1);

    hLayout->addWidget(label_);
    hLayout->addWidget(groupBox);

    setLayout(hLayout);

    updateFromProperty();
}

LightPropertyWidgetQt::~LightPropertyWidgetQt() = default;

void LightPropertyWidgetQt::onPositionLightWidgetChanged() {
    property_->setInitiatingWidget(this);
    property_->set(lightWidget_->getPosition());
    property_->clearInitiatingWidget();
}

void LightPropertyWidgetQt::onRadiusSpinBoxChanged(double radius) {
    lightWidget_->setRadius(static_cast<float>(radius));
}

void LightPropertyWidgetQt::updateFromProperty() {
    // Prevent widgets from signaling changes just after setting them
    QSignalBlocker lblocker(lightWidget_);
    QSignalBlocker rblocker(radiusSpinBox_);

    float r = glm::length(property_->get());
    r *= property_->get().z < 0.0f ? -1.0f : 1.0f;
    if (radiusSpinBox_->value() != r) {
        radiusSpinBox_->setValue(r);
    }

    if (lightWidget_->getPosition() != property_->get()) {
        lightWidget_->setPosition(property_->get());
    }
}

} // namespace