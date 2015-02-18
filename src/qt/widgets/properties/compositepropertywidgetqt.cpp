/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <inviwo/qt/widgets/properties/compositepropertywidgetqt.h>

#include <inviwo/core/properties/property.h>

namespace inviwo {

CompositePropertyWidgetQt::CompositePropertyWidgetQt(CompositeProperty* property)
    : CollapsibleGroupBoxWidgetQt(property->getDisplayName())
    , property_(property) {
    setPropertyOwner(property);
    PropertyWidget::setProperty(property);
    std::vector<Property*> subProperties = property_->getProperties();
    for (size_t i = 0; i < subProperties.size(); i++) {
        addProperty(subProperties[i]);
    }

    property->addObserver(this);
    updateFromProperty();
}                           

void CompositePropertyWidgetQt::updateFromProperty() {
    for (size_t i = 0; i < propertyWidgets_.size(); i++)
        propertyWidgets_[i]->updateFromProperty();
    this->setDisabled(property_->getReadOnly());
    
    setCollapsed(property_->isCollapsed());
}

void CompositePropertyWidgetQt::labelDidChange() {
    CollapsibleGroupBoxWidgetQt::labelDidChange();
    property_->setDisplayName(getDisplayName());
}

void CompositePropertyWidgetQt::setDeveloperUsageMode(bool value) {
    CollapsibleGroupBoxWidgetQt::setDeveloperUsageMode(value);
    property_->setUsageMode(DEVELOPMENT);
}

void CompositePropertyWidgetQt::setApplicationUsageMode(bool value) {
    CollapsibleGroupBoxWidgetQt::setApplicationUsageMode(value);
    property_->setUsageMode(APPLICATION);
}

bool CompositePropertyWidgetQt::isCollapsed() const {
    return property_->isCollapsed();
}
void CompositePropertyWidgetQt::setCollapsed(bool value) {
    CollapsibleGroupBoxWidgetQt::setCollapsed(value);
    if (property_->isCollapsed() != value) {
        property_->setCollapsed(value);
    }
}

} // namespace
