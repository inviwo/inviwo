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

#include <modules/userinterfacegl/glui/widgets/buttonpropertywidget.h>

namespace inviwo {

namespace glui {

const std::string ButtonPropertyWidget::classIdentifier = "org.inviwo.glui.ButtonPropertyWidget";
std::string ButtonPropertyWidget::getClassIdentifier() const { return classIdentifier; }

ButtonPropertyWidget::ButtonPropertyWidget(ButtonProperty &property, Processor &processor,
                                           Renderer &uiRenderer, const ivec2 &extent)
    : Button(property.getDisplayName(), processor, uiRenderer, extent)
    , PropertyWidget(&property)
    , property_(&property) {
    property_->addObserver(this);
    property_->registerWidget(this);
    action_ = [&]() {
        if (!property_->getReadOnly()) {
            property_->pressButton();
        }
    };
    updateFromProperty();
}

void ButtonPropertyWidget::updateFromProperty() {
    setLabel(property_->getDisplayName());
    setEnabled(!property_->getReadOnly());
}

void ButtonPropertyWidget::onSetVisible(Property *, bool visible) { setVisible(visible); }

void ButtonPropertyWidget::onSetDisplayName(Property *, const std::string &displayName) {
    setLabel(displayName);
    property_->propertyModified();
}

void ButtonPropertyWidget::onSetReadOnly(Property *, bool readonly) { setEnabled(!readonly); }

ToolButtonPropertyWidget::ToolButtonPropertyWidget(const std::string &imageFileName,
                                                   ButtonProperty &property, Processor &processor,
                                                   Renderer &uiRenderer, const ivec2 &extent)
    : ToolButton(imageFileName, processor, uiRenderer, extent)
    , PropertyWidget(&property)
    , property_(&property) {
    property_->addObserver(this);
    property_->registerWidget(this);
    action_ = [&]() {
        if (!property_->getReadOnly()) {
            property_->pressButton();
        }
    };
    updateFromProperty();
}

ToolButtonPropertyWidget::ToolButtonPropertyWidget(ButtonProperty &property,
                                                   std::shared_ptr<Texture2D> image,
                                                   Processor &processor, Renderer &uiRenderer,
                                                   const ivec2 &extent)
    : ToolButton(image, processor, uiRenderer, extent)
    , PropertyWidget(&property)
    , property_(&property) {
    property_->addObserver(this);
    property_->registerWidget(this);
    action_ = [&]() {
        if (!property_->getReadOnly()) {
            property_->pressButton();
        }
    };
    updateFromProperty();
}

void ToolButtonPropertyWidget::updateFromProperty() {
    setLabel(property_->getDisplayName());
    setEnabled(!property_->getReadOnly());
}

void ToolButtonPropertyWidget::onSetVisible(Property *, bool visible) { setVisible(visible); }

void ToolButtonPropertyWidget::onSetDisplayName(Property *, const std::string &displayName) {
    setLabel(displayName);
    property_->propertyModified();
}

void ToolButtonPropertyWidget::onSetReadOnly(Property *, bool readonly) { setEnabled(!readonly); }

}  // namespace glui

}  // namespace inviwo
