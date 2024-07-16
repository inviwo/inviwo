/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/properties/propertyownerobserver.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/interaction/events/eventlistener.h>
#include <inviwo/core/algorithm/markdown.h>

#include <vector>
#include <memory>
#include <string_view>

namespace inviwo {

class Processor;
class Event;
class EventProperty;
class CompositeProperty;
class InviwoApplication;
class NetworkVisitor;

class IVW_CORE_API PropertyOwner : public PropertyOwnerObservable,
                                   public virtual Serializable,
                                   public EventListener {
public:
    using iterator = std::vector<Property*>::iterator;
    using const_iterator = std::vector<Property*>::const_iterator;

    /**
     * \brief Removes all properties and notifies its observers of the removal.
     */
    virtual ~PropertyOwner();

    virtual void addProperty(Property* property, bool owner = true);
    virtual void addProperty(Property& property);
    virtual Property* addProperty(std::unique_ptr<Property> property);

    template <typename... Ts>
    void addProperties(Ts&... properties);

    /**
     * \brief insert property \p property at position \p index
     * If \p index is not valid, the property is appended.
     *
     * @param index      insertion point for property
     * @param property   property to be inserted
     * @param owner      PropertyOwner will take ownership of the property if true
     */
    virtual void insertProperty(size_t index, Property* property, bool owner = true);

    /**
     * \brief insert property \p property at position \p index
     * If \p index is not valid, the property is appended.
     *
     * @param index      insertion point for property
     * @param property   property to be inserted
     */
    virtual void insertProperty(size_t index, Property& property);

    /**
     * \brief insert property \p property at position \p index
     * If \p index is not valid, the property is appended.
     *
     * @param index      insertion point for property
     * @param property   property to be inserted
     */
    virtual Property* insertProperty(size_t index, std::unique_ptr<Property> property);

    virtual Property* removeProperty(std::string_view identifier);
    virtual Property* removeProperty(Property* property);
    virtual Property* removeProperty(Property& property);
    /**
     * \brief remove property referred to by \p index
     *
     * @param index   index of property to be removed
     * @throw RangeException if \p index is not valid
     */
    virtual Property* removeProperty(size_t index);

    virtual bool move(Property* property, size_t newIndex);

    /**
     * @brief Remove all properties;
     * Owned properties will be deleted
     */
    virtual void clear();

    virtual const std::string& getIdentifier() const;

    const std::vector<Property*>& getProperties() const;
    const std::vector<CompositeProperty*>& getCompositeProperties() const;
    std::vector<Property*> getPropertiesRecursive() const;
    std::vector<Property*>& getPropertiesRecursive(std::vector<Property*>& destination) const;
    Property* getPropertyByIdentifier(std::string_view identifier,
                                      bool recursiveSearch = false) const;

    Property* getPropertyByPath(std::string_view path) const;

    template <class T>
    std::vector<T*> getPropertiesByType(bool recursiveSearch = false) const;

    bool empty() const;
    size_t size() const;
    Property* operator[](size_t);
    const Property* operator[](size_t) const;

    const_iterator find(Property* property) const;

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;

    virtual bool isValid() const;
    virtual void setValid();
    InvalidationLevel getInvalidationLevel() const;
    virtual void invalidate(InvalidationLevel invalidationLevel, Property* modifiedProperty);

    // Should return the processor that the owner belongs or is.
    // This should be overridden by all subclasses.
    // It is used by the linking.
    virtual Processor* getProcessor();
    virtual const Processor* getProcessor() const;

    virtual const PropertyOwner* getOwner() const;
    virtual PropertyOwner* getOwner();

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    virtual void setAllPropertiesCurrentStateAsDefault();
    virtual void resetAllProperties();

    virtual void invokeEvent(Event* event) override;

    virtual InviwoApplication* getInviwoApplication() = 0;

protected:
    PropertyOwner();
    PropertyOwner(const PropertyOwner& rhs);
    PropertyOwner& operator=(const PropertyOwner& that) = delete;

    // Add the properties belonging the property owner
    // PropertyOwner do not assume owner ship here since in the most common case these are
    // pointers to members of derived classes.
    std::vector<Property*> properties_;

    // Cached lists of certain property types
    std::vector<EventProperty*> eventProperties_;          //< non-owning references.
    std::vector<CompositeProperty*> compositeProperties_;  //< non-owning references.

    // An additional list of properties for which PropertyOwner assumes ownership.
    // I.e. PropertyOwner will take care of deleting them. Usually used for dynamic properties
    // allocated on the heap.
    std::vector<std::unique_ptr<Property>> ownedProperties_;

    void forEachProperty(std::function<void(Property&)> callback,
                         bool recursiveSearch = false) const;

private:
    Property* removeProperty(std::vector<Property*>::iterator it);
    bool findPropsForComposites(TxElement*);
    InvalidationLevel invalidationLevel_;
};

template <class T>
std::vector<T*> PropertyOwner::getPropertiesByType(bool recursiveSearch) const {
    std::vector<T*> foundProperties;
    forEachProperty(
        [&](Property& property) {
            if (auto p = dynamic_cast<T*>(&property)) {
                foundProperties.push_back(p);
            }
        },
        recursiveSearch);

    return foundProperties;
}

template <typename... Ts>
void PropertyOwner::addProperties(Ts&... properties) {
    (addProperty(properties), ...);
}
}  // namespace inviwo
