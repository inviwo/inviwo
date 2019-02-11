/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <modules/userinterfacegl/glui/widgets/floatpropertywidget.h>

namespace inviwo {

namespace glui {

const std::string FloatPropertyWidget::classIdentifier = "org.inviwo.glui.FloatPropertyWidget";
std::string FloatPropertyWidget::getClassIdentifier() const { return classIdentifier; }

FloatPropertyWidget::FloatPropertyWidget(FloatProperty &property, Processor &processor,
                                         Renderer &uiRenderer, const ivec2 &extent,
                                         UIOrientation orientation)
    : Slider(property.getDisplayName(), 0, 0, 100, processor, uiRenderer, extent, orientation)
    , PropertyWidget(&property)
    , sliderMax_(10000)
    , property_(&property) {

    property_->addObserver(this);

    moveAction_ = [this](const dvec2 &delta) {
        bool triggerUpdate = false;
        if (!property_->getReadOnly()) {
            // delta in pixel (screen coords),
            // need to scale from graphical representation to slider
            float newVal = sliderToRepr(glm::clamp(
                static_cast<int>(round(getPreviousValue() +
                                       convertDeltaToSlider(delta) *
                                           static_cast<double>(getMaxValue() - getMinValue()))),
                getMinValue(), getMaxValue()));
            if (std::fabs(newVal - property_->get()) > glm::epsilon<float>()) {
                property_->set(newVal);
                triggerUpdate = true;
            }
        }
        return triggerUpdate;
    };
    updateFromProperty();
}

void FloatPropertyWidget::updateFromProperty() {
    set(reprToSlider(property_->get()), 0, sliderMax_);
    setEnabled(!property_->getReadOnly());
}

void FloatPropertyWidget::onSetVisible(Property *, bool visible) { setVisible(visible); }

void FloatPropertyWidget::onSetDisplayName(Property *, const std::string &displayName) {
    setLabel(displayName);
    property_->propertyModified();
}

void FloatPropertyWidget::onSetReadOnly(Property *, bool readonly) { setEnabled(!readonly); }

float FloatPropertyWidget::sliderToRepr(int val) const {
    return property_->getMinValue() +
           (static_cast<float>(val) * (property_->getMaxValue() - property_->getMinValue()) /
            static_cast<float>(sliderMax_));
}
int FloatPropertyWidget::reprToSlider(float val) const {
    return static_cast<int>((val - property_->getMinValue()) /
                            (property_->getMaxValue() - property_->getMinValue()) * sliderMax_);
}

}  // namespace glui

}  // namespace inviwo
