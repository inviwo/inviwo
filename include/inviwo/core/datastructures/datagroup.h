/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_DATAGROUP_H
#define IVW_DATAGROUP_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/datagrouprepresentation.h>
#include <type_traits>
#include <typeindex>

namespace inviwo {
/**
 *  \ingroup datastructures
 *
 *  \brief The base class for all DataGroup objects.
 *
 *  It owns or has reference to zero or many Data objects.
 *
 *  It also owns DataGroupRepresentations, which has references to DataRepresentations,
 *  which are owned by the referenced/owned Data objects.
 *
 *  Differences between DataGroup and Data:
 *    - DataGroup can never hold any data with owning(referencing[later]) a Data object or a
 *      DataGroup object
 *    - DataGroupRepresentation need reference to all Data objects to be created correctly
 *    - DataGroup does not have converters, as the DataGroup objects always can create them self
 *      correctly.
 *    - DataGroupRepresentation becomes invalid when a child representations becomes invalid,
 *      thus we do not know when it's valid and we need to call update before we return it
 *      from getRepresentation.
 */
template <typename Self, typename Repr>
class DataGroup {
public:
    using self = Self;
    using repr = Repr;

    virtual DataGroup<Self, Repr>* clone() const = 0;
    virtual ~DataGroup() = default;

    // Representations
    template <typename T>
    const T* getRepresentation() const;
    template <typename T>
    T* getEditableRepresentation();

    template <typename T>
    bool hasRepresentation() const;
    bool hasRepresentations() const;

    void clearRepresentations();

protected:
    DataGroup() = default;
    DataGroup(const DataGroup<Self, Repr>& rhs);
    DataGroup<Self, Repr>& operator=(const DataGroup<Self, Repr>& rhs);

    mutable std::mutex mutex_;
    mutable std::unordered_map<std::type_index, std::shared_ptr<Repr>> representations_;

private:
    template <typename T>
    T* getRepresentation(bool editable) const;
};

namespace detail {
template <typename Repr, typename T,
          typename std::enable_if<!std::is_abstract<T>::value &&
                                      std::is_default_constructible<T>::value &&
                                      std::is_base_of<Repr, T>::value,
                                  int>::type = 0>
std::shared_ptr<T> createGroupRepresentation() {
    return std::make_shared<T>();
}

template <typename Repr, typename T,
          typename std::enable_if<std::is_abstract<T>::value ||
                                      !std::is_default_constructible<T>::value ||
                                      !std::is_base_of<Repr, T>::value,
                                  int>::type = 0>
std::shared_ptr<T> createGroupRepresentation() {
    return std::shared_ptr<T>();
}
}  // namespace detail

template <typename Self, typename Repr>
DataGroup<Self, Repr>::DataGroup(const DataGroup<Self, Repr>& rhs) {
    for (const auto& rep : rhs.representations_) {
        auto clone = std::shared_ptr<Repr>(rep.second->clone());
        clone->setOwner(static_cast<Self*>(this));
        representations_[clone->getTypeIndex()] = std::move(clone);
    }
}

template <typename Self, typename Repr>
DataGroup<Self, Repr>& DataGroup<Self, Repr>::operator=(const DataGroup<Self, Repr>& that) {
    if (this != &that) {
        decltype(representations_) newrepresentation;
        for (const auto& rep : that.representations_) {
            auto clone = std::shared_ptr<Repr>(rep.second->clone());
            clone->setOwner(static_cast<Self*>(this));
            representations_[clone->getTypeIndex()] = std::move(clone);
        }
        std::swap(representations_, newrepresentation);
    }
    return *this;
}

template <typename Self, typename Repr>
bool DataGroup<Self, Repr>::hasRepresentations() const {
    std::unique_lock<std::mutex> lock(mutex_);
    return !representations_.empty();
}

template <typename Self, typename Repr>
void DataGroup<Self, Repr>::clearRepresentations() {
    std::unique_lock<std::mutex> lock(mutex_);
    representations_.clear();
}

template <typename Self, typename Repr>
template <typename T>
T* DataGroup<Self, Repr>::getRepresentation(bool editable) const {
    static_assert(std::is_base_of<Repr, T>::value,
                  "Can only ask for representations that derive from the base representation");

    // check if a representation exists and return it
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = representations_.find(std::type_index(typeid(T)));
    if (it != representations_.end()) {
        it->second->update(editable);
        return dynamic_cast<T*>(it->second.get());
    }

    // no representation exists, create one
    lock.unlock();
    auto representation = detail::createGroupRepresentation<Repr, T>();
    if (!representation) {
        throw Exception(
            "Trying to create an invalid group representation: " + std::string(typeid(T).name()) +
                " for data: " + std::string(typeid(this).name()),
            IVW_CONTEXT);
    }
    representation->setOwner(static_cast<Self*>(const_cast<DataGroup<Self, Repr>*>(this)));
    representation->update(editable);

    lock.lock();
    representations_[representation->getTypeIndex()] = representation;

    return representation.get();
}

template <typename Self, typename Repr>
template <typename T>
const T* DataGroup<Self, Repr>::getRepresentation() const {
    return static_cast<const T*>(getRepresentation<T>(false));
}

template <typename Self, typename Repr>
template <typename T>
T* DataGroup<Self, Repr>::getEditableRepresentation() {
    return getRepresentation<T>(true);
}

template <typename Self, typename Repr>
template <typename T>
bool DataGroup<Self, Repr>::hasRepresentation() const {
    std::unique_lock<std::mutex> lock(mutex_);
    return representations_.find(std::type_index(typeid(T))) != representations_.end();
}

}  // namespace inviwo

#endif  // IVW_DATAGROUP_H
