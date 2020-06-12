/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2020 Inviwo Foundation
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

#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/io/serialization/serializable.h>
#include <inviwo/core/io/serialization/versionconverter.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/network/networkvisitor.h>
#include <inviwo/core/network/lambdanetworkvisitor.h>

#include <iterator>

namespace inviwo {

PropertyOwner::PropertyOwner()
    : PropertyOwnerObservable(), invalidationLevel_(InvalidationLevel::Valid) {}

PropertyOwner::PropertyOwner(const PropertyOwner& rhs)
    : PropertyOwnerObservable(), invalidationLevel_(rhs.invalidationLevel_) {

    for (const auto& p : rhs.ownedProperties_) addProperty(p->clone());
}

PropertyOwner::~PropertyOwner() {
    while (size() != 0) {
        removeProperty(begin());
    }
}

void PropertyOwner::addProperty(Property* property, bool owner) {
    insertProperty(properties_.size(), property, owner);
}

void PropertyOwner::addProperty(Property& property) {
    insertProperty(properties_.size(), &property, false);
}

void PropertyOwner::insertProperty(size_t index, Property* property, bool owner) {
    if (index > properties_.size()) {
        index = properties_.size();
    }

    if (getPropertyByIdentifier(property->getIdentifier()) != nullptr) {
        throw Exception(
            "Can't add property, identifier \"" + property->getIdentifier() + "\" already exist.",
            IVW_CONTEXT);
    }
    if (auto parent = dynamic_cast<Property*>(this)) {
        if (parent == property) {
            throw Exception("Can't add property \"" + property->getIdentifier() + "\" to itself.",
                            IVW_CONTEXT);
        }
    }

    notifyObserversWillAddProperty(property, index);
    properties_.insert(properties_.begin() + index, property);
    property->setOwner(this);

    if (dynamic_cast<EventProperty*>(property)) {
        eventProperties_.push_back(static_cast<EventProperty*>(property));
    }
    if (dynamic_cast<CompositeProperty*>(property)) {
        compositeProperties_.push_back(static_cast<CompositeProperty*>(property));
    }

    if (owner) {  // Assume ownership of property;
        ownedProperties_.emplace_back(property);
    }
    notifyObserversDidAddProperty(property, index);
}

void PropertyOwner::insertProperty(size_t index, Property& property) {
    insertProperty(index, &property, false);
}

Property* PropertyOwner::removeProperty(const std::string& identifier) {
    return removeProperty(
        std::find_if(properties_.begin(), properties_.end(),
                     [&identifier](Property* p) { return p->getIdentifier() == identifier; }));
}

Property* PropertyOwner::removeProperty(Property* property) {
    return removeProperty(std::find(properties_.begin(), properties_.end(), property));
}

Property* PropertyOwner::removeProperty(Property& property) { return removeProperty(&property); }

Property* PropertyOwner::removeProperty(size_t index) {
    if (index >= size()) {
        throw RangeException("Invalid index when removing property " + std::to_string(index) +
                                 " (" + std::to_string(size()) + " elements)",
                             IVW_CONTEXT);
    }
    return removeProperty(begin() + index);
}

void PropertyOwner::forEachProperty(std::function<void(Property&)> callback,
                                    bool recursiveSearch) const {
    LambdaNetworkVisitor visitor{[&](Property& property) {
        callback(property);
        return recursiveSearch;
    }};
    for (auto* elem : properties_) {
        elem->accept(visitor);
    }
}

Property* PropertyOwner::removeProperty(std::vector<Property*>::iterator it) {
    Property* prop = nullptr;
    if (it != properties_.end()) {
        prop = *it;
        size_t index = std::distance(properties_.begin(), it);
        notifyObserversWillRemoveProperty(prop, index);

        util::erase_remove(eventProperties_, *it);
        util::erase_remove(compositeProperties_, *it);

        prop->setOwner(nullptr);
        properties_.erase(it);
        notifyObserversDidRemoveProperty(prop, index);

        // This will delete the property if owned; in that case set prop to nullptr.
        util::erase_remove_if(ownedProperties_, [&prop](const std::unique_ptr<Property>& p) {
            if (p.get() == prop) {
                prop = nullptr;
                return true;
            } else {
                return false;
            }
        });
    }
    return prop;
}

const std::vector<Property*>& PropertyOwner::getProperties() const { return properties_; }

const std::vector<CompositeProperty*>& PropertyOwner::getCompositeProperties() const {
    return compositeProperties_;
}

std::vector<Property*> PropertyOwner::getPropertiesRecursive() const {
    std::vector<Property*> result;
    result.reserve(properties_.size());
    result.insert(result.end(), properties_.begin(), properties_.end());

    for (auto comp : compositeProperties_) {
        std::vector<Property*> subprops = comp->getPropertiesRecursive();
        result.insert(result.end(), subprops.begin(), subprops.end());
    }
    return result;
}

Property* PropertyOwner::getPropertyByIdentifier(const std::string& identifier,
                                                 bool recursiveSearch) const {
    for (auto* property : properties_) {
        if (property->getIdentifier() == identifier) return property;
    }
    if (recursiveSearch) {
        for (auto* compositeProperty : compositeProperties_) {
            if (auto* p = compositeProperty->getPropertyByIdentifier(identifier, true)) return p;
        }
    }
    return nullptr;
}

Property* PropertyOwner::getPropertyByPath(const std::vector<std::string>& path) const {
    if (path.empty()) return nullptr;

    auto lastIt = --path.end();
    auto* curr = this;
    for (auto pathIt = path.begin(); pathIt != lastIt; ++pathIt) {
        auto compIt =
            std::find_if(curr->compositeProperties_.begin(), curr->compositeProperties_.end(),
                         [&](auto* comp) { return comp->getIdentifier() == *pathIt; });
        if (compIt != curr->compositeProperties_.end()) {
            curr = *compIt;
        } else {
            return nullptr;
        }
    }
    return curr->getPropertyByIdentifier(*lastIt);
}

size_t PropertyOwner::size() const { return properties_.size(); }

Property* PropertyOwner::operator[](size_t i) { return properties_[i]; }

const Property* PropertyOwner::operator[](size_t i) const { return properties_[i]; }

PropertyOwner::iterator PropertyOwner::begin() { return properties_.begin(); }

PropertyOwner::iterator PropertyOwner::end() { return properties_.end(); }

PropertyOwner::const_iterator PropertyOwner::cbegin() const { return properties_.cbegin(); }

PropertyOwner::const_iterator PropertyOwner::cend() const { return properties_.cend(); }

bool PropertyOwner::isValid() const { return invalidationLevel_ == InvalidationLevel::Valid; }

void PropertyOwner::setValid() {
    for (auto& elem : properties_) elem->setValid();
    invalidationLevel_ = InvalidationLevel::Valid;
}

inviwo::InvalidationLevel PropertyOwner::getInvalidationLevel() const { return invalidationLevel_; }

void PropertyOwner::invalidate(InvalidationLevel invalidationLevel, Property*) {
    invalidationLevel_ = std::max(invalidationLevel_, invalidationLevel);
}

Processor* PropertyOwner::getProcessor() { return nullptr; }

const Processor* PropertyOwner::getProcessor() const { return nullptr; }

void PropertyOwner::serialize(Serializer& s) const {
    auto ownedIdentifiers = util::transform(
        ownedProperties_, [](const std::unique_ptr<Property>& p) { return p->getIdentifier(); });
    s.serialize("OwnedPropertyIdentifiers", ownedIdentifiers, "PropertyIdentifier");

    s.serialize("Properties", properties_, "Property");
}

void PropertyOwner::deserialize(Deserializer& d) {
    // This is for finding renamed composites, and moving old properties to new composites.
    NodeVersionConverter tvc(this, &PropertyOwner::findPropsForComposites);
    d.convertVersion(&tvc);

    std::vector<std::string> ownedIdentifiers;
    d.deserialize("OwnedPropertyIdentifiers", ownedIdentifiers, "PropertyIdentifier");

    auto des = util::IdentifiedDeserializer<std::string, Property*>("Properties", "Property")
                   .setGetId([](Property* const& p) { return p->getIdentifier(); })
                   .setMakeNew([]() { return nullptr; })
                   .setNewFilter([&](const std::string& id, size_t /*ind*/) {
                       return util::contains(ownedIdentifiers, id);
                   })
                   .onNewIndexed([&](Property*& p, size_t i) { insertProperty(i, p, true); })
                   .onRemove([&](const std::string& id) {
                       if (util::contains_if(ownedProperties_, [&](std::unique_ptr<Property>& op) {
                               return op->getIdentifier() == id;
                           })) {
                           delete removeProperty(id);
                       }
                   });

    des(d, properties_);
}

bool PropertyOwner::findPropsForComposites(TxElement* node) {
    std::vector<const CompositeProperty*> props;
    std::copy(compositeProperties_.begin(), compositeProperties_.end(), std::back_inserter(props));
    return xml::findMatchingSubPropertiesForComposites(node, props);
}

void PropertyOwner::setAllPropertiesCurrentStateAsDefault() {
    for (auto& elem : properties_) (elem)->setCurrentStateAsDefault();
}

void PropertyOwner::resetAllPoperties() {
    for (auto& elem : properties_) elem->resetToDefaultState();
}

std::vector<std::string> PropertyOwner::getPath() const { return std::vector<std::string>(); }

void PropertyOwner::invokeEvent(Event* event) {
    for (auto elem : eventProperties_) {
        elem->invokeEvent(event);
        if (event->hasBeenUsed()) return;
    }
    for (auto elem : compositeProperties_) {
        elem->invokeEvent(event);
        if (event->hasBeenUsed()) return;
    }
}

InviwoApplication* PropertyOwner::getInviwoApplication() {
    return util::getInviwoApplication(getProcessor());
}

}  // namespace inviwo
