/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2016 Inviwo Foundation
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

#include <inviwo/core/properties/propertywidget.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/util/assertion.h>

namespace inviwo {

PropertyWidget::PropertyWidget() = default;

PropertyWidget::PropertyWidget(Property* property) : property_(property) {}

Property* PropertyWidget::getProperty() { return property_; }

PropertyEditorWidget* PropertyWidget::getEditorWidget() const { return nullptr; }

bool PropertyWidget::hasEditorWidget() const { return false; }

//////////////////////////////////////////////////////////////////////////

// Additional widgets owned by property

PropertyEditorWidget::PropertyEditorWidget(Property* property) : property_(property), metaData_(nullptr) {
    ivwAssert(property != nullptr, "property must not be null");
    metaData_ = property->createMetaData<PropertyEditorWidgetMetaData>(
        PropertyEditorWidgetMetaData::CLASS_IDENTIFIER);
}

PropertyEditorWidget::~PropertyEditorWidget() = default;

void PropertyEditorWidget::setVisibility(bool visible) { updateVisibility(visible); }


void PropertyEditorWidget::setDimensions(const ivec2& dimensions) {
    updateDimensions(dimensions);
}

void PropertyEditorWidget::setPosition(const ivec2& pos) { updatePosition(pos); }

void PropertyEditorWidget::setDockStatus(PropertyEditorWidgetDockStatus dockStatus) {
    updateDockStatus(dockStatus);
}

void PropertyEditorWidget::setSticky(bool sticky) {
    updateSticky(sticky);
}

bool PropertyEditorWidget::isVisible() const { return metaData_->isVisible(); }

ivec2 PropertyEditorWidget::getPosition() const {
    return metaData_->getWidgetPosition();
}

ivec2 PropertyEditorWidget::getDimensions() const {
    return metaData_->getDimensions();
}

PropertyEditorWidgetDockStatus PropertyEditorWidget::getDockStatus() const {
    return metaData_->getDockStatus();
}

bool PropertyEditorWidget::isSticky() const {
    return metaData_->isSticky();
}

void PropertyEditorWidget::updateVisibility(bool visible) { metaData_->setVisible(visible); }


void PropertyEditorWidget::updateDimensions(const ivec2& dimensions) {
    metaData_->setDimensions(dimensions);
}

void PropertyEditorWidget::updatePosition(const ivec2& pos) { metaData_->setWidgetPosition(pos); }

void PropertyEditorWidget::updateDockStatus(PropertyEditorWidgetDockStatus dockStatus) {
    metaData_->setDockStatus(dockStatus);
}

void PropertyEditorWidget::updateSticky(bool sticky) {
    metaData_->setSticky(sticky);
}


}  // namespace
