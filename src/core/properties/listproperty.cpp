/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2024 Inviwo Foundation
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

#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/stringconversion.h>

#include <algorithm>
#include <iterator>
#include <set>

namespace inviwo {

std::string_view ListProperty::getClassIdentifier() const { return classIdentifier; }

ListProperty::ListProperty(std::string_view identifier, std::string_view displayName, Document help,
                           std::vector<std::unique_ptr<Property>> prefabs,
                           size_t maxNumberOfElements, ListPropertyUIFlags uiFlags,
                           InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, std::move(help), invalidationLevel, semantics)
    , uiFlags_(uiFlags)
    , maxNumElements_("maxNumberOfElements", maxNumberOfElements)
    , prefabs_{"prefabs", std::move(prefabs)} {}

ListProperty::ListProperty(std::string_view identifier, std::string_view displayName,
                           std::vector<std::unique_ptr<Property>> prefabs,
                           size_t maxNumberOfElements, ListPropertyUIFlags uiFlags,
                           InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : ListProperty(identifier, displayName, Document{}, std::move(prefabs), maxNumberOfElements,
                   uiFlags, invalidationLevel, semantics) {}

ListProperty::ListProperty(std::string_view identifier, std::string_view displayName,
                           std::unique_ptr<Property> prefab, size_t maxNumberOfElements,
                           ListPropertyUIFlags uiFlags, InvalidationLevel invalidationLevel,
                           PropertySemantics semantics)
    : ListProperty(
          identifier, displayName, Document{},
          [&]() {
              std::vector<std::unique_ptr<Property>> tmp;
              tmp.push_back(std::move(prefab));
              return tmp;
          }(),
          maxNumberOfElements, uiFlags, invalidationLevel, semantics) {}

ListProperty::ListProperty(std::string_view identifier, std::string_view displayName,
                           size_t maxNumberOfElements, ListPropertyUIFlags uiFlags,
                           InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : ListProperty(identifier, displayName, Document{}, std::vector<std::unique_ptr<Property>>{},
                   maxNumberOfElements, uiFlags, invalidationLevel, semantics) {}

ListProperty::ListProperty(const ListProperty& rhs)
    : CompositeProperty(rhs)
    , uiFlags_(rhs.uiFlags_)
    , maxNumElements_(rhs.maxNumElements_)
    , prefabs_(rhs.prefabs_) {}

ListProperty* ListProperty::clone() const { return new ListProperty(*this); }

std::string_view ListProperty::getClassIdentifierForWidget() const {
    return ListProperty::classIdentifier;
}

void ListProperty::set(const Property* src) {
    if (const auto listproperty = dynamic_cast<const ListProperty*>(src)) {
        set(listproperty);
    }
}

void ListProperty::set(const ListProperty* src) {
    NetworkLock l(this);
    // check for matching prefab types first
    if (getPrefabIDs() == src->getPrefabIDs()) {
        // TODO: should we sync/consider the UI flags here as well?
        maxNumElements_ = src->maxNumElements_;

        const auto& srcItems = src->getProperties();
        std::vector<Property*> dstItems = getProperties();

        const auto count = std::min(srcItems.size(), dstItems.size());

        auto linkSameId = [&]() {
            for (size_t i = 0; i < count; ++i) {
                if (srcItems[i]->getClassIdentifier() == dstItems[i]->getClassIdentifier()) {
                    dstItems[i]->set(srcItems[i]);
                } else {
                    return i;
                }
            }
            return count;
        };
        auto synced = linkSameId();

        for (size_t i = synced; i < dstItems.size(); ++i) {
            removeProperty(dstItems[i]);
        }
        for (size_t i = synced; i < srcItems.size(); ++i) {
            addProperty(srcItems[i]->clone(), true);
        }
        propertyModified();
    } else {
        log::warn("ListProperty prefab type mismatch. Unable to link");
    }
}

void ListProperty::setMaxNumberOfElements(size_t n) {
    NetworkLock lock(this);
    maxNumElements_ = n;
    if (n > 0) {  // n = 0 means no limit
        // remove superfluous list items
        const bool removedElements = size() > n;
        while (size() > n) {
            removeProperty(size() - 1);
        }
        if (removedElements) {
            propertyModified();
        }
    }
}

size_t ListProperty::getMaxNumberOfElements() const { return maxNumElements_; }

void ListProperty::clear() {
    NetworkLock l(this);
    CompositeProperty::clear();
    propertyModified();
}

Property* ListProperty::constructProperty(size_t prefabIndex) {
    if (prefabIndex >= prefabs_.value.size()) {
        throw RangeException("Invalid prefab index " + std::to_string(prefabIndex) + " (" +
                                 std::to_string(prefabs_.value.size()) + " prefabs)",
                             IVW_CONTEXT);
    }

    if ((maxNumElements_ == size_t{0}) || (size() + 1 < maxNumElements_)) {
        auto property = prefabs_.value[prefabIndex]->clone();
        IVW_ASSERT(
            property->getClassIdentifier() == prefabs_.value[prefabIndex]->getClassIdentifier(),
            "Class identifier mismatch after cloning, does your property implement clone?");
        property->setSerializationMode(PropertySerializationMode::All);
        property->setIdentifier(util::findUniqueIdentifier(
            property->getIdentifier(),
            [&](std::string_view id) { return getPropertyByIdentifier(id) == nullptr; }, ""));

        // if prefab has a trailing number in its display name, use number of identifier
        std::string displayName = property->getDisplayName();
        auto it = std::find_if(displayName.rbegin(), displayName.rend(),
                               [](char c) { return !std::isdigit(c); });
        std::string baseName = trim(std::string{displayName.begin(), it.base()});
        if (it.base() != displayName.end()) {
            // extract number from identifier
            std::string identifier = property->getIdentifier();
            auto itIdentifier = std::find_if(identifier.rbegin(), identifier.rend(),
                                             [](char c) { return !std::isdigit(c); });
            std::string number(itIdentifier.base(), identifier.end());
            if (!number.empty()) {
                displayName = baseName + " " + number;
            } else {
                displayName = baseName + " 1";
            }
            property->setDisplayName(displayName);
        }

        CompositeProperty::addProperty(property, true);
        invalidate(Property::getInvalidationLevel());
        return property;
    } else {
        log::error("Maximum number of list entries reached ({})", this->getDisplayName());
    }
    return nullptr;
}

void ListProperty::addProperty(Property* property, bool owner) {
    insertProperty(getProperties().size(), property, owner);
}

void ListProperty::addProperty(Property& property) {
    insertProperty(getProperties().size(), &property, false);
}

void ListProperty::insertProperty(size_t index, Property* property, bool owner) {
    if (!util::contains_if(prefabs_.value.properties,
                           [&, id = property->getClassIdentifier()](auto& elem) {
                               return elem->getClassIdentifier() == id;
                           })) {
        throw Exception(IVW_CONTEXT, "Unsupported property type, no prefab matching `{}`.",
                        property->getClassIdentifier());
    }

    if ((maxNumElements_ == size_t{0}) || (size() + 1 < maxNumElements_)) {
        property->setSerializationMode(PropertySerializationMode::All);
        CompositeProperty::insertProperty(index, property, owner);
        invalidate(Property::getInvalidationLevel());
    } else {
        log::error("Maximum number of list entries reached ({})", this->getDisplayName());
    }
}

void ListProperty::insertProperty(size_t index, Property& property) {
    insertProperty(index, &property, false);
}

Property* ListProperty::removeProperty(std::string_view identifier) {
    auto result = CompositeProperty::removeProperty(identifier);
    invalidate(Property::getInvalidationLevel());
    return result;
}

Property* ListProperty::removeProperty(Property* property) {
    auto result = CompositeProperty::removeProperty(property);
    invalidate(Property::getInvalidationLevel());
    return result;
}

Property* ListProperty::removeProperty(Property& property) {
    auto result = CompositeProperty::removeProperty(property);
    invalidate(Property::getInvalidationLevel());
    return result;
}

Property* ListProperty::removeProperty(size_t index) {
    auto result = CompositeProperty::removeProperty(index);
    invalidate(Property::getInvalidationLevel());
    return result;
}

bool ListProperty::move(Property* property, size_t newIndex) {
    auto result = CompositeProperty::move(property, newIndex);
    invalidate(Property::getInvalidationLevel());
    return result;
}

size_t ListProperty::getPrefabCount() const { return prefabs_.value.size(); }

void ListProperty::addPrefab(std::unique_ptr<Property> p) {
    prefabs_.value.properties.emplace_back(std::move(p));
}

const std::vector<std::unique_ptr<Property>>& ListProperty::getPrefabs() const {
    return prefabs_.value.properties;
}

ListPropertyUIFlags ListProperty::getUIFlags() const { return uiFlags_; }

ListProperty& ListProperty::setCurrentStateAsDefault() {
    prefabs_.setAsDefault();
    maxNumElements_.setAsDefault();
    CompositeProperty::setCurrentStateAsDefault();
    return *this;
}
ListProperty& ListProperty::resetToDefaultState() {
    prefabs_.reset();
    maxNumElements_.reset();
    CompositeProperty::resetToDefaultState();
    return *this;
}
bool ListProperty::isDefaultState() const {
    return CompositeProperty::isDefaultState() && maxNumElements_.isDefault() &&
           prefabs_.isDefault();
}
bool ListProperty::needsSerialization() const {
    return CompositeProperty::needsSerialization() || !maxNumElements_.isDefault() ||
           !prefabs_.isDefault();
}

void ListProperty::serialize(Serializer& s) const {
    maxNumElements_.serialize(s);
    prefabs_.serialize(s);
    CompositeProperty::serialize(s);
}

void ListProperty::deserialize(Deserializer& d) {
    maxNumElements_.deserialize(d);
    prefabs_.deserialize(d);
    CompositeProperty::deserialize(d);
}

std::set<std::string> ListProperty::getPrefabIDs() const {
    std::set<std::string> ids;
    for (const auto& elem : prefabs_.value.properties) {
        ids.emplace(elem->getClassIdentifier());
    }

    return ids;
}

}  // namespace inviwo
