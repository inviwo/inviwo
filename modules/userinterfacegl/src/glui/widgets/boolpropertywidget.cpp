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

#include <modules/userinterfacegl/glui/widgets/boolpropertywidget.h>

namespace inviwo {

namespace glui {

const std::string BoolPropertyWidget::classIdentifier = "org.inviwo.glui.BoolPropertyWidget";
std::string BoolPropertyWidget::getClassIdentifier() const { return classIdentifier; }

BoolPropertyWidget::BoolPropertyWidget(BoolProperty &property, Processor &processor,
                                       Renderer &uiRenderer, const ivec2 &extent)
    : CheckBox(property.getDisplayName(), processor, uiRenderer, extent)
    , PropertyWidget(&property)
    , property_(&property) {
    property_->addObserver(this);
    action_ = [&]() {
        if (!property_->getReadOnly()) {
            property_->set(getValue());
        }
    };
    updateFromProperty();
}

void BoolPropertyWidget::updateFromProperty() {
    setValue(property_->get());
    setEnabled(!property_->getReadOnly());
}

void BoolPropertyWidget::onSetVisible(Property *, bool visible) { setVisible(visible); }

void BoolPropertyWidget::onSetDisplayName(Property *, const std::string &displayName) {
    setLabel(displayName);
}

void BoolPropertyWidget::onSetReadOnly(Property *, bool readonly) { setEnabled(!readonly); }

}  // namespace glui

}  // namespace inviwo
