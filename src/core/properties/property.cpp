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

namespace inviwo {

PropertyClassIdentifier(Property, "org.inviwo.undefined");

Property::Property(const std::string& identifier, const std::string& displayName,
                   InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : IvwSerializable()
    , MetaDataOwner()
    , serializationMode_(DEFAULT)
    , identifier_(identifier)
    , displayName_("displayName", displayName)
    , readOnly_("readonly", false)
    , semantics_("semantics", semantics)
    , usageMode_("usageMode", APPLICATION)
    , visible_("visible", true)
    , propertyModified_(false)
    , invalidationLevel_(invalidationLevel)
    , owner_(nullptr)
    , initiatingWidget_(nullptr) {}

Property::Property(const Property& rhs)
    : IvwSerializable()
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
    return new Property(*this);
}

Property::~Property() {
}

std::string Property::getIdentifier() const {
    return identifier_;
}
std::vector<std::string> Property::getPath() const {
    std::vector<std::string> path;
    if(owner_) {
        path = owner_->getPath();
    } 
    path.push_back(identifier_);
    return path;
}


void Property::setIdentifier(const std::string& identifier) {
    identifier_ = identifier;
}

std::string Property::getDisplayName() const {
    return displayName_;
}

void Property::setDisplayName(const std::string& displayName) {
    displayName_ = displayName;
    updateWidgets();
}

PropertySemantics Property::getSemantics()const {
    return semantics_;
}

std::string Property::getClassIdentifierForWidget()const{
    return getClassIdentifier();
}

void Property::setSemantics(const PropertySemantics& semantics) {
    semantics_ = semantics;
}


void Property::setReadOnly(const bool& value) {
    readOnly_ = value;
    updateWidgets();
}

bool Property::getReadOnly()const {
    return readOnly_;
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
    if (std::find(propertyWidgets_.begin(), propertyWidgets_.end(), propertyWidget) !=
        propertyWidgets_.end()) {
        propertyWidgets_.erase(
            std::remove(propertyWidgets_.begin(), propertyWidgets_.end(), propertyWidget),
            propertyWidgets_.end());
    }
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
    InviwoApplication::getPtr()->getProcessorNetwork()->lock();
    onChangeCallback_.invokeAll();
    setPropertyModified(true);

    PropertyOwner* owner = getOwner();
    if (owner) {
        // Evaluate property links
        Processor* processor = owner->getProcessor();
        if (processor) {
             processor->notifyObserversAboutPropertyChange(this);
        }

        // Invalidate Owner
        if (getInvalidationLevel() > VALID) {
            owner->invalidate(getInvalidationLevel(), this);        
        }
    }

    updateWidgets();
    InviwoApplication::getPtr()->getProcessorNetwork()->unlock();
}

void Property::setPropertyModified(bool modified) {
    propertyModified_ = modified;
}

bool Property::isPropertyModified() const {
    return propertyModified_;
}

void Property::serialize(IvwSerializer& s) const {
    s.serialize("type", getClassIdentifier(), true);
    s.serialize("identifier", identifier_, true);
    if (serializationMode_ == ALL || !displayName_.isDefault()) {
        s.serialize(displayName_.name, displayName_, true);
    }
    semantics_.serialize(s, serializationMode_);
    usageMode_.serialize(s, serializationMode_);
    visible_.serialize(s, serializationMode_);
    readOnly_.serialize(s, serializationMode_);

    MetaDataOwner::serialize(s);
}

void Property::deserialize(IvwDeserializer& d) {
    std::string className;
    d.deserialize("type", className, true);
    d.deserialize("identifier", identifier_, true);
    d.deserialize(displayName_.name, displayName_.value, true);
    semantics_.deserialize(d);

    int mode = usageMode_;
    d.deserialize(usageMode_.name, mode);
    usageMode_ = static_cast<UsageMode>(mode);

    visible_.deserialize(d);
    readOnly_.deserialize(d);

    updateVisibility();
    MetaDataOwner::deserialize(d);
}

void Property::setUsageMode(UsageMode visibilityMode) {
    this->usageMode_ = visibilityMode;
    updateVisibility();
}

void Property::updateVisibility() {
    UsageMode appMode =
        InviwoApplication::getPtr()->getSettingsByType<SystemSettings>()->getApplicationUsageMode();

    UsageMode mode = getUsageMode();

    if (getVisible() == false) {
        for (auto& elem : propertyWidgets_) elem->hideWidget();
    }

    if (mode == APPLICATION) {
        for (auto& elem : propertyWidgets_) elem->showWidget();
    } else if (mode == DEVELOPMENT && appMode == DEVELOPMENT) {
        for (auto& elem : propertyWidgets_) elem->showWidget();
    } else if (mode == DEVELOPMENT && appMode == APPLICATION) {
        for (auto& elem : propertyWidgets_) elem->hideWidget();
    }

}

void Property::setVisible(bool val) {
    visible_ = val;
    updateVisibility();
}
bool Property::getVisible() {
    return visible_;
}

void Property::setCurrentStateAsDefault() {
    readOnly_.setAsDefault();
    semantics_.setAsDefault();
}

void Property::resetToDefaultState() {
    readOnly_.reset();
    propertyModified();
}

void Property::set(const Property* src) {
    propertyModified();
}

inviwo::UsageMode Property::getUsageMode() const {
    return usageMode_;
}

const std::vector<PropertyWidget*>& Property::getWidgets() const {
    return propertyWidgets_;
}

void Property::setSerializationMode(PropertySerializationMode mode) {
    serializationMode_ = mode;
}

PropertySerializationMode Property::getSerializationMode() const {
    return serializationMode_;
}

void Property::onChange(std::function<void()> callback) {
    onChangeCallback_.addLambdaCallback(callback);   
}



} // namespace
