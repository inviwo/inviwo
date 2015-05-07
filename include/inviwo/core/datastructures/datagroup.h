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
 *    - DataGroup can never hold any data with owning(referencing[later]) a Data object or a DataGroup object
 *    - DataGroupRepresentation need reference to all Data objects to be created correctly
 *    - DataGroup does not have converters, as the DataGroup objects always can create them self correctly.
 *    - DataGroupRepresentation becomes invalid when a child representations becomes invalid, thus we do not know when it's valid
 *      and we need to call update before we return it from getRepresentation.
 */
class IVW_CORE_API DataGroup : public BaseData {

public:
    DataGroup();
    DataGroup(const DataGroup& rhs);
    DataGroup& operator=(const DataGroup& rhs);
    virtual DataGroup* clone() const = 0;
    virtual ~DataGroup();

    void deinitialize();

    //Representations
    template<typename T>
    const T* getRepresentation() const;
    template<typename T>
    T* getEditableRepresentation();

    template<typename T>
    bool hasRepresentation() const;
    bool hasRepresentations() const;

    void setRepresentationsAsInvalid();

    void clearRepresentations();

protected:
    mutable std::vector<DataGroupRepresentation*> representations_;

private:
    template<typename T>
    T* getRepresentation(bool editable) const; 
};

namespace detail {

template <typename T, typename std::enable_if<
                          !std::is_abstract<T>::value && std::is_default_constructible<T>::value &&
                              std::is_base_of<DataGroupRepresentation, T>::value,
                          int>::type = 0>
T* createGroupRepresentation() {
    return new T();
};

template <typename T, typename std::enable_if<
                          std::is_abstract<T>::value || !std::is_default_constructible<T>::value ||
                              !std::is_base_of<DataGroupRepresentation, T>::value,
                          int>::type = 0>
T* createGroupRepresentation() {
    return nullptr;
};
}

template <typename T>
T* inviwo::DataGroup::getRepresentation(bool editable) const {
    // check if a representation exists and return it
    for (int i = 0; i < static_cast<int>(representations_.size()); ++i) {
        if (T* representation = dynamic_cast<T*>(representations_[i])) {
            representations_[i]->update(editable);
            representations_[i]->setAsValid();
            return representation;
        }
    }

    // no representation exists, create one
    T* representation = detail::createGroupRepresentation<T>();

    if (!representation) {
        throw Exception("Trying to create an invalid group representation: " +
                            std::string(typeid(T).name()) + " for data: " +
                            std::string(typeid(this).name()),
                        IvwContext);
    }

    // Need to cast to be able to call the protected update function, we are friends with base.
    DataGroupRepresentation* baseRepr = static_cast<DataGroupRepresentation*>(representation);
    baseRepr->setOwner(const_cast<DataGroup*>(this));
    baseRepr->update(editable);
    baseRepr->setAsValid();
    representations_.push_back(baseRepr);

    return representation;
}

template<typename T>
const T* DataGroup::getRepresentation() const {
    return static_cast<const T*>(getRepresentation<T>(false));
}

template<typename T>
T* DataGroup::getEditableRepresentation() {
    return getRepresentation<T>(true);
}

template<typename T>
bool DataGroup::hasRepresentation() const {
    for (int i=0; i<static_cast<int>(representations_.size()); i++) {
        if (dynamic_cast<T*>(representations_[i])) return true;
    }
    return false;
}

} // namespace

#endif // IVW_DATAGROUP_H
