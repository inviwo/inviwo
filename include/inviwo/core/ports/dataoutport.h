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

    virtual std::shared_ptr<const T> getData() const;
    // Return data and release ownership. Data in the port will be nullptr after call.
    virtual std::shared_ptr<const T> detachData();

    virtual void setData(std::shared_ptr<const T> data);
    virtual void setData(const T* data); // will assume ownership of data.

    /**
     * An outport is ready if it has data and is valid.
     */
    virtual bool isReady() const override;
    bool hasData() const;

    virtual std::string getContentInfo() const override;

protected:
    std::shared_ptr<const T> data_;
};

template <typename T>
DataOutport<T>::DataOutport(std::string identifier)
    : Outport(identifier)
    , OutportIterableImpl<T>(this)
    , data_() {}

template <typename T>
DataOutport<T>::~DataOutport() {}

template <typename T>
std::string inviwo::DataOutport<T>::getClassIdentifier() const {
    return port_traits<T>::class_identifier() + "Outport";
}

template <typename T>
uvec3 inviwo::DataOutport<T>::getColorCode() const {
    return port_traits<T>::color_code();
}

template <typename T>
std::shared_ptr<const T> DataOutport<T>::getData() const {
    return data_;
}

template <typename T>
void DataOutport<T>::setData(std::shared_ptr<const T> data) {
    data_ = data;
}

template <typename T>
void DataOutport<T>::setData(const T* data) {
    data_.reset(data);
}

template <typename T>
std::shared_ptr<const T> DataOutport<T>::detachData() {
    std::shared_ptr<const T> data(data_);
    data_.reset();
    return data;
}

template <typename T>
bool DataOutport<T>::hasData() const {
    return data_.get() != nullptr;
}

template <typename T>
bool DataOutport<T>::isReady() const {
    return hasData() && invalidationLevel_ == VALID;
}

template <typename T>
std::string DataOutport<T>::getContentInfo() const {
    if (hasData()) {
        std::string info = port_traits<T>::data_info(data_.get());
        if (!info.empty()) {
            return info;
        } else {
            return "No information available for: " + util::class_identifier<T>();
        }
    } else {
        return port_traits<T>::class_identifier() + "Outport has no data";
    }
}

}  // namespace

#endif  // IVW_DATAOUTPORT_H
