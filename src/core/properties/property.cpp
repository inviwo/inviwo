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

#include <inviwo/core/properties/property.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/network/networklock.h>

namespace inviwo {

 Property::Property(const std::string& identifier, const std::string& displayName,
                   InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : PropertyObservable()
    , Serializable()
    , MetaDataOwner()
    , serializationMode_(PropertySerializationMode::Default)
    , identifier_(identifier)
    , displayName_("displayName", displayName)
    , readOnly_("readonly", false)
    , semantics_("semantics", semantics)
    , usageMode_("usageMode", UsageMode::Application)
    , visible_("visible", true)
    , propertyModified_(false)
    , invalidationLevel_(invalidationLevel)
    , owner_(nullptr)
    , initiatingWidget_(nullptr) {}

Property::Property(const Property& rhs)
    : PropertyObservable(rhs)
    , Serializable(rhs)
    , MetaDataOwner(rhs)
    , serializationMode_(rhs.serializationMode_)
    , identifier_(rhs.identifier_)
    , displayName_(rhs.displayName_)
    , readOnly_(rhs.readOnly_)
    , semantics_(rhs.semantics_)
    , usageMode_(rhs.usageMode_)
    , visible_(rhs.visible_)
    , propertyModified_(rhs.propertyModified_)
    , invalidationLevel_(rhs.invalidationLevel_)
    , owner_(rhs.owner_)
    , initiatingWidget_(rhs.initiatingWidget_) {
}

Property& Property::operator=(const Property& that) {
    if (this != &that) {
        PropertyObservable::operator=(that);
        Serializable::operator=(that);
        MetaDataOwner::operator=(that);
        serializationMode_ = that.serializationMode_;
        identifier_ = that.identifier_;
        displayName_  = that.displayName_;
        readOnly_ = that.readOnly_;
        semantics_ = that.semantics_;
        usageMode_ = that.usageMode_;
        visible_ = that.visible_;
        propertyModified_ = that.propertyModified_;
        invalidationLevel_ = that.invalidationLevel_;
        owner_ = that.owner_;
        initiatingWidget_ = that.initiatingWidget_;
    }
    return *this;
}

Property* Property::clone() const {
    return nullptr; // See ticket #642 //make abstract...
}

std::string Property::getIdentifier() const {
    return identifier_;
}
void Property::setIdentifier(const std::string& identifier) {
    if (identifier_ != identifier) {
        identifier_ = identifier;
        notifyObserversOnSetIdentifier(identifier_);
        notifyAboutChange();
    }
}
std::vector<std::string> Property::getPath() const {
    std::vector<std::string> path;
    if(owner_) {
        path = owner_->getPath();
    } 
    path.push_back(identifier_);
    return path;
}

std::string Property::getDisplayName() const { return displayName_; }

void Property::setDisplayName(const std::string& displayName) {
    if (displayName_ != displayName) {
        displayName_ = displayName;
        notifyObserversOnSetDisplayName(displayName_);
        notifyAboutChange();
    }
}

PropertySemantics Property::getSemantics() const { return semantics_; }

void Property::setSemantics(const PropertySemantics& semantics) {
    if (semantics_ != semantics) {
        semantics_ = semantics;
        notifyObserversOnSetSemantics(semantics_);
        notifyAboutChange();
    }
}

std::string Property::getClassIdentifierForWidget() const { return getClassIdentifier(); }

bool Property::getReadOnly()const {
    return readOnly_;
}
void Property::setReadOnly(const bool& readOnly) {
    if (readOnly_ != readOnly) {
        readOnly_ = readOnly;
        notifyObserversOnSetReadOnly(readOnly_);
        notifyAboutChange();
    }
}

InvalidationLevel Property::getInvalidationLevel() const {
    return invalidationLevel_;
}
void Property::setInvalidationLevel(InvalidationLevel invalidationLevel) {
    invalidationLevel_ = invalidationLevel;
}

PropertyOwner* Property::getOwner() {
    return owner_;
}

const PropertyOwner* Property::getOwner() const {
    return owner_;
}

void Property::setOwner(PropertyOwner* owner) {
    owner_ = owner;
}

void Property::registerWidget(PropertyWidget* propertyWidget) {
    propertyWidgets_.push_back(propertyWidget);
}

void Property::deregisterWidget(PropertyWidget* propertyWidget) {
    util::erase_remove(propertyWidgets_, propertyWidget);
}

void Property::setInitiatingWidget(PropertyWidget* propertyWidget){
    initiatingWidget_ = propertyWidget;
}
void Property::clearInitiatingWidget() { initiatingWidget_ = nullptr; }
void Property::updateWidgets() {
    for (auto& elem : propertyWidgets_) {
        if (elem != nullptr && elem != initiatingWidget_) {
            elem->updateFromProperty();
        }
    }
}

bool Property::hasWidgets() const {
    return !propertyWidgets_.empty();
}

void Property::propertyModified() {
    NetworkLock lock(this);
    onChangeCallback_.invokeAll();
    setPropertyModified(true);

    PropertyOwner* owner = getOwner();
    if (owner) {
        // Evaluate property links       
        if (Processor* processor = owner->getProcessor()) {
             processor->notifyObserversAboutPropertyChange(this);
        }

        // Invalidate Owner
        if (getInvalidationLevel() > InvalidationLevel::Valid) {
            owner->invalidate(getInvalidationLevel(), this);        
        }
    }

    updateWidgets();
}

void Property::setPropertyModified(bool modified) {
    propertyModified_ = modified;
}

bool Property::isPropertyModified() const {
    return propertyModified_;
}

void Property::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
    s.serialize("identifier", identifier_, SerializationTarget::Attribute);
    displayName_.serialize(s, serializationMode_);
    semantics_.serialize(s, serializationMode_);
    usageMode_.serialize(s, serializationMode_);
    visible_.serialize(s, serializationMode_);
    readOnly_.serialize(s, serializationMode_);

    MetaDataOwner::serialize(s);
}

void Property::deserialize(Deserializer& d) {
    std::string className;
    d.deserialize("type", className, SerializationTarget::Attribute);

    {
        auto old = identifier_;
        d.deserialize("identifier", identifier_, SerializationTarget::Attribute);
        if (old != identifier_) notifyObserversOnSetIdentifier(identifier_);
    }
    {
        auto old = displayName_.value;
        displayName_.deserialize(d);
        if (old != displayName_.value) notifyObserversOnSetDisplayName(displayName_);
    }
    {
        auto old = semantics_.value;
        semantics_.deserialize(d);
        if (old != semantics_.value) notifyObserversOnSetSemantics(semantics_);
    }
    {
        auto old = usageMode_.value;
        int mode = static_cast<int>(usageMode_.value);
        d.deserialize(usageMode_.name, mode);
        usageMode_ = static_cast<UsageMode>(mode);
        if (old != usageMode_.value) notifyObserversOnSetUsageMode(usageMode_);
    }
    {
        auto old = visible_.value;
        visible_.deserialize(d);
        if (old != visible_.value) notifyObserversOnSetVisible(visible_);
    }
    {
        auto old = readOnly_.value;
        readOnly_.deserialize(d);
        if (old != readOnly_.value) notifyObserversOnSetReadOnly(readOnly_);
    }

    MetaDataOwner::deserialize(d);
}

inviwo::UsageMode Property::getUsageMode() const {
    return usageMode_;
}
void Property::setUsageMode(UsageMode usageMode) {
    if (usageMode_ != usageMode) {
        usageMode_ = usageMode;
        notifyObserversOnSetUsageMode(usageMode_);
        notifyAboutChange();
    }
}

bool Property::getVisible() {
    return visible_;
}
void Property::setVisible(bool visible) {
    if (visible_ != visible) {
        visible_ = visible;
        notifyObserversOnSetVisible(visible_);
        notifyAboutChange();
    }
}

const std::vector<std::pair<std::string, std::string>>& Property::getAutoLinkToProperty() const {
    return autoLinkTo_;
}

// Call this when a property has changed in a way not related to it's "value"
// When for example semantics have changed, i.e. for stuff where property 
// modified is __not__ called. This state changes should not effect the outcome of a 
// network evaluation. 
void Property::notifyAboutChange() {     
    if (auto owner = getOwner()) { 
        if (auto processor = owner->getProcessor()) {
            // By putting a nullptr here we will avoid evaluation links.
            processor->notifyObserversAboutPropertyChange(nullptr);
        }
    }
}

void Property::setCurrentStateAsDefault() {
    readOnly_.setAsDefault();
    semantics_.setAsDefault();
}
void Property::resetToDefaultState() {
    propertyModified();
}

void Property::set(const Property* src) {
    propertyModified();
}

const std::vector<PropertyWidget*>& Property::getWidgets() const {
    return propertyWidgets_;
}

PropertySerializationMode Property::getSerializationMode() const {
    return serializationMode_;
}
void Property::setSerializationMode(PropertySerializationMode mode) {
    serializationMode_ = mode;
}

const BaseCallBack* Property::onChange(std::function<void()> callback) {
    return onChangeCallback_.addLambdaCallback(callback);   
}

void Property::removeOnChange(const BaseCallBack* callback) {
    onChangeCallback_.remove(callback);
}

} // namespace
