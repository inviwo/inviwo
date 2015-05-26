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

#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/templateproperty.h>
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

PropertyClassIdentifier(CompositeProperty, "org.inviwo.CompositeProperty");

CompositeProperty::CompositeProperty(std::string identifier, std::string displayName,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : Property(identifier, displayName, invalidationLevel, semantics)
    , PropertyOwner()
    , collapsed_(false)
    , subPropertyInvalidationLevel_(VALID) {}

CompositeProperty::CompositeProperty(const CompositeProperty& rhs)
    : Property(rhs), PropertyOwner(rhs), collapsed_(rhs.collapsed_) {}

CompositeProperty& CompositeProperty::operator=(const CompositeProperty& that) {
    if (this != &that) {
        Property::operator=(that);
        PropertyOwner::operator=(that);
        collapsed_ = that.collapsed_;
        subPropertyInvalidationLevel_ = that.subPropertyInvalidationLevel_;
    }
    return *this;
}

CompositeProperty* CompositeProperty::clone() const {
    return new CompositeProperty(*this);
}

CompositeProperty::~CompositeProperty() {}


void CompositeProperty::setOwner(PropertyOwner* owner) {
    Property::setOwner(owner);
    for (Property* property: properties_) property->setOwner(this);
}

UsageMode CompositeProperty::getUsageMode() const {
    UsageMode mode = DEVELOPMENT;
    for (Property* property : properties_) mode = std::min(mode, property->getUsageMode());
    return mode;
}
void CompositeProperty::setUsageMode(UsageMode usageMode) {
    for (Property* property : properties_) property->setUsageMode(usageMode);
    Property::setUsageMode(usageMode);
}

void CompositeProperty::updateVisibility() {
    Property::updateVisibility();
    for (Property* property : properties_) property->updateVisibility();
}

bool CompositeProperty::getVisible() {
    bool visible = false;
    for (Property* property : properties_) visible = visible || property->getVisible();
    return visible;
}

void CompositeProperty::setVisible(bool val) {
    for (Property* property : properties_) property->setVisible(val);
    Property::setVisible(val);
}

void CompositeProperty::setPropertyModified(bool modified) {
    for (Property* property: properties_) property->setPropertyModified(modified);
    Property::setPropertyModified(modified);
}

bool CompositeProperty::isPropertyModified() const {
    for (Property* property : properties_)
        if (property->isPropertyModified()) return true;

    return false;
}

void CompositeProperty::set(const Property* srcProperty) {
    NetworkLock lock;
    
    const CompositeProperty* compositeSrcProp = dynamic_cast<const CompositeProperty*>(srcProperty);

    if (compositeSrcProp) {
        std::vector<Property*> subProperties = compositeSrcProp->getProperties();

        if (subProperties.size() == this->properties_.size()) {
            for (size_t i = 0; i < subProperties.size(); i++) {
                this->properties_[i]->set(subProperties[i]);
            }

            propertyModified();
        } else {
            LogWarn("CompositeProperty mismatch. Unable to link");
        }
    } 
}

inviwo::InvalidationLevel CompositeProperty::getInvalidationLevel() const  {
    return std::min(subPropertyInvalidationLevel_, Property::getInvalidationLevel());
}

void CompositeProperty::invalidate(InvalidationLevel invalidationLevel,
                                   Property* modifiedProperty) {
    PropertyOwner::invalidate(invalidationLevel, modifiedProperty);
    subPropertyInvalidationLevel_ = invalidationLevel;
    Property::propertyModified();
}

void CompositeProperty::setValid() {
    PropertyOwner::setValid();
    subPropertyInvalidationLevel_ = VALID;
}

void CompositeProperty::setCurrentStateAsDefault() {
    Property::setCurrentStateAsDefault();
    for (auto& elem : properties_) {
        elem->setCurrentStateAsDefault();
    }
}

void CompositeProperty::resetToDefaultState() {
    for (auto& elem : properties_) {
        elem->resetToDefaultState();
    }
    Property::resetToDefaultState();
}

void CompositeProperty::serialize(IvwSerializer& s) const{
    Property::serialize(s);
    PropertyOwner::serialize(s);
    s.serialize("collapsed", collapsed_);
}

void CompositeProperty::deserialize(IvwDeserializer& d){
    Property::deserialize(d);
    PropertyOwner::deserialize(d);
    d.deserialize("collapsed", collapsed_);
}

std::vector<std::string> CompositeProperty::getPath() const {
    std::vector<std::string> path;
    const PropertyOwner* owner = getOwner();
    if (owner) {
        path = owner->getPath();
    }
    path.push_back(getIdentifier());
    return path;
}

Processor* CompositeProperty::getProcessor() {
    PropertyOwner* owner = getOwner();
    if (owner) {
        return owner->getProcessor();
    } else {
        return nullptr;
    }
}

const Processor* CompositeProperty::getProcessor() const {
    const PropertyOwner* owner = getOwner();
    if (owner) {
        return owner->getProcessor();
    } else {
        return nullptr;
    }
}

bool CompositeProperty::isCollapsed() const {
    return collapsed_;
}
void CompositeProperty::setCollapsed(bool value) {
    if (collapsed_ != value) {
        collapsed_ = value;
        Property::propertyModified();
    }
}



} // namespace
