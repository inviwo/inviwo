/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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
#include <inviwo/core/datastructures/data.h>
#include <inviwo/core/datastructures/datagrouprepresentation.h>
#include <type_traits>

namespace inviwo {
/** 
 *  \class DataGroup
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
template <typename Repr>
class DataGroup : public BaseData {

public:
    DataGroup() = default;
    DataGroup(const DataGroup<Repr>& rhs);
    DataGroup<Repr>& operator=(const DataGroup<Repr>& rhs);
    virtual DataGroup<Repr>* clone() const override = 0;
    virtual ~DataGroup() = default;

    //Representations
    template<typename T>
    const T* getRepresentation() const;
    template<typename T>
    T* getEditableRepresentation();

    template<typename T>
    bool hasRepresentation() const;
    bool hasRepresentations() const;

    void clearRepresentations();

protected:
    mutable std::mutex mutex_;
    mutable std::vector<std::shared_ptr<Repr>> representations_;

private:
    template<typename T>
    T* getRepresentation(bool editable) const; 
};

namespace detail {

template <typename T, typename std::enable_if<
                          !std::is_abstract<T>::value && std::is_default_constructible<T>::value &&
                              std::is_base_of<DataGroupRepresentation, T>::value,
                          int>::type = 0>
std::shared_ptr<T> createGroupRepresentation() {
    return std::make_shared<T>();
};

template <typename T, typename std::enable_if<
                          std::is_abstract<T>::value || !std::is_default_constructible<T>::value ||
                              !std::is_base_of<DataGroupRepresentation, T>::value,
                          int>::type = 0>
std::shared_ptr<T> createGroupRepresentation() {
    return std::shared_ptr<T>();
};
}

template <typename Repr>
DataGroup<Repr>::DataGroup(const DataGroup<Repr>& rhs) : BaseData(rhs) {}

template <typename Repr>
DataGroup<Repr>& DataGroup<Repr>::operator=(const DataGroup<Repr>& that) {
    if (this != &that) {
        BaseData::operator=(that);
        clearRepresentations();
    }
    return *this;
}

template <typename Repr>
bool DataGroup<Repr>::hasRepresentations() const {
    std::unique_lock<std::mutex> lock(mutex_);
    return !representations_.empty();
}

template <typename Repr>
void DataGroup<Repr>::clearRepresentations() {
    std::unique_lock<std::mutex> lock(mutex_);
    representations_.clear();
}

template <typename Repr>
template <typename T>
T* inviwo::DataGroup<Repr>::getRepresentation(bool editable) const {
    // check if a representation exists and return it
    std::unique_lock<std::mutex> lock(mutex_);
    for (size_t i = 0; i < representations_.size(); ++i) {
        if (auto representation = std::dynamic_pointer_cast<T>(representations_[i])) {
            auto baseRepr = std::static_pointer_cast<DataGroupRepresentation>(representation);
            baseRepr->update(editable);
            return representation.get();
        }
    }

    // no representation exists, create one
    auto representation = detail::createGroupRepresentation<T>();

    if (!representation) {
        throw Exception("Trying to create an invalid group representation: " +
                            std::string(typeid(T).name()) + " for data: " +
                            std::string(typeid(this).name()),
                        IvwContext);
    }

    // Need to cast to be able to call the protected update function, we are friends with base.
    auto baseRepr = std::static_pointer_cast<DataGroupRepresentation>(representation);
    baseRepr->setOwner(const_cast<DataGroup*>(this));
    baseRepr->update(editable);
    representations_.push_back(representation);

    return representation.get();
}

template <typename Repr>
template<typename T>
const T* DataGroup<Repr>::getRepresentation() const {
    return static_cast<const T*>(getRepresentation<T>(false));
}

template <typename Repr>
template<typename T>
T* DataGroup<Repr>::getEditableRepresentation() {
    return getRepresentation<T>(true);
}

template <typename Repr>
template<typename T>
bool DataGroup<Repr>::hasRepresentation() const {
    for (size_t i=0; i<representations_.size(); i++) {
        if (std::dynamic_pointer_cast<T>(representations_[i])) return true;
    }
    return false;
}

} // namespace

#endif // IVW_DATAGROUP_H
