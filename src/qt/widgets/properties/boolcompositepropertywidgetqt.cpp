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

#include <inviwo/qt/widgets/properties/boolcompositepropertywidgetqt.h>
#include <inviwo/core/properties/boolcompositeproperty.h>

#include <inviwo/core/properties/property.h>

namespace inviwo {

BoolCompositePropertyWidgetQt::BoolCompositePropertyWidgetQt(BoolCompositeProperty* property)
    : CollapsibleGroupBoxWidgetQt(property->getDisplayName(), true)
    , property_(property) {
    setPropertyOwner(property);
    PropertyWidget::setProperty(property);
    std::vector<Property*> subProperties = property_->getProperties();
    for (auto& subPropertie : subProperties) {
        addProperty(subPropertie);
    }

    property->addObserver(this);
    updateFromProperty();
}                           

void BoolCompositePropertyWidgetQt::updateFromProperty() {
    for (auto& elem : propertyWidgets_) elem->updateFromProperty();
    this->setDisabled(property_->getReadOnly());
    
    setChecked(property_->isChecked());
    setCollapsed(property_->isCollapsed());
}

void BoolCompositePropertyWidgetQt::labelDidChange() {
    CollapsibleGroupBoxWidgetQt::labelDidChange();
    property_->setDisplayName(getDisplayName());
}

void BoolCompositePropertyWidgetQt::setDeveloperUsageMode(bool value) {
    CollapsibleGroupBoxWidgetQt::setDeveloperUsageMode(value);
    property_->setUsageMode(DEVELOPMENT);
}

void BoolCompositePropertyWidgetQt::setApplicationUsageMode(bool value) {
    CollapsibleGroupBoxWidgetQt::setApplicationUsageMode(value);
    property_->setUsageMode(APPLICATION);
}

bool BoolCompositePropertyWidgetQt::isChecked() const {
    return property_->isChecked();
}
void BoolCompositePropertyWidgetQt::setChecked(bool checked) {
    CollapsibleGroupBoxWidgetQt::setChecked(checked);
    if (property_->isChecked() != checked) {
        property_->setChecked(checked);
    }
}

bool BoolCompositePropertyWidgetQt::isCollapsed() const {
    return property_->isCollapsed();
}
void BoolCompositePropertyWidgetQt::setCollapsed(bool value) {
    CollapsibleGroupBoxWidgetQt::setCollapsed(value);
    if (property_->isCollapsed() != value) {
        property_->setCollapsed(value);
    }
}

} // namespace
