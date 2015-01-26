/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#ifndef IVW_DATAOUTPORT_H
#define IVW_DATAOUTPORT_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/data.h>
#include <inviwo/core/datastructures/datasequence.h>
#include <inviwo/core/ports/outport.h>

namespace inviwo {

template< typename T, typename U> class MultiDataInport;

template< typename T> class DataInport;

template<typename T>
class DataOutport : public Outport {

    friend class MultiDataInport<T, DataInport<T> >;
public:
    DataOutport(std::string identifier, InvalidationLevel invalidationLevel=INVALID_OUTPUT);
    virtual ~DataOutport();

    void initialize();
    void deinitialize();

    virtual T* getData();
    virtual DataSequence<T>* getDataSequence();

    virtual const T* getConstData() const;
    virtual const DataSequence<T>* getConstDataSequence() const;

    void setData(T* data, bool ownsData = true);
    void setConstData(const T* data);

    T* detachDataFromOutport();

    bool hasData() const;
    bool hasDataSequence() const;

    bool isDataOwner() const;
    virtual std::string getContentInfo() const;

    virtual void dataChanged() {}

protected:
    T* data_;
    bool ownsData_;
    bool isSequence_;
};

template <typename T>
DataOutport<T>::DataOutport(std::string identifier, InvalidationLevel invalidationLevel)
    : Outport(identifier, invalidationLevel),
    data_(NULL), ownsData_(false), isSequence_(false)
{
}

template <typename T>
DataOutport<T>::~DataOutport() {
    if (ownsData_ && data_)
        delete data_;
}

template <typename T>
void DataOutport<T>::initialize() {}

template <typename T>
void DataOutport<T>::deinitialize() {}

template <typename T>
T* DataOutport<T>::getData() {
    if (data_) {
        ivwAssert(ownsData_, "Port does not own data, so can not return writable data.");
    }

    if(isSequence_)
        return static_cast<DataSequence<T>*>(data_)->getCurrent();

    return data_;
}

template <typename T>
DataSequence<T>* DataOutport<T>::getDataSequence() {
    if (data_) {
        ivwAssert(ownsData_, "Port does not own data, so can not return writable data.");
    }

    if(isSequence_)
        return static_cast<DataSequence<T>*>(data_);

    return NULL;
}

template <typename T>
const T* DataOutport<T>::getConstData() const {
    if(isSequence_)
        return const_cast<const T*>(static_cast<DataSequence<T>*>(data_)->getCurrent());

    return const_cast<const T*>(data_);
}

template <typename T>
const DataSequence<T>* DataOutport<T>::getConstDataSequence() const {
    if(isSequence_)
        return const_cast<const DataSequence<T>*>(static_cast<DataSequence<T>*>(data_));

    return NULL;
}

template <typename T>
void DataOutport<T>::setData(T* data, bool ownsData) {
    if (ownsData_ && data_) {
        if (data != data_) {
            //Delete old data
            delete data_;
        }
    }

    isSequence_ = (dynamic_cast<DataSequence<T>*>(data) != NULL);

    ownsData_ = ownsData;
    //Add reference to new data
    data_ = data;

    dataChanged();
}

template <typename T>
void DataOutport<T>::setConstData(const T* data) {
    if (ownsData_ && data_) {
        if (data != data_) {
            //Delete old data
            delete data_;
        }
    }

    ownsData_ = false;

    isSequence_ = (dynamic_cast<const DataSequence<T>*>(data) != NULL);

    //Add reference to new data
    data_ = const_cast<T*>(data);

    dataChanged();
}

template <typename T>
T* DataOutport<T>::detachDataFromOutport() {
    if (ownsData_) {
        ownsData_ = false;
        return data_;
    }

    return NULL;
}

template <typename T>
bool DataOutport<T>::hasData() const {
    return (data_ != NULL);
}

template <typename T>
bool DataOutport<T>::hasDataSequence() const {
    return (hasData() && isSequence_);
}

template <typename T>
bool DataOutport<T>::isDataOwner() const {
    return ownsData_;
}

template <typename T>
std::string DataOutport<T>::getContentInfo() const {
    if (hasDataSequence()) {
        const DataSequence<T>* seq = static_cast<const DataSequence<T>*>(data_);
        return seq->getDataInfo();
    } else if (hasData()) {
        const BaseData* data = dynamic_cast<const BaseData*>(data_);
        if (data) {
            return data->getDataInfo();
        } else {
            return "Not a BaseData Object";
        }
    } else {
        return getClassIdentifier() + " has no data.";
    }
}

} // namespace

#endif // IVW_DATAOUTPORT_H
