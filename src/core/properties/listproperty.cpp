/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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
#include <inviwo/core/network/networklock.h>

#include <algorithm>
#include <iterator>
#include <set>

namespace inviwo {

const std::string ListProperty::classIdentifier = "org.inviwo.ListProperty";
std::string ListProperty::getClassIdentifier() const { return classIdentifier; }

namespace detail {

std::vector<std::unique_ptr<Property>> clonePropertyVector(
    const std::vector<std::unique_ptr<Property>>& v) {
    std::vector<std::unique_ptr<Property>> result;
    result.reserve(v.size());
    std::transform(v.begin(), v.end(), std::back_inserter(result),
                   [](auto& p) { return std::unique_ptr<Property>(p->clone()); });

    return result;
}

}  // namespace detail

ListProperty::ListProperty(std::string identifier, const std::string& displayName,
                           size_t maxNumberOfElements, ListPropertyUIFlags uiFlags,
                           InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , uiFlags_(uiFlags)
    , maxNumElements_("maxNumberOfElements", maxNumberOfElements) {}

ListProperty::ListProperty(std::string identifier, const std::string& displayName,
                           std::vector<std::unique_ptr<Property>> prefabs,
                           size_t maxNumberOfElements, ListPropertyUIFlags uiFlags,
                           InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : ListProperty(identifier, displayName, maxNumberOfElements, uiFlags, invalidationLevel,
                   semantics) {
    prefabs_ = std::move(prefabs);
}

ListProperty::ListProperty(std::string identifier, const std::string& displayName,
                           std::unique_ptr<Property> prefab, size_t maxNumberOfElements,
                           ListPropertyUIFlags uiFlags, InvalidationLevel invalidationLevel,
                           PropertySemantics semantics)
    : ListProperty(identifier, displayName, maxNumberOfElements, uiFlags, invalidationLevel,
                   semantics) {
    prefabs_.emplace_back(std::move(prefab));
}

ListProperty::ListProperty(const ListProperty& rhs)
    : CompositeProperty(rhs)
    , uiFlags_(rhs.uiFlags_)
    , maxNumElements_(rhs.maxNumElements_)
    , prefabs_(detail::clonePropertyVector(rhs.prefabs_)) {}

ListProperty* ListProperty::clone() const { return new ListProperty(*this); }

std::string ListProperty::getClassIdentifierForWidget() const {
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
        LogWarn("ListProperty prefab type mismatch. Unable to link");
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
    while (size()) {
        removeProperty(static_cast<size_t>(0));
    }
    propertyModified();
}

Property* ListProperty::constructProperty(size_t prefabIndex) {
    if (prefabIndex >= prefabs_.size()) {
        throw RangeException("Invalid prefab index " + std::to_string(prefabIndex) + " (" +
                                 std::to_string(prefabs_.size()) + " prefabs)",
                             IVW_CONTEXT);
    }

    if ((maxNumElements_ == 0) || (size() + 1 < maxNumElements_)) {
        auto property = prefabs_[prefabIndex]->clone();
        property->setSerializationMode(PropertySerializationMode::All);
        property->setIdentifier(util::findUniqueIdentifier(
            property->getIdentifier(),
            [&](const std::string& id) { return getPropertyByIdentifier(id) == nullptr; }, ""));

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
        propertyModified();
        return property;
    } else {
        LogError("Maximum number of list entries reached (" << this->getDisplayName() << ")");
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
    if (!util::contains_if(prefabs_, [&, id = property->getClassIdentifier()](auto& elem) {
            return elem->getClassIdentifier() == id;
        })) {
        throw Exception("Unsupported property type, no prefab matching `" +
                            property->getClassIdentifier() + "`.",
                        IVW_CONTEXT);
    }

    if ((maxNumElements_ == 0) || (size() + 1 < maxNumElements_)) {
        property->setSerializationMode(PropertySerializationMode::All);
        CompositeProperty::insertProperty(index, property, owner);
        propertyModified();
    } else {
        LogError("Maximum number of list entries reached (" << this->getDisplayName() << ")");
    }
}

void ListProperty::insertProperty(size_t index, Property& property) {
    insertProperty(index, &property, false);
}

Property* ListProperty::removeProperty(const std::string& identifier) {
    auto result = CompositeProperty::removeProperty(identifier);
    propertyModified();
    return result;
}

Property* ListProperty::removeProperty(Property* property) {
    auto result = CompositeProperty::removeProperty(property);
    propertyModified();
    return result;
}

Property* ListProperty::removeProperty(Property& property) {
    auto result = CompositeProperty::removeProperty(property);
    propertyModified();
    return result;
}

Property* ListProperty::removeProperty(size_t index) {
    auto result = CompositeProperty::removeProperty(index);
    propertyModified();
    return result;
}

size_t ListProperty::getPrefabCount() const { return prefabs_.size(); }

void ListProperty::addPrefab(std::unique_ptr<Property> p) { prefabs_.emplace_back(std::move(p)); }

const std::vector<std::unique_ptr<Property>>& ListProperty::getPrefabs() const { return prefabs_; }

ListPropertyUIFlags ListProperty::getUIFlags() const { return uiFlags_; }

void ListProperty::serialize(Serializer& s) const {
    maxNumElements_.serialize(s, PropertySerializationMode::All);
    CompositeProperty::serialize(s);
}

void ListProperty::deserialize(Deserializer& d) {
    maxNumElements_.deserialize(d, PropertySerializationMode::All);
    CompositeProperty::deserialize(d);
}

std::set<std::string> ListProperty::getPrefabIDs() const {
    std::set<std::string> ids;
    for (auto& elem : prefabs_) {
        ids.insert(elem->getClassIdentifier());
    }

    return ids;
}

}  // namespace inviwo
