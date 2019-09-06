/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/stringconversion.h>

namespace inviwo {

const std::string CompositeProperty::classIdentifier = "org.inviwo.CompositeProperty";
std::string CompositeProperty::getClassIdentifier() const { return classIdentifier; }

CompositeProperty::CompositeProperty(const std::string& identifier, const std::string& displayName,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : Property(identifier, displayName, invalidationLevel, semantics)
    , PropertyOwner()
    , collapsed_(false)
    , subPropertyInvalidationLevel_(InvalidationLevel::Valid) {}

CompositeProperty* CompositeProperty::clone() const { return new CompositeProperty(*this); }

std::string CompositeProperty::getClassIdentifierForWidget() const {
    return CompositeProperty::classIdentifier;
}

void CompositeProperty::setOwner(PropertyOwner* owner) {
    Property::setOwner(owner);
    for (Property* property : properties_) property->setOwner(this);
}

void CompositeProperty::set(const Property* srcProperty) {
    if (const auto compositeSrcProp = dynamic_cast<const CompositeProperty*>(srcProperty)) {
        set(compositeSrcProp);
    }
}

void CompositeProperty::set(const CompositeProperty* src) {
    NetworkLock lock(this);
    const auto& subProperties = src->getProperties();
    if (subProperties.size() == properties_.size()) {
        for (size_t i = 0; i < subProperties.size(); i++) {
            this->properties_[i]->set(subProperties[i]);
        }
        propertyModified();
    } else {
        LogWarn("Unable to link CompositeProperties: \n"
                << joinString(src->getPath(), ".") << "\n to \n"
                << joinString(getPath(), ".") << ".\nNumber of sub properties differ ("
                << subProperties.size() << " vs " << properties_.size() << ")");
    }
}

InvalidationLevel CompositeProperty::getInvalidationLevel() const {
    return std::min(subPropertyInvalidationLevel_, Property::getInvalidationLevel());
}

void CompositeProperty::invalidate(InvalidationLevel invalidationLevel,
                                   Property* modifiedProperty) {
    PropertyOwner::invalidate(invalidationLevel, modifiedProperty);
    subPropertyInvalidationLevel_ = invalidationLevel;
    Property::propertyModified();
}

void CompositeProperty::setValid() {
    Property::setValid();
    PropertyOwner::setValid();
    subPropertyInvalidationLevel_ = InvalidationLevel::Valid;
}

CompositeProperty& CompositeProperty::setCurrentStateAsDefault() {
    Property::setCurrentStateAsDefault();
    for (auto& elem : properties_) {
        elem->setCurrentStateAsDefault();
    }
    return *this;
}

CompositeProperty& CompositeProperty::resetToDefaultState() {
    NetworkLock lock(this);
    for (auto& elem : properties_) {
        elem->resetToDefaultState();
    }
    return *this;
}

CompositeProperty& CompositeProperty::setReadOnly(bool value) {
    Property::setReadOnly(value);
    for (auto& elem : properties_) {
        elem->setReadOnly(value);
    }
    return *this;
}

void CompositeProperty::serialize(Serializer& s) const {
    Property::serialize(s);
    PropertyOwner::serialize(s);
    s.serialize("collapsed", collapsed_);
}

void CompositeProperty::deserialize(Deserializer& d) {
    Property::deserialize(d);
    PropertyOwner::deserialize(d);
    d.deserialize("collapsed", collapsed_);
}

std::vector<std::string> CompositeProperty::getPath() const {
    std::vector<std::string> path;
    if (const auto owner = getOwner()) {
        path = owner->getPath();
    }
    path.push_back(getIdentifier());
    return path;
}

Processor* CompositeProperty::getProcessor() {
    if (auto owner = getOwner()) {
        return owner->getProcessor();
    } else {
        return nullptr;
    }
}

const Processor* CompositeProperty::getProcessor() const {
    if (const auto owner = getOwner()) {
        return owner->getProcessor();
    } else {
        return nullptr;
    }
}

bool CompositeProperty::isCollapsed() const { return collapsed_; }
void CompositeProperty::setCollapsed(bool value) {
    if (collapsed_ != value) {
        collapsed_ = value;
        notifyObserversOnSetCollapsed(collapsed_);
    }
}

}  // namespace inviwo
