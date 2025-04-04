/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#pragma once

#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/exception.h>
#include <typeindex>

namespace inviwo {

struct ResourceMeta;

class IVW_CORE_API MissingRepresentation : public Exception {
public:
    MissingRepresentation(const std::string& message = "", SourceContext context = SourceContext());
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

    virtual std::type_index getTypeIndex() const = 0;

    void setOwner(const Owner* owner);
    const Owner* getOwner() const;

    bool isValid() const;
    void setValid(bool valid);

    virtual void updateResource(const ResourceMeta&) const {};

protected:
    DataRepresentation() = default;
    DataRepresentation(const DataRepresentation& rhs) = default;
    DataRepresentation& operator=(const DataRepresentation& that) = default;

    bool isValid_ = true;
    const Owner* owner_ = nullptr;
};

template <typename Owner>
void DataRepresentation<Owner>::setOwner(const Owner* owner) {
    owner_ = owner;
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
