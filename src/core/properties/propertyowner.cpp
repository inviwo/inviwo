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

#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/io/serialization/ivwserializable.h>
#include <inviwo/core/io/serialization/versionconverter.h>

namespace inviwo {

PropertyOwner::PropertyOwner()
    : PropertyOwnerObservable()
    , invalidationLevel_(VALID) {
}

PropertyOwner::PropertyOwner(const PropertyOwner& rhs)
    : PropertyOwnerObservable()
    , invalidationLevel_(rhs.invalidationLevel_) {

    for(Property* p : rhs.ownedProperties_) {
        addProperty(p->clone());   
    }
}

PropertyOwner& PropertyOwner::operator=(const PropertyOwner& that) {
    if (this != &that) {
        invalidationLevel_ = that.invalidationLevel_;
        properties_.clear();

        for (Property* p : ownedProperties_) {
            delete p;
        }
        ownedProperties_.clear();

        for (Property* p : that.ownedProperties_) {
            addProperty(p->clone());
        }
    }
    return *this;
}

PropertyOwner::~PropertyOwner() {
    for (Property* p : ownedProperties_) delete p;
}

void PropertyOwner::addProperty(Property* property, bool owner) {
    ivwAssert(getPropertyByIdentifier(property->getIdentifier()) == nullptr,
              "Property already exist");

    notifyObserversWillAddProperty(property, properties_.size());
    properties_.push_back(property);
    property->setOwner(this);
    if (dynamic_cast<EventProperty*>(property)) {
        eventProperties_.push_back(static_cast<EventProperty*>(property));
    }
    if (dynamic_cast<CompositeProperty*>(property)) {
        compositeProperties_.push_back(static_cast<CompositeProperty*>(property));
    }

    if (owner) { // Assume ownership of property;
        ownedProperties_.push_back(property);
    }
    notifyObserversDidAddProperty(property, properties_.size()-1);
}

void PropertyOwner::addProperty(Property& property) {
    addProperty(&property, false);
}

Property* PropertyOwner::removeProperty(const std::string& identifier) {
    std::vector<Property*>::iterator it =
        std::find_if(properties_.begin(), properties_.end(), property_has_identifier(identifier));
    return removeProperty(it);;
}

Property* PropertyOwner::removeProperty(Property* property) {  
    std::vector<Property*>::iterator it =
        std::find(properties_.begin(), properties_.end(), property);
    return removeProperty(it);
}

Property* PropertyOwner::removeProperty(Property& property) {
    return removeProperty(&property);
}

Property* PropertyOwner::removeProperty(std::vector<Property*>::iterator it) {
    Property* prop = nullptr;
    if (it != properties_.end()) {
        prop = *it;
        size_t index = std::distance(properties_.begin(), it);
        notifyObserversWillRemoveProperty(prop, index);

        ownedProperties_.erase(std::remove(ownedProperties_.begin(), ownedProperties_.end(), *it),
                               ownedProperties_.end());
        eventProperties_.erase(std::remove(eventProperties_.begin(), eventProperties_.end(), *it),
                               eventProperties_.end());
        compositeProperties_.erase(
            std::remove(compositeProperties_.begin(), compositeProperties_.end(), *it),
            compositeProperties_.end());

        prop->setOwner(nullptr);
        properties_.erase(it);
        notifyObserversDidRemoveProperty(prop, index);
    }
    return prop;
}

std::vector<Property*> PropertyOwner::getProperties(bool recursive) const {
    if (!recursive) {
        return properties_;
    } else {
        std::vector<Property*> result;
        result.reserve(properties_.size());
        result.insert(result.end(), properties_.begin(), properties_.end());

        for (auto comp : compositeProperties_) {
            std::vector<Property*> subprops = comp->getProperties(true);
            result.insert(result.end(), subprops.begin(), subprops.end());
        }
        return result;
    }
}

Property* PropertyOwner::getPropertyByIdentifier(const std::string& identifier,
                                                 bool recursiveSearch) const {
    for (Property* property : properties_) {
        if (property->getIdentifier() == identifier) return property;
    }
    if (recursiveSearch) {
        for (CompositeProperty* compositeProperty : compositeProperties_) {
            Property* p = compositeProperty->getPropertyByIdentifier(identifier, true);
            if (p) return p;
        }
    }
    return nullptr;
}

Property* PropertyOwner::getPropertyByPath(const std::vector<std::string>& path) const {
    Property* property = getPropertyByIdentifier(path[0]);
    if (property) {
        size_t i = 1;
        while (path.size() > i) {
            CompositeProperty* comp = dynamic_cast<CompositeProperty*>(property);
            if (comp) {
                property = comp->getPropertyByIdentifier(path[i]);
                if (!property) return nullptr;
            } else {
                return nullptr;
            }
            ++i;
        }
        return property;
    }
    return nullptr;
}

void PropertyOwner::setValid() {
    for (auto& elem : properties_) elem->setPropertyModified(false);
    invalidationLevel_ = VALID;
}

void PropertyOwner::invalidate(InvalidationLevel invalidationLevel, Property*) {
    invalidationLevel_ = std::max(invalidationLevel_, invalidationLevel);
}

void PropertyOwner::serialize(IvwSerializer& s) const {
    s.serialize("Properties", properties_, "Property");
}

void PropertyOwner::deserialize(IvwDeserializer& d) {

    // This is for finding renamed composites, and moving old properties to new composites.
    NodeVersionConverter tvc(this, &PropertyOwner::findPropsForComposites);
    d.convertVersion(&tvc);

    std::vector<std::string> identifers;
    for (Property* p : properties_) identifers.push_back(p->getIdentifier());

    StandardIdentifier<Property> propertyIdentifier;
    d.deserialize("Properties", properties_, "Property", propertyIdentifier);

    for (size_t i = 0; i < properties_.size(); ++i) {
        Property* p = properties_[i];
        auto it =
            std::find_if(identifers.begin(), identifers.end(),
                         [&p](const std::string& id) -> bool { return id == p->getIdentifier(); });

        // Property is created in the de-serialization, assume ownership
        if (it == identifers.end()) {
            notifyObserversWillAddProperty(p, i);
            p->setOwner(this);
            if (dynamic_cast<EventProperty*>(p)) {
                eventProperties_.push_back(static_cast<EventProperty*>(p));
            }
            if (dynamic_cast<CompositeProperty*>(p)) {
                compositeProperties_.push_back(static_cast<CompositeProperty*>(p));
            }
            ownedProperties_.push_back(p);
            notifyObserversDidAddProperty(p, i);
        }
    }
}

bool PropertyOwner::findPropsForComposites(TxElement* node) {
    std::vector<const CompositeProperty*> props;
    for (auto it = properties_.begin(); it != properties_.end(); ++it) {
        CompositeProperty* cp = dynamic_cast<CompositeProperty*>(*it);
        if(cp){
            props.push_back(cp);
        }
    }
    return util::xmlFindMatchingSubPropertiesForComposites(node, props);
}

void PropertyOwner::setAllPropertiesCurrentStateAsDefault(){
    for (auto& elem : properties_) (elem)->setCurrentStateAsDefault();
}

void PropertyOwner::resetAllPoperties(){
    for (auto& elem : properties_) (elem)->resetToDefaultState();
}

bool PropertyOwner::property_has_identifier::operator () (const Property* p) {
    return p->getIdentifier() == id_;
}

std::string PropertyOwner::invalidationLevelToString(InvalidationLevel level) {
    switch (level) {
        case VALID: return "Valid";
        case INVALID_OUTPUT: return "Invalid output";
        case INVALID_RESOURCES: return "Invalid resources";
        default: return "Unknown";
    }
}

std::vector<std::string> PropertyOwner::getPath() const {
    return std::vector<std::string>();
}

void PropertyOwner::invokeEvent(Event* event) {
    for (auto elem : eventProperties_) {
        if (elem->getEvent()->matching(event)) {
            elem->getAction()->invoke(event);
            if (event->hasBeenUsed()) return;
        }
    }
    for (auto elem : compositeProperties_) {
        elem->invokeEvent(event);
        if (event->hasBeenUsed()) return;
    }
}



} // namespace
