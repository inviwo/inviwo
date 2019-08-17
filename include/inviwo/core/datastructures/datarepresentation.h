/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#ifndef IVW_DATAREPRESENTATION_H
#define IVW_DATAREPRESENTATION_H

#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/exception.h>
#include <typeindex>

namespace inviwo {

class IVW_CORE_API MissingRepresentation : public Exception {
public:
    MissingRepresentation(const std::string& message = "",
                          ExceptionContext context = ExceptionContext());
    virtual ~MissingRepresentation() noexcept = default;
};

/**
 * \ingroup datastructures
 * \brief Base class for all DataRepresentations \see Data
 */
template <typename Owner>
class DataRepresentation {
public:
    using ReprOwner = Owner;

    virtual DataRepresentation* clone() const = 0;
    virtual ~DataRepresentation() = default;

    const DataFormatBase* getDataFormat() const;
    std::string getDataFormatString() const;
    DataFormatId getDataFormatId() const;

    virtual std::type_index getTypeIndex() const = 0;

    void setOwner(Owner* owner);
    Owner* getOwner();
    const Owner* getOwner() const;

    bool isValid() const;
    void setValid(bool valid);

protected:
    DataRepresentation() = default;
    DataRepresentation(const DataFormatBase* format);
    DataRepresentation(const DataRepresentation& rhs) = default;
    DataRepresentation& operator=(const DataRepresentation& that) = default;
    void setDataFormat(const DataFormatBase* format);

    bool isValid_ = true;
    const DataFormatBase* dataFormatBase_ = DataUInt8::get();
    Owner* owner_ = nullptr;
};

template <typename Owner>
DataRepresentation<Owner>::DataRepresentation(const DataFormatBase* format)
    : isValid_(true), dataFormatBase_(format), owner_(nullptr) {}

template <typename Owner>
const DataFormatBase* DataRepresentation<Owner>::getDataFormat() const {
    return dataFormatBase_;
}

template <typename Owner>
std::string DataRepresentation<Owner>::getDataFormatString() const {
    return std::string(dataFormatBase_->getString());
}

template <typename Owner>
DataFormatId DataRepresentation<Owner>::getDataFormatId() const {
    return dataFormatBase_->getId();
}

template <typename Owner>
void DataRepresentation<Owner>::setDataFormat(const DataFormatBase* format) {
    dataFormatBase_ = format;
}

template <typename Owner>
void DataRepresentation<Owner>::setOwner(Owner* owner) {
    owner_ = owner;
}

template <typename Owner>
Owner* DataRepresentation<Owner>::getOwner() {
    return owner_;
}

template <typename Owner>
const Owner* DataRepresentation<Owner>::getOwner() const {
    return owner_;
}

template <typename Owner>
bool DataRepresentation<Owner>::isValid() const {
    return isValid_;
}

template <typename Owner>
void DataRepresentation<Owner>::setValid(bool valid) {
    isValid_ = valid;
}

}  // namespace inviwo

#endif  // IVW_DATAREPRESENTATION_H
