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

#ifndef IVW_PROPERTYOWNER_H
#define IVW_PROPERTYOWNER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/propertyownerobserver.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/interaction/events/eventlistener.h>

namespace inviwo {

class Processor;
class Event;
class EventProperty;
class CompositeProperty;
class InviwoApplication;

class IVW_CORE_API PropertyOwner : public PropertyOwnerObservable,
                                   public Serializable,
                                   public EventListener {
public:
    using iterator = std::vector<Property*>::iterator;
    using const_iterator = std::vector<Property*>::const_iterator;

    PropertyOwner();
    PropertyOwner(const PropertyOwner& rhs);
    PropertyOwner& operator=(const PropertyOwner& that);
    virtual ~PropertyOwner() = default;

    virtual void addProperty(Property* property, bool owner = true);
    virtual void addProperty(Property& property);

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

    virtual Property* removeProperty(const std::string& identifier);
    virtual Property* removeProperty(Property* property);
    virtual Property* removeProperty(Property& property);
    /**
     * \brief remove property referred to by \p index
     *
     * @param index   index of property to be removed
     * @throw RangeException if \p index is not valid
     */
    virtual Property* removeProperty(size_t index);

    virtual std::vector<std::string> getPath() const;

    const std::vector<Property*>& getProperties() const;
    const std::vector<CompositeProperty*>& getCompositeProperties() const;
    std::vector<Property*> getPropertiesRecursive() const;
    Property* getPropertyByIdentifier(const std::string& identifier,
                                      bool recursiveSearch = false) const;
    Property* getPropertyByPath(const std::vector<std::string>& path) const;
    template <class T>
    std::vector<T*> getPropertiesByType(bool recursiveSearch = false) const;

    size_t size() const;
    Property* operator[](size_t);
    const Property* operator[](size_t) const;
    iterator begin();
    iterator end();
    const_iterator cbegin() const;
    const_iterator cend() const;

    virtual bool isValid() const;
    virtual void setValid();
    InvalidationLevel getInvalidationLevel() const;
    virtual void invalidate(InvalidationLevel invalidationLevel,
                            Property* modifiedProperty = nullptr);

    // Should return the processor that the owner belongs or is.
    // This should be overridden by all subclasses.
    // It is used by the linking.
    virtual Processor* getProcessor();
    virtual const Processor* getProcessor() const;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    void setAllPropertiesCurrentStateAsDefault();
    void resetAllPoperties();

    virtual void invokeEvent(Event* event) override;

    virtual InviwoApplication* getInviwoApplication();

protected:
    // Add the properties belonging the the property owner
    // PropertyOwner do not assume owner ship here since in the most common case these are
    // pointers to members of derived classes.
    std::vector<Property*> properties_;

    // An additional list of properties for which PropertyOwner assumes ownership.
    // I.e. PropertyOwner will take care of deleting them. Usually used for dynamic properties
    // allocated on the heap.
    std::vector<std::unique_ptr<Property>> ownedProperties_;

    // Cached lists of certain property types
    std::vector<EventProperty*> eventProperties_;          //< non-owning references.
    std::vector<CompositeProperty*> compositeProperties_;  //< non-owning references.

private:
    Property* removeProperty(std::vector<Property*>::iterator it);
    bool findPropsForComposites(TxElement*);
    InvalidationLevel invalidationLevel_;
};

template <class T>
std::vector<T*> PropertyOwner::getPropertiesByType(bool recursiveSearch /* = false */) const {
    std::vector<T*> foundProperties;
    for (size_t i = 0; i < properties_.size(); i++) {
        if (dynamic_cast<T*>(properties_[i])) {
            foundProperties.push_back(static_cast<T*>(properties_[i]));
        } else if (recursiveSearch && dynamic_cast<PropertyOwner*>(properties_[i])) {
            std::vector<T*> subProperties =
                dynamic_cast<PropertyOwner*>(properties_[i])->getPropertiesByType<T>(true);
            foundProperties.insert(foundProperties.end(), subProperties.begin(),
                                   subProperties.end());
        }
    }
    return foundProperties;
}

namespace detail {
inline void addPropertyHelper(PropertyOwner&) {}
template <typename... Ts>
void addPropertyHelper(PropertyOwner& owner, Property& p, Ts&... props) {
    owner.addProperty(p);
    addPropertyHelper(owner, props...);
}
}  // namespace detail

template <typename... Ts>
void PropertyOwner::addProperties(Ts&... properties) {
    detail::addPropertyHelper(*this, properties...);
}
}  // namespace inviwo

#endif  // IVW_PROPERTYOWNER_H
