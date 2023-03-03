/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2023 Inviwo Foundation
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
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/network/networkvisitor.h>
#include <inviwo/core/io/serialization/serialization.h>

namespace inviwo {

Property::Property(std::string_view identifier, std::string_view displayName, Document help,
                   InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : PropertyObservable()
    , MetaDataOwner()
    , serializationMode_(PropertySerializationMode::Default)
    , identifier_(identifier)
    , displayName_("displayName", std::string(displayName))
    , readOnly_("readonly", false)
    , semantics_("semantics", semantics)
    , visible_("visible", true)
    , propertyModified_(true)
    , invalidationLevel_(invalidationLevel)
    , owner_(nullptr)
    , initiatingWidget_(nullptr)
    , help_{std::move(help)} {

    util::validateIdentifier(identifier, "Property", IVW_CONTEXT);
}

Property::Property(std::string_view identifier, std::string_view displayName,
                   InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : Property(identifier, displayName, Document{}, invalidationLevel, semantics) {}

Property::Property(const Property& rhs)
    : PropertyObservable(rhs)
    , MetaDataOwner(rhs)
    , serializationMode_(rhs.serializationMode_)
    , identifier_(rhs.identifier_)
    , displayName_(rhs.displayName_)
    , readOnly_(rhs.readOnly_)
    , semantics_(rhs.semantics_)
    , visible_(rhs.visible_)
    , propertyModified_(rhs.propertyModified_)
    , invalidationLevel_(rhs.invalidationLevel_)
    , owner_(nullptr)
    , initiatingWidget_(rhs.initiatingWidget_)
    , help_{rhs.help_} {}

Property::~Property() {
    if (auto owner = getOwner()) {
        owner->removeProperty(this);
    }
}

const std::string& Property::getIdentifier() const { return identifier_; }
Property& Property::setIdentifier(std::string_view identifier) {
    if (identifier_ != identifier) {
        identifier_ = identifier;

        util::validateIdentifier(identifier, "Property", IVW_CONTEXT);

        notifyObserversOnSetIdentifier(this, identifier_);
        notifyAboutChange();
    }
    return *this;
}

const std::string& Property::getPath() const {
    path_.clear();

    const auto traverse = [&](auto& self, PropertyOwner* owner) -> void {
        if (owner) {
            self(self, owner->getOwner());
            std::copy(owner->getIdentifier().begin(), owner->getIdentifier().end(),
                      std::back_inserter(path_));
            path_.push_back('.');
        }
    };
    traverse(traverse, owner_);
    std::copy(identifier_.begin(), identifier_.end(), std::back_inserter(path_));

    return path_;
}

const std::string& Property::getDisplayName() const { return displayName_; }

Property& Property::setDisplayName(std::string_view displayName) {
    if (displayName_.value != displayName) {
        displayName_.value = displayName;
        notifyObserversOnSetDisplayName(this, displayName_);
        notifyAboutChange();
    }
    return *this;
}

PropertySemantics Property::getSemantics() const { return semantics_; }

Property& Property::setSemantics(const PropertySemantics& semantics) {
    if (semantics_.update(semantics)) {
        notifyObserversOnSetSemantics(this, semantics_);
        notifyAboutChange();
    }
    return *this;
}

std::string Property::getClassIdentifierForWidget() const { return getClassIdentifier(); }

bool Property::getReadOnly() const { return readOnly_; }
Property& Property::setReadOnly(bool readOnly) {
    if (readOnly_.update(readOnly)) {
        notifyObserversOnSetReadOnly(this, readOnly_);
        notifyAboutChange();
    }
    return *this;
}

InvalidationLevel Property::getInvalidationLevel() const { return invalidationLevel_; }
Property& Property::setInvalidationLevel(InvalidationLevel invalidationLevel) {
    invalidationLevel_ = invalidationLevel;
    return *this;
}

PropertyOwner* Property::getOwner() { return owner_; }

const PropertyOwner* Property::getOwner() const { return owner_; }

void Property::setOwner(PropertyOwner* owner) { owner_ = owner; }

void Property::registerWidget(PropertyWidget* propertyWidget) {
    propertyWidgets_.push_back(propertyWidget);
}

void Property::deregisterWidget(PropertyWidget* propertyWidget) {
    util::erase_remove(propertyWidgets_, propertyWidget);
}

void Property::setInitiatingWidget(PropertyWidget* propertyWidget) {
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

bool Property::hasWidgets() const { return !propertyWidgets_.empty(); }

Property& Property::propertyModified() {
    NetworkLock lock(this);
    onChangeCallback_.invokeAll();
    setModified();

    if (auto owner = getOwner()) {
        // Evaluate property links
        if (auto processor = owner->getProcessor()) {
            processor->notifyObserversAboutPropertyChange(this);
        }

        // Invalidate Owner
        if (getInvalidationLevel() > InvalidationLevel::Valid) {
            owner->invalidate(getInvalidationLevel(), this);
        }
    }

    updateWidgets();
    return *this;
}

void Property::setValid() { propertyModified_ = false; }

Property& Property::setModified() {
    propertyModified_ = true;
    return *this;
}

bool Property::isModified() const { return propertyModified_; }

void Property::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
    s.serialize("identifier", identifier_, SerializationTarget::Attribute);
    displayName_.serialize(s, serializationMode_);
    semantics_.serialize(s, serializationMode_);
    visible_.serialize(s, serializationMode_);
    readOnly_.serialize(s, serializationMode_);

    MetaDataOwner::serialize(s);
}

void Property::deserialize(Deserializer& d) {
    std::string className;
    d.deserialize("type", className, SerializationTarget::Attribute);
    if (className != getClassIdentifier()) {
        LogWarn("Deserialized property: " + getPath() +
                " with class identifier: " + getClassIdentifier() +
                " from a serialized property with a different class identifier: " + className);
    }

    {
        auto old = identifier_;
        d.deserialize("identifier", identifier_, SerializationTarget::Attribute);
        if (old != identifier_) {
            notifyObserversOnSetIdentifier(this, identifier_);
        }
    }

    if (displayName_.deserialize(d, serializationMode_)) {
        notifyObserversOnSetDisplayName(this, displayName_);
    }
    if (semantics_.deserialize(d, serializationMode_)) {
        notifyObserversOnSetSemantics(this, semantics_);
    }
    if (visible_.deserialize(d, serializationMode_)) {
        notifyObserversOnSetVisible(this, visible_);
    }
    if (readOnly_.deserialize(d, serializationMode_)) {
        notifyObserversOnSetReadOnly(this, readOnly_);
    }

    MetaDataOwner::deserialize(d);
}

bool Property::getVisible() const { return visible_; }
Property& Property::setVisible(bool visible) {
    if (visible_ != visible) {
        visible_ = visible;
        notifyObserversOnSetVisible(this, visible_);
        notifyAboutChange();
    }
    return *this;
}

const Document& Property::getHelp() const { return help_; }
Document& Property::getHelp() { return help_; }

Property& Property::setHelp(Document help) {
    help_ = std::move(help);
    return *this;
}

Document Property::getDescription() const {
    Document doc;
    using P = Document::PathComponent;

    doc.append("div", displayName_.value, {{"class", "name"}});

    if (!help_.empty()) {
        doc.append("div", "", {{"class", "help"}}).append(help_);
    }

    using H = utildoc::TableBuilder::Header;

    utildoc::TableBuilder tb(doc.handle(), P::end(), {{"class", "propertyInfo"}});
    tb(H("Identifier"), identifier_);
    tb(H("Class Identifier"), getClassIdentifier());
    tb(H("Path"), getPath());
    util::for_each_argument([&tb](auto p) { tb(H(camelCaseToHeader(p.name)), p.value); }, readOnly_,
                            semantics_, visible_);
    tb(H("Validation Level"), invalidationLevel_);

    return doc;
}

const std::vector<std::pair<std::string, std::string>>& Property::getAutoLinkToProperty() const {
    return autoLinkTo_;
}

void Property::accept(NetworkVisitor& visitor) { visitor.visit(*this); }

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

Property& Property::setCurrentStateAsDefault() {
    displayName_.setAsDefault();
    readOnly_.setAsDefault();
    semantics_.setAsDefault();
    visible_.setAsDefault();
    return *this;
}

Property& Property::resetToDefaultState() {
    propertyModified();
    return *this;
}

bool Property::isDefaultState() const { return false; }

bool Property::needsSerialization() const {
    switch (serializationMode_) {
        case PropertySerializationMode::All:
            return true;
        case PropertySerializationMode::None:
            return false;
        case PropertySerializationMode::Default:
            [[fallthrough]];
        default:
            return !isDefaultState() || !getMetaDataMap()->empty();
    }
}

void Property::set(const Property* /*src*/) { propertyModified(); }

const std::vector<PropertyWidget*>& Property::getWidgets() const { return propertyWidgets_; }

PropertySerializationMode Property::getSerializationMode() const { return serializationMode_; }
Property& Property::setSerializationMode(PropertySerializationMode mode) {
    serializationMode_ = mode;
    return *this;
}

std::shared_ptr<std::function<void()>> Property::onChangeScoped(std::function<void()> callback) {
    return onChangeCallback_.addLambdaCallbackRaii(std::move(callback));
}

const BaseCallBack* Property::onChange(std::function<void()> callback) {
    return onChangeCallback_.addLambdaCallback(std::move(callback));
}

void Property::removeOnChange(const BaseCallBack* callback) { onChangeCallback_.remove(callback); }

Property::OnChangeBlocker::OnChangeBlocker(Property& property) : property_(property) {
    property_.onChangeCallback_.startBlockingCallbacks();
}
Property::OnChangeBlocker::~OnChangeBlocker() {
    property_.onChangeCallback_.stopBlockingCallbacks();
}

}  // namespace inviwo
