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
#include <inviwo/qt/widgets/editablelabelqt.h>
#include <inviwo/core/properties/property.h>

namespace inviwo {

BoolCompositePropertyWidgetQt::BoolCompositePropertyWidgetQt(BoolCompositeProperty* property)
    : CollapsibleGroupBoxWidgetQt(property, true)
    , boolCompProperty_(property) {
   
    setPropertyOwner(property);
    boolCompProperty_->PropertyOwnerObservable::addObserver(this);
    boolCompProperty_->CompositePropertyObservable::addObserver(this);
}          

void BoolCompositePropertyWidgetQt::updateFromProperty() {
    for (auto& elem : propertyWidgets_) elem->updateFromProperty();    
    setChecked(boolCompProperty_->isChecked());
}

void BoolCompositePropertyWidgetQt::labelDidChange() {
    CollapsibleGroupBoxWidgetQt::labelDidChange();
    property_->setDisplayName(getDisplayName());
}

void BoolCompositePropertyWidgetQt::setCollapsed(bool value) {
    boolCompProperty_->setCollapsed(value);
}
void BoolCompositePropertyWidgetQt::onSetCollapsed(bool value) {
    CollapsibleGroupBoxWidgetQt::setCollapsed(value);
}

void BoolCompositePropertyWidgetQt::initState() {
    CollapsibleGroupBoxWidgetQt::initState();
    CollapsibleGroupBoxWidgetQt::setCollapsed(boolCompProperty_->isCollapsed());

    for (auto& prop : boolCompProperty_->getProperties()) {
        addProperty(prop);
    }
    updateFromProperty();
}

void BoolCompositePropertyWidgetQt::onSetDisplayName(const std::string& displayName) {
    displayName_ = displayName;
    label_->setText(displayName);
}

bool BoolCompositePropertyWidgetQt::isChecked() const {
    return boolCompProperty_->isChecked();
}
void BoolCompositePropertyWidgetQt::setChecked(bool checked) {
    CollapsibleGroupBoxWidgetQt::setChecked(checked);
    if (boolCompProperty_->isChecked() != checked) {
        boolCompProperty_->setChecked(checked);
    }
}

bool BoolCompositePropertyWidgetQt::isCollapsed() const {
    return boolCompProperty_->isCollapsed();
}

} // namespace
