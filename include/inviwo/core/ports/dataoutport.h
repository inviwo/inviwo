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

#ifndef IVW_DATAOUTPORT_H
#define IVW_DATAOUTPORT_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/data.h>
#include <inviwo/core/datastructures/datasequence.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/ports/outportiterable.h>
#include <inviwo/core/util/introspection.h>

namespace inviwo {

template <typename T>
class DataOutport : public Outport, public OutportIterableImpl<T> {
public:
    DataOutport(std::string identifier);
    virtual ~DataOutport();

    virtual uvec3 getColorCode() const override;
    virtual std::string getClassIdentifier() const override;

    virtual T* getData();
    virtual DataSequence<T>* getDataSequence();

    virtual const T* getConstData() const;
    virtual const DataSequence<T>* getConstDataSequence() const;

    virtual void setData(T* data, bool ownsData = true);
    virtual void setConstData(const T* data);

    /**
     * Return data and release ownership. Data in the port will be nullptr after call.
     */
    virtual T* detachData();

    /**
     * An outport is ready if it has data and is valid.
     */
    virtual bool isReady() const override;
    bool hasData() const;
    bool hasDataSequence() const;

    bool isDataOwner() const;
    virtual std::string getContentInfo() const;

protected:
    T* data_;
    bool ownsData_;
    bool isSequence_;
};


namespace detail {

template <typename T, typename std::enable_if<!std::is_polymorphic<T>::value, int>::type = 0>
bool isDataSequence(T* data) {
    return false;
};
template <typename T, typename std::enable_if<std::is_polymorphic<T>::value, int>::type = 0>
bool isDataSequence(T* data) {
    return dynamic_cast<DataSequence<T>*>(data) != nullptr;
};

template <typename T, typename std::enable_if<!std::is_polymorphic<T>::value, int>::type = 0>
DataSequence<T>* getDataSequence(T* data) {
    return nullptr;
};
template <typename T, typename std::enable_if<std::is_polymorphic<T>::value, int>::type = 0>
DataSequence<T>* getDataSequence(T* data) {
    return dynamic_cast<DataSequence<T>*>(data);
};

}


template <typename T>
DataOutport<T>::DataOutport(std::string identifier)
    : Outport(identifier)
    , OutportIterableImpl<T>(this)
    , data_(nullptr)
    , ownsData_(true)
    , isSequence_(false) {}

template <typename T>
DataOutport<T>::~DataOutport() {
    if (ownsData_ && data_) delete data_;
}

template <typename T>
std::string inviwo::DataOutport<T>::getClassIdentifier() const {
    return port_traits<T>::class_identifier() + "Outport";
}

template <typename T>
uvec3 inviwo::DataOutport<T>::getColorCode() const {
    return port_traits<T>::color_code();
}

template <typename T>
T* DataOutport<T>::getData() {
    ivwAssert(ownsData_, "Port does not own data, so can not return writable data.");

    if (isSequence_)
        return detail::getDataSequence<T>(data_)->getCurrent();
    else
        return data_;
}

template <typename T>
DataSequence<T>* DataOutport<T>::getDataSequence() {
    ivwAssert(ownsData_, "Port does not own data, so can not return writable data.");

    if (isSequence_)
        return detail::getDataSequence<T>(data_);
    else
        return nullptr;
}

template <typename T>
const T* DataOutport<T>::getConstData() const {
    if (isSequence_)
        return const_cast<const T*>(detail::getDataSequence<T>(data_)->getCurrent());
    else
        return const_cast<const T*>(data_);
}

template <typename T>
const DataSequence<T>* DataOutport<T>::getConstDataSequence() const {
    if (isSequence_)
        return const_cast<const DataSequence<T>*>(detail::getDataSequence<T>(data_));
    else
        return nullptr;
}

template <typename T>
void DataOutport<T>::setData(T* data, bool ownsData) {
    if (ownsData_ && data_ && data_ != data) {
        delete data_;  // Delete old data
    }
    
    isSequence_ = detail::isDataSequence<T>(data);
    ownsData_ = ownsData;
    data_ = data;  // Add reference to new data
}

template <typename T>
void DataOutport<T>::setConstData(const T* data) {
    setData(const_cast<T*>(data), false);
}

template <typename T>
T* DataOutport<T>::detachData() {
    if (ownsData_) {
        ownsData_ = false;
        T* data = nullptr;
        std::swap(data, data_);
        return data;
    }
    return nullptr;
}

template <typename T>
bool DataOutport<T>::hasData() const {
    return (data_ != nullptr);
}

template <typename T>
bool DataOutport<T>::hasDataSequence() const {
    return (hasData() && isSequence_);
}

template <typename T>
bool DataOutport<T>::isReady() const {
    return hasData() && invalidationLevel_ == VALID;
}

template <typename T>
bool DataOutport<T>::isDataOwner() const {
    return ownsData_;
}

template <typename T>
std::string DataOutport<T>::getContentInfo() const {
    if (hasDataSequence()) {
        auto seq = static_cast<const DataSequence<T>*>(detail::getDataSequence<T>(data_));
        return seq->getDataInfo();
    } else if (hasData()) {
        std::string info = port_traits<T>::data_info(data_);
        if (!info.empty()) {
            return info;
        } else {
            return "No information available for: " + util::class_identifier<T>();
        }
    } else {
        return "Port has no data";
    }
}

}  // namespace

#endif  // IVW_DATAOUTPORT_H
