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

#ifndef IVW_DATA_H
#define IVW_DATA_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/datarepresentation.h>
#include <inviwo/core/datastructures/representationconverterfactory.h>
#include <inviwo/core/metadata/metadatamap.h>
#include <inviwo/core/metadata/metadataowner.h>

namespace inviwo {

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
 * \class Data
 *
 * \brief The base class for all data objects.
 *
 *  It is responsible for holding the DateRepresentations
 *  and the format of the data.
 *
 *  Requirements:
 *      1. Copy constructor for deep copy
 *      2. Assignment operator for deep copy
 *      3. Clone pattern
 *
 *  1 and 2 are needed to be a vaild member type of std::vector.
 *  3 is needed for the factory pattern, 3 should be implemented using 1.
 *
 */
class IVW_CORE_API Data : public BaseData {
public:
    Data();
    Data(const DataFormatBase*);
    Data(const Data& rhs);
    Data& operator=(const Data& rhs);
    virtual Data* clone() const = 0;
    virtual ~Data();

    // Representations
    template <typename T>
    const T* getRepresentation() const;

    template <typename T>
    T* getEditableRepresentation();

    template <typename T>
    bool hasRepresentation() const;

    bool hasRepresentations() const;
    void addRepresentation(std::shared_ptr<DataRepresentation> representation);

    /**
     * Remove representation from data object.
     * This will delete the representation, thus rendering the representation pointer invalid.
     * @param representation The representation to remove
     */
    void removeRepresentation(std::shared_ptr<DataRepresentation> representation);
    void removeOtherRepresentations(std::shared_ptr<DataRepresentation> representation);
    void clearRepresentations();

    // DataFormat
    void setDataFormat(const DataFormatBase* format);
    const DataFormatBase* getDataFormat() const;

protected:
    virtual std::shared_ptr<DataRepresentation> createDefaultRepresentation() const = 0;
    template <typename T>
    const T* getValidRepresentation() const;
    void invalidateAllOther(DataRepresentation* repr);
    void copyRepresentationsTo(Data* targetData) const;


    mutable std::mutex mutex_;
    mutable std::unordered_map<std::type_index, std::shared_ptr<DataRepresentation>> representations_;
    // A pointer to the the most recently updated representation. Makes updates and creation faster.
    mutable std::shared_ptr<DataRepresentation> lastValidRepresentation_; 
    const DataFormatBase* dataFormatBase_;
};

template <typename T>
const T* Data::getRepresentation() const {
    if (!hasRepresentations()) {
        auto repr = const_cast<Data*>(this)->createDefaultRepresentation();
        if (!repr) {
            throw Exception(
                "CreateDefaultRepresentation returned nullptr. Possible missing subclass "
                "implementation",
                IvwContext);
        }
        repr->setValid(true);
        repr->setOwner(const_cast<Data*>(this));

        {
            std::unique_lock<std::mutex> lock(mutex_);
            representations_[repr->getTypeIndex()] = repr;
            lastValidRepresentation_ = repr;
        }
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

template <typename T>
const T* Data::getValidRepresentation() const {
    auto factory = RepresentationConverterFactory::getPtr();

    auto package = factory->getRepresentationConverter(lastValidRepresentation_->getTypeIndex(),
                                                       std::type_index(typeid(T)));

    if (package) {
        for (auto converter : package->getConverters()) {
            std::shared_ptr<DataRepresentation> result;
            
            auto dest = converter->getConverterID().second;
            auto it = representations_.find(dest);
            if (it != representations_.end()) { // Next representation already exist, just update it
                result = it->second;
                converter->update(lastValidRepresentation_.get(), result.get());
            } else {  // No representation found, create it
                result = std::shared_ptr<DataRepresentation>(
                    converter->createFrom(lastValidRepresentation_.get()));
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

template <typename T>
T* Data::getEditableRepresentation() {
    T* result = const_cast<T*>(getRepresentation<T>());
    invalidateAllOther(result);
    return result;
}

template <typename T>
bool Data::hasRepresentation() const {
    return util::has_key(representations_, std::type_index(typeid(T)));
}



}  // namespace

#endif  // IVW_DATA_H
