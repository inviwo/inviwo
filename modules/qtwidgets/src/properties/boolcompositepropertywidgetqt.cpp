/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2022 Inviwo Foundation
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

#include <modules/qtwidgets/properties/boolcompositepropertywidgetqt.h>

#include <inviwo/core/properties/boolcompositeproperty.h>              // for BoolCompositeProperty
#include <inviwo/core/properties/boolproperty.h>                       // for BoolProperty
#include <inviwo/core/properties/compositepropertyobserver.h>          // for CompositePropertyO...
#include <inviwo/core/properties/propertyobserver.h>                   // for PropertyObserverDe...
#include <inviwo/core/properties/propertyownerobserver.h>              // for PropertyOwnerObser...
#include <modules/qtwidgets/editablelabelqt.h>                         // for EditableLabelQt
#include <modules/qtwidgets/properties/collapsiblegroupboxwidgetqt.h>  // for CollapsibleGroupBo...

#include <functional>                                                  // for __base, function
#include <vector>                                                      // for vector

namespace inviwo {

class Property;

BoolCompositePropertyWidgetQt::BoolCompositePropertyWidgetQt(BoolCompositeProperty* property)
    : CollapsibleGroupBoxWidgetQt(property, true)
    , boolCompProperty_(property)
    , boolObserverDelegate_{} {

    boolObserverDelegate_.onDisplayNameChange = [this](Property*, const std::string& name) {
        setCheckBoxText(name);
    };
    boolObserverDelegate_.onVisibleChange = [this](Property*, bool visible) {
        setCheckBoxVisible(visible);
    };
    boolObserverDelegate_.onReadOnlyChange = [this](Property*, bool readonly) {
        setCheckBoxReadonly(readonly);
    };
    setCheckBoxText(boolCompProperty_->getBoolProperty()->getDisplayName());
    setCheckBoxVisible(boolCompProperty_->getBoolProperty()->getVisible());
    setCheckBoxReadonly(boolCompProperty_->getBoolProperty()->getReadOnly());
    boolCompProperty_->getBoolProperty()->addObserver(&boolObserverDelegate_);

    setPropertyOwner(property);
    boolCompProperty_->PropertyOwnerObservable::addObserver(this);
    boolCompProperty_->CompositePropertyObservable::addObserver(this);
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
    CollapsibleGroupBoxWidgetQt::setChecked(boolCompProperty_->getBoolProperty()->get());

    for (auto& prop : boolCompProperty_->getProperties()) {
        if (prop != boolCompProperty_->getBoolProperty()) {
            addProperty(prop);
        }
    }
    updateFromProperty();
}

// hack since we don't add the first special "bool" property
void BoolCompositePropertyWidgetQt::onDidAddProperty(Property* property, size_t index) {
    if (index > 0) {
        CollapsibleGroupBoxWidgetQt::onDidAddProperty(property, --index);
    }
}

// hack since we don't add the first special "bool" property
void BoolCompositePropertyWidgetQt::onWillRemoveProperty(Property* property, size_t index) {
    if (index > 0) {
        CollapsibleGroupBoxWidgetQt::onWillRemoveProperty(property, --index);
    }
}

void BoolCompositePropertyWidgetQt::updateFromProperty() {
    CollapsibleGroupBoxWidgetQt::updateFromProperty();
    CollapsibleGroupBoxWidgetQt::setCollapsed(boolCompProperty_->isCollapsed());
    CollapsibleGroupBoxWidgetQt::setChecked(boolCompProperty_->getBoolProperty()->get());
}

void BoolCompositePropertyWidgetQt::onSetDisplayName(Property*, const std::string& displayName) {
    displayName_ = displayName;
    label_->setText(displayName);
}

bool BoolCompositePropertyWidgetQt::isChecked() const { return boolCompProperty_->isChecked(); }
void BoolCompositePropertyWidgetQt::setChecked(bool checked) {
    CollapsibleGroupBoxWidgetQt::setChecked(checked);
    boolCompProperty_->setChecked(checked);
}

bool BoolCompositePropertyWidgetQt::isCollapsed() const { return boolCompProperty_->isCollapsed(); }

}  // namespace inviwo
