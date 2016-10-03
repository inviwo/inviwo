/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2016 Inviwo Foundation
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

#ifndef IVW_DATA_H
#define IVW_DATA_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/datarepresentation.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/representationconverterfactory.h>
#include <inviwo/core/metadata/metadatamap.h>
#include <inviwo/core/metadata/metadataowner.h>

namespace inviwo {

/**
 *	\defgroup datastructures Data Structures
 *  \brief Various data structures used in inviwo
 */

class IVW_CORE_API BaseData : public MetaDataOwner {
public:
    BaseData() = default;
    BaseData(const BaseData& rhs) = default;
    BaseData& operator=(const BaseData& rhs) = default;
    virtual BaseData* clone() const = 0;
    virtual ~BaseData() = default;
    virtual std::string getDataInfo() const;
};

/**
 * \ingroup datastructures
 *
 * \brief The base class for all data objects.
 *
 * Data is a handle class and, by it self does not have any data, it only stores metadata, and a
 * list of representations. The representations is were the actually data is stored. A Volume can
 * have a set of different representations, for example a VolumeRAM representation and a VolumeGL
 * representation. At any time at least one of the representations should be in a valid state. When
 * ever we want to access the data of a volume we will ask the volume for a representation of some
 * kind, and the handle is then responsible to try and provide that to us. If the requested
 * representation is valid the handle can just return the that representation. If not, it will have
 * to find a valid representation and try to either create the representation we wanted from the
 * valid representation, if there was no representation of the kind we asked for around. Or if there
 * is a invalid representation around, update that representation with the valid representation. 
 *
 * Call getRepresentation to retrieve the data:
 * \code{.cpp}
 *      // Create a Buffer with a BufferRAM representation
 *      auto buffer = util::makeBuffer<vec2>(
 *      { {0.0f, 1.0f}, {-1.0f, -1.0f}, {1.0f, -1.0f} });
 *      // Retrieve a read-only representation of the data
 *      auto bufferRAM = buffer.getRepresentation<BufferRAM>();
 *      // The data will be transferred to the GPU if not already there.
 *      auto bufferGL = buffer.getRepresentation<BufferGL>();
 * \endcode
 *
 * Requirements:
 *      1. Copy constructor for deep copy
 *      2. Assignment operator for deep copy
 *      3. Clone pattern
 *
 * 1 and 2 are needed to be a vaild member type of std::vector.
 * 3 is needed for the factory pattern, 3 should be implemented using 1.
 *
 *
 *
 * @note Do not use the same representation in different Data objects.
 * This can cause inconsistencies since the Data objects cannot know if
 * another one has edited the representation.
 * @see Representation and RepresentationConverter
 */
template <class Repr>
class Data : public BaseData {
public:
    Data(const DataFormatBase*);
    Data(const Data<Repr>& rhs);
    Data<Repr>& operator=(const Data<Repr>& rhs);
    virtual Data<Repr>* clone() const = 0;
    virtual ~Data() = default;

    /**
     * Get a representation of type T. If there already is a valid representation of type T, just
     * return it, if it's invalid use the last valid representation to update it so it will be
     * valid. It there is no representation of type T, create it from the last valid representation.
     * If there are no representations create a default representation and from that create a
     * representation of type T.
     */
    template <typename T>
    const T* getRepresentation() const;

    /**
     * Get an editable representation. This will invalidate all other representations.
     * They will now have to be updated from this one before use.
     * @see getRepresentation and invalidateAllOther
     */
    template <typename T>
    T* getEditableRepresentation();

    /**
    * Check if a specific representation type exists.
    * Example:
    * \code{.cpp}
    *     hasRepresentation<BufferRAM>();
    * \endcode
    * @return true if existing, false otherwise.
    */
    template <typename T>
    bool hasRepresentation() const;

    /**
     * Check if the Data object has any representation.
     * @return true if any representation exist, false otherwise.
     */
    bool hasRepresentations() const;

    /**
    * Add the representation and set it as last valid.
    * The owner of the representation will be set to this object.
    * @note A representation can only be added to one Data object.
    * @param representation The representation to add
    */
    void addRepresentation(std::shared_ptr<Repr> representation);

    /**
     * Remove representation from data object.
     * This will delete the representation, thus rendering the representation pointer invalid.
     * @param representation The representation to remove
     */
    void removeRepresentation(const Repr* representation);

    /**
     * Remove all other representations.
     * This will delete all representations except the one passed in.
     * @param representation The representation to keep
     */
    void removeOtherRepresentations(const Repr* representation);
    /**
     * Delete all representations.
     */
    void clearRepresentations();

    /**
     * This call will make all other representations invalid. You need to call this function if
     * you are modifying a representation directly without calling getEditableRepresentation.
     * getEditableRepresentation will automatically invalidate all other representations.
     * @see getEditableRepresentation
     */
    void invalidateAllOther(const Repr* repr);

    /**
    * Set the format of the data.
    * @see DataFormatBase
    * @param format The format of the data.
    */
    void setDataFormat(const DataFormatBase* format);
    const DataFormatBase* getDataFormat() const;

protected:
    virtual std::shared_ptr<Repr> createDefaultRepresentation() const = 0;
    template <typename T>
    const T* getValidRepresentation() const;
    void copyRepresentationsTo(Data* targetData) const;

    mutable std::mutex mutex_;
    mutable std::unordered_map<std::type_index, std::shared_ptr<Repr>> representations_;
    // A pointer to the the most recently updated representation. Makes updates and creation faster.
    mutable std::shared_ptr<Repr> lastValidRepresentation_;
    const DataFormatBase* dataFormatBase_;
};

template <class Repr>
Data<Repr>::Data(const DataFormatBase* format)
    : BaseData(), lastValidRepresentation_(), dataFormatBase_(format) {}

template <class Repr>
Data<Repr>::Data(const Data<Repr>& rhs)
    : BaseData(rhs), lastValidRepresentation_(), dataFormatBase_(rhs.dataFormatBase_) {
    rhs.copyRepresentationsTo(this);
}

template <class Repr>
Data<Repr>& Data<Repr>::operator=(const Data<Repr>& that) {
    if (this != &that) {
        BaseData::operator=(that);
        that.copyRepresentationsTo(this);
        dataFormatBase_ = that.dataFormatBase_;
    }

    return *this;
}

template <class Repr>
template <typename T>
const T* Data<Repr>::getRepresentation() const {
    if (!hasRepresentations()) {
        auto repr = createDefaultRepresentation();
        if (!repr) throw Exception("Failed to create default representation", IvwContext);
        const_cast<Data*>(this)->addRepresentation(repr);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        auto it = representations_.find(std::type_index(typeid(T)));
        if (it != representations_.end() && it->second->isValid()) {
            lastValidRepresentation_ = it->second;
            return dynamic_cast<const T*>(lastValidRepresentation_.get());
        } else {
            return getValidRepresentation<T>();
        }
    }
}

template <class Repr>
template <typename T>
const T* Data<Repr>::getValidRepresentation() const {
    auto factory = InviwoApplication::getPtr()->getRepresentationConverterFactory();
    auto package = factory->getRepresentationConverter(lastValidRepresentation_->getTypeIndex(),
                                                       std::type_index(typeid(T)));

    if (package) {
        for (auto converter : package->getConverters()) {
            std::shared_ptr<Repr> result;

            auto dest = converter->getConverterID().second;
            auto it = representations_.find(dest);
            if (it != representations_.end()) {  // Next repr. already exist, just update it
                result = it->second;
                converter->update(lastValidRepresentation_, result);
            } else {  // No representation found, create it
                // TODO remove static cast here with template factory...
                result = std::dynamic_pointer_cast<Repr>(
                    converter->createFrom(lastValidRepresentation_));
                if (!result) throw ConverterException("Converter failed to create", IvwContext);
                result->setOwner(const_cast<Data*>(this));
                representations_[result->getTypeIndex()] = result;
            }
            result->setValid(true);
            lastValidRepresentation_ = result;
        }
        return dynamic_cast<const T*>(lastValidRepresentation_.get());
    } else {
        throw ConverterException("Found no converters", IvwContext);
    }
}

template <class Repr>
template <typename T>
T* Data<Repr>::getEditableRepresentation() {
    auto repr = getRepresentation<T>();
    invalidateAllOther(repr);
    return const_cast<T*>(repr);
}

template <class Repr>
template <typename T>
bool Data<Repr>::hasRepresentation() const {
    std::unique_lock<std::mutex> lock(mutex_);
    return util::has_key(representations_, std::type_index(typeid(T)));
}

template <class Repr>
void Data<Repr>::invalidateAllOther(const Repr* repr) {
    bool found = false;
    std::unique_lock<std::mutex> lock(mutex_);
    for (auto& elem : representations_) {
        if (elem.second.get() != repr) {
            elem.second->setValid(false);
        } else {
            found = true;
            elem.second->setValid(true);
            lastValidRepresentation_ = elem.second;
        }
    }
    if(!found) throw Exception("Called with representation not in representations.", IvwContext);
}

template <class Repr>
void Data<Repr>::clearRepresentations() {
    std::unique_lock<std::mutex> lock(mutex_);
    representations_.clear();
}

template <class Repr>
void Data<Repr>::copyRepresentationsTo(Data* targetData) const {
    targetData->clearRepresentations();

    if (lastValidRepresentation_) {
        auto rep = std::shared_ptr<Repr>(lastValidRepresentation_->clone());
        targetData->addRepresentation(rep);
    }
}

template <class Repr>
void Data<Repr>::addRepresentation(std::shared_ptr<Repr> repr) {
    repr->setValid(true);
    repr->setOwner(this);
    {
        std::unique_lock<std::mutex> lock(mutex_);
        representations_[repr->getTypeIndex()] = repr;
        lastValidRepresentation_ = repr;
    }
}

template <class Repr>
void Data<Repr>::removeRepresentation(const Repr* representation) {
    std::unique_lock<std::mutex> lock(mutex_);

    for (auto& elem : representations_) {
        if (elem.second.get() == representation) {
            representations_.erase(elem.first);
            break;
        }
    }

    if (lastValidRepresentation_.get() == representation) {
        lastValidRepresentation_.reset();

        for (auto& elem : representations_) {
            if (elem.second->isValid()) {
                lastValidRepresentation_ = elem.second;
            }
        }
    }
}

template <class Repr>
void Data<Repr>::removeOtherRepresentations(const Repr* representation) {
    std::unique_lock<std::mutex> lock(mutex_);

    std::unordered_map<std::type_index, std::shared_ptr<Repr>> repr;
    for (auto& elem : representations_) {
        if (elem.second.get() == representation) {
            repr.insert(elem);
            if (lastValidRepresentation_.get() != representation) {
                if (elem.second->isValid()) {
                    lastValidRepresentation_ = elem.second;
                } else {
                    lastValidRepresentation_.reset();
                }
            }
            break;
        }
    }
    std::swap(repr, representations_);
}

template <class Repr>
bool Data<Repr>::hasRepresentations() const {
    std::unique_lock<std::mutex> lock(mutex_);
    return !representations_.empty();
}

template <class Repr>
void Data<Repr>::setDataFormat(const DataFormatBase* format) {
    dataFormatBase_ = format;
}

template <class Repr>
const DataFormatBase* Data<Repr>::getDataFormat() const {
    return dataFormatBase_;
}

}  // namespace

#endif  // IVW_DATA_H
