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

#ifndef IVW_DATA_H
#define IVW_DATA_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/datarepresentation.h>
#include <inviwo/core/datastructures/representationconverterfactory.h>
#include <inviwo/core/metadata/metadatamap.h>
#include <inviwo/core/metadata/metadataowner.h>

namespace inviwo {

class IVW_CORE_API BaseData :  public MetaDataOwner {
public:
    BaseData();
    BaseData(const BaseData& rhs);
    BaseData& operator=(const BaseData& rhs);
    virtual BaseData* clone() const = 0;
    virtual ~BaseData();
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
    void addRepresentation(DataRepresentation* representation);

    /**
     * Remove representation from data object.
     * This will delete the representation, thus rendering the representation pointer invalid.
     *
     * @param representation The representation to remove
     */
    void removeRepresentation(DataRepresentation* representation);
    void clearRepresentations();

    //DataFormat
    void setDataFormat(const DataFormatBase* format);
    const DataFormatBase* getDataFormat() const;

protected:
    virtual DataRepresentation* createDefaultRepresentation() = 0;

    virtual void newRepresentationCreated() const { }

    template<typename T>
    void updateRepresentation(T* representation, int index) const;

    template<typename T>
    const T* createNewRepresentationUsingConverters() const;

    template<typename T>
    RepresentationConverterPackage<T>* findRepresentationConverterPackage(DataRepresentation*, RepresentationConverterFactory*) const;

    template<class T>
    void invalidateAllOther();

    void copyRepresentationsTo(Data* targetData) const;


    /**
     * Check if data needs to be updated.
     * See http://www.cprogramming.com/tutorial/bitwise_operators.html
     * @param index Index into representations_ vector.
     * @return True if up-to-date
     */
    inline bool isRepresentationValid(int index) const { return (validRepresentations_ & (1 << index)) != 0; }
    inline void setRepresentationAsValid(int index) const { validRepresentations_ = validRepresentations_ | (1 << index); }
    inline void setRepresentationAsInvalid(int index) const { validRepresentations_ = validRepresentations_ & ~(1 << index); }
    inline void setAllOtherRepresentationsAsInvalid(int index) const { validRepresentations_ = (1 << index); }
    inline void setAllRepresentationsAsInvalid() const { validRepresentations_ = 0; }
    inline void setAllRepresentationsAsValid() const { validRepresentations_ = ~0; }

    mutable std::vector<DataRepresentation*> representations_;
    mutable int validRepresentations_; ///< Bit representation of valid representation. A maximum of 32 representations are supported.
    mutable DataRepresentation*
    lastValidRepresentation_; ///< A pointer to the the most recently updated representation. Makes updates and creation faster.

    const DataFormatBase* dataFormatBase_;
};

template<typename T>
const T* Data::getRepresentation() const {
    if (!hasRepresentations()) {
        DataRepresentation* repr = const_cast<Data*>(this)->createDefaultRepresentation();
        ivwAssert(repr != NULL, " CreateDefaultRepresentation retured NULL. Possible missing subclass implementation")
        repr->setOwner(const_cast<Data *>(this));
        representations_.push_back(repr);
        lastValidRepresentation_ = representations_[0];
        setRepresentationAsValid(static_cast<int>(representations_.size())-1);
    }

    // check if a representation exists and return it
    for (int i=0; i<static_cast<int>(representations_.size()); ++i) {
        T* representation = dynamic_cast<T*>(representations_[i]);

        if (representation) {
            if (isRepresentationValid(i)) {
                lastValidRepresentation_ = representation;
                return representation;
            } else {
                updateRepresentation<T>(representation, i);
                lastValidRepresentation_ = representation;
                return representation;
            }
        }
    }

    //no representation exists, so we try to create one
    const T* result = 0;
    result = createNewRepresentationUsingConverters<T>();
    //ivwAssert(result!=0, "Required representation converter does not exist.");
    return result;
}

template<typename T>
const T* Data::createNewRepresentationUsingConverters() const {
    // no representation exists, so we try to create one
    DataRepresentation* result = 0;
    RepresentationConverterFactory* representationConverterFactory = RepresentationConverterFactory::getPtr();
    RepresentationConverter* converter = representationConverterFactory->getRepresentationConverter<T>(lastValidRepresentation_);

    if (converter) {
        result = converter->createFrom(lastValidRepresentation_);
        if(!result)
            return NULL;
        result->setOwner(const_cast<Data *>(this));
        representations_.push_back(result);
        setRepresentationAsValid(static_cast<int>(representations_.size())-1);
        lastValidRepresentation_ = result;
        newRepresentationCreated();
        return dynamic_cast<T*>(result);
    }

    //A one-2-one converter could not be found, thus we want to find the smallest package of converters to get to our destination
    RepresentationConverterPackage<T>* converterPackage = findRepresentationConverterPackage<T>(lastValidRepresentation_, representationConverterFactory);

    if (converterPackage) {
        const std::vector<RepresentationConverter*>* converters = converterPackage->getConverters();

        for (std::vector<RepresentationConverter*>::const_iterator converterIt = converters->begin(),
             converterEnd = converters->end(); converterIt != converterEnd; ++converterIt) {
            // Check if we already have the destination representation
            // and can update to it instead of creating a new
            bool updatedRepresentation = false;

            for (std::vector<DataRepresentation*>::const_iterator it = representations_.begin(), itEnd = representations_.end();
                 it != itEnd; ++it) {
                if (lastValidRepresentation_ != *it && (*converterIt)->canConvertTo(*it)) {
                    (*converterIt)->update(lastValidRepresentation_, *it);
                    setRepresentationAsValid(static_cast<int>(representations_.size())-static_cast<int>(itEnd-it));
                    lastValidRepresentation_ = *it;
                    updatedRepresentation = true;
                    break;
                }
            }

            // Create the representation if it did not exist
            if (!updatedRepresentation) {
                lastValidRepresentation_ = (*converterIt)->createFrom(lastValidRepresentation_);
                if(!lastValidRepresentation_)
                    return NULL;
                lastValidRepresentation_->setOwner(const_cast<Data *>(this));
                representations_.push_back(lastValidRepresentation_);
                setRepresentationAsValid(static_cast<int>(representations_.size())-1);
            }
        }

        newRepresentationCreated();
        return dynamic_cast<T*>(lastValidRepresentation_);
    }

    return NULL;
}

template<typename T>
void Data::updateRepresentation(T* representation, int index) const {
    RepresentationConverterFactory* representationConverterFactory = RepresentationConverterFactory::getPtr();

    if (lastValidRepresentation_) {
        RepresentationConverter* converter = representationConverterFactory->getRepresentationConverter<T>(lastValidRepresentation_);

        if (converter) {
            converter->update(lastValidRepresentation_, representation);
            setRepresentationAsValid(index);
            lastValidRepresentation_ = representation;
            return;
        }

        //A one-2-one converter could not be found, thus we want to find the smallest package of converters to get to our destination
        RepresentationConverterPackage<T>* converterPackage = findRepresentationConverterPackage<T>(lastValidRepresentation_, representationConverterFactory);

        //Go-through the conversion package
        if (converterPackage) {
            const std::vector<RepresentationConverter*>* converters = converterPackage->getConverters();

            for (int j=0; j<static_cast<int>(converters->size()); ++j) {
                for (int k=0; k<static_cast<int>(representations_.size()); ++k) {
                    if (converters->at(j)->canConvertTo(representations_[k])) {
                        converters->at(j)->update(lastValidRepresentation_, representations_[k]);
                        setRepresentationAsValid(k);
                        lastValidRepresentation_ = representations_[k];
                        break;
                    }
                }
            }
        }
    }
}

template<typename T>
RepresentationConverterPackage<T>* Data::findRepresentationConverterPackage(DataRepresentation* validRepresentation, RepresentationConverterFactory* representationConverterFactory) const {
    RepresentationConverterPackage<T>* converterPackage = representationConverterFactory->getRepresentationConverterPackage<T>
        (validRepresentation);

    DataRepresentation* result = NULL;
    if (converterPackage)
        return converterPackage;
    else {
        // Not possible to convert from last valid representation.
        // Check if it is possible to convert from another valid representation.
        for (int i=0; i<static_cast<int>(representations_.size()); ++i) {
            if (isRepresentationValid(i)) {
                RepresentationConverterPackage<T>* currentConverterPackage = representationConverterFactory->getRepresentationConverterPackage<T>
                    (representations_[i]);

                if (currentConverterPackage) {
                    if (converterPackage) {
                        if (currentConverterPackage->getNumberOfConverters() < converterPackage->getNumberOfConverters()) {
                            converterPackage = currentConverterPackage;
                            result = representations_[i];
                        }
                    } else {
                        converterPackage = currentConverterPackage;
                        result = representations_[i];
                    }
                }
            }
        }
    }

    //If not converter package was found, try to create one by combining one-2-one converters
    if (!converterPackage) {
        std::vector<RepresentationConverter*> srcConverters = representationConverterFactory->getRepresentationConvertersFrom(validRepresentation);
        std::vector<RepresentationConverter*> toConverters = representationConverterFactory->getRepresentationConvertersTo<T>();

        std::vector<RepresentationConverter*>::iterator srcConverterEnd = srcConverters.end();
        std::vector<RepresentationConverter*>::iterator toConverterEnd = toConverters.end();

        for (std::vector<RepresentationConverter*>::iterator srcConverterIt = srcConverters.begin(); srcConverterIt != srcConverterEnd; ++srcConverterIt) {
            for (std::vector<RepresentationConverter*>::iterator toConverterIt = toConverters.begin(); toConverterIt != toConverterEnd; ++toConverterIt) {

                //Match srcTo with toSrc if src->srcTo and toSrc->to
                if(!converterPackage && (*toConverterIt)->isConverterReverse(*srcConverterIt)){
                    converterPackage = new RepresentationConverterPackage<T>;
                    converterPackage->addConverter(*srcConverterIt);
                    converterPackage->addConverter(*toConverterIt);
                    //Register the new convert so we don't have to do this again
                    representationConverterFactory->registerObject(converterPackage);

                    break;
                }
            }
        }
    }
    else{
        validRepresentation = result;
    }

    //TODO, If not converter package was found, try to create one by combining one-2-one converters with converter packages

    return converterPackage;
}

template<typename T>
T* Data::getEditableRepresentation() {
    T* result = const_cast<T*>(getRepresentation<T>());

    if (representations_.size()>1)
        invalidateAllOther<T>();

    return result;
}

template<typename T>
bool Data::hasRepresentation() const {
    for (size_t i=0; i<representations_.size(); i++) {
        T* representation = dynamic_cast<T*>(representations_[i]);

        if (representation) return true;
    }

    return false;
}

template<typename T>
void Data::invalidateAllOther() {
    for (int i = 0; i < static_cast<int>(representations_.size()); ++i) {
        T* representation = dynamic_cast<T*>(representations_[i]);

        if (representation) {
            setAllOtherRepresentationsAsInvalid(i);
            lastValidRepresentation_ = representations_[i];
            break;
        }
    }
}

} // namespace

#endif // IVW_DATA_H
