/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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
}

PropertyOwner& PropertyOwner::operator=(const PropertyOwner& that) {
    if (this != &that) {
        invalidationLevel_ = that.invalidationLevel_;
        properties_.clear();
    }
    return *this;
}

PropertyOwner::~PropertyOwner() {
    properties_.clear();
}

void PropertyOwner::addProperty(Property* property) {
    ivwAssert(getPropertyByIdentifier(property->getIdentifier())==0, "Property already exist");
    notifyObserversWillAddProperty(property, properties_.size());
    properties_.push_back(property);
    property->setOwner(this);
    if (dynamic_cast<EventProperty*>(property)) {
        eventProperties_.push_back(static_cast<EventProperty*>(property));
    }
    if (dynamic_cast<CompositeProperty*>(property)) {
        compositeProperties_.push_back(static_cast<CompositeProperty*>(property));
    }
    notifyObserversDidAddProperty(property, properties_.size()-1);
}

void PropertyOwner::addProperty(Property& property) {
    addProperty(&property);
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
    Property* prop = NULL;
    if (it != properties_.end()) {
        prop = *it;
        size_t index = std::distance(properties_.begin(), it);
        notifyObserversWillRemoveProperty(prop, index);

        std::vector<EventProperty*>::iterator 
            eit = std::find(eventProperties_.begin(),eventProperties_.end(), *it);
        if (eit != eventProperties_.end()) eventProperties_.erase(eit);
        
        std::vector<CompositeProperty*>::iterator
            cit = std::find(compositeProperties_.begin(),compositeProperties_.end(), *it);
        if (cit != compositeProperties_.end()) compositeProperties_.erase(cit);

        properties_.erase(it);
        notifyObserversDidRemoveProperty(prop, index);
    }
    return prop;
}

Property* PropertyOwner::getPropertyByIdentifier(const std::string& identifier,
                                                 bool recursiveSearch) const {
    for (size_t i = 0; i < properties_.size(); i++) {
        if (properties_[i]->getIdentifier() == identifier) return properties_[i];
    }
    if (recursiveSearch) {
        for (size_t i = 0; i < compositeProperties_.size(); i++) {
            Property* p = compositeProperties_[i]->getPropertyByIdentifier(identifier, true);
            if (p) return p;
        }
    }
    return NULL;
}

Property* PropertyOwner::getPropertyByPath(const std::vector<std::string>& path) const {
    Property* property = getPropertyByIdentifier(path[0]);
    if (property) {
        size_t i = 1;
        while (path.size() > i) {
            CompositeProperty* comp = dynamic_cast<CompositeProperty*>(property);
            if (comp) {
                property = comp->getPropertyByIdentifier(path[i]);
                if (!property) return NULL;
            } else {
                return NULL;
            }
            ++i;
        }
        return property;
    }
    return NULL;
}

void PropertyOwner::setValid() {
    for (size_t i=0; i<properties_.size(); i++)
        properties_[i]->setPropertyModified(false);

    invalidationLevel_ = VALID;
}

void PropertyOwner::invalidate(InvalidationLevel invalidationLevel,
                               Property* modifiedProperty) {
    IVW_UNUSED_PARAM(modifiedProperty);
    invalidationLevel_ = std::max(invalidationLevel_, invalidationLevel);
}

void PropertyOwner::serialize(IvwSerializer& s) const {
    std::map<std::string, Property*> propertyMap;

    for (std::vector<Property*>::const_iterator it = properties_.begin(); it != properties_.end(); ++it)
        propertyMap[(*it)->getIdentifier()] = *it;

    s.serialize("Properties", propertyMap, "Property");
}

void PropertyOwner::deserialize(IvwDeserializer& d) {
    /* 1) Vector deserialization does not allow
    *     specification of comparison attribute string.
    *  2) But Map deserialization does allow
    *     specification of comparision attribute string.
    *     (eg. "identifier" in this case).
    *  3) Hence map deserialization is preferred here.
    *  4) TODO: Vector can be made to behave like Map.
    *           But then it necessitates passing of two extra arguments.
    *           And they are list of attribute values, comparison attribute string.
    *           eg., list of identifier for each property and "identifier"
    *
    */


    NodeVersionConverter<PropertyOwner> tvc(this, &PropertyOwner::findPropsForComposites);
    d.convertVersion(&tvc);


    std::map<std::string, Property*> propertyMap;

    for (std::vector<Property*>::const_iterator it = properties_.begin(); it != properties_.end(); ++it)
        propertyMap[(*it)->getIdentifier()] = *it;

    d.deserialize("Properties", propertyMap, "Property", "identifier") ;
}

bool PropertyOwner::findPropsForComposites(TxElement* node) {
    std::vector<const CompositeProperty*> props;
    for (std::vector<Property*>::const_iterator it = properties_.begin(); it != properties_.end(); ++it) {
        CompositeProperty* cp = dynamic_cast<CompositeProperty*>(*it);
        if(cp){
            props.push_back(cp);
        }
    }
    return util::xmlFindMatchingSubPropertiesForComposites(node, props);
}

void PropertyOwner::setAllPropertiesCurrentStateAsDefault(){
    for (std::vector<Property*>::iterator it = properties_.begin(); it != properties_.end(); ++it)
        (*it)->setCurrentStateAsDefault();
}

void PropertyOwner::resetAllPoperties(){
    for (std::vector<Property*>::iterator it = properties_.begin(); it != properties_.end(); ++it)
        (*it)->resetToDefaultState();
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

void PropertyOwner::invokeInteractionEvent(Event* event) {
    for (std::vector<EventProperty*>::iterator it = eventProperties_.begin();
         it != eventProperties_.end(); ++it) {
        if ((*it)->getEvent()->matching(event)) {
            (*it)->getAction()->invoke(event);
            if (event->hasBeenUsed()) return;
        }
    }
    for (std::vector<CompositeProperty*>::iterator it = compositeProperties_.begin();
         it != compositeProperties_.end(); ++it) {
        (*it)->invokeInteractionEvent(event);
        if (event->hasBeenUsed()) return;
    }
}



} // namespace
