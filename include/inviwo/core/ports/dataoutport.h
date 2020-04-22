/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2020 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/datatraits.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/ports/outportiterable.h>
#include <inviwo/core/ports/porttraits.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/document.h>

#include <glm/fwd.hpp>

#include <memory>

namespace inviwo {

/**
 * \ingroup ports
 * DataOutport hold data of type T
 */
template <typename T>
class DataOutport : public Outport, public OutportIterableImpl<DataOutport<T>, T> {
public:
    using type = T;
    DataOutport(std::string identifier);
    virtual ~DataOutport() = default;

    virtual std::string getClassIdentifier() const override;
    virtual glm::uvec3 getColorCode() const override;
    virtual Document getInfo() const override;

    virtual std::shared_ptr<const T> getData() const;

    /**
     * Return data and release ownership. Data in the port will be nullptr after call.
     */
    virtual std::shared_ptr<const T> detachData();

    /**
     * \copydoc Outport::clear
     */
    virtual void clear() override;

    virtual void setData(std::shared_ptr<const T> data);
    virtual void setData(const T* data);  // will assume ownership of data.

    /**
     * Pass data along to the port using the Move constructor.
     * Example:
     * ```c++
     * void SomePorcessor::process() {
     *     std::vector<vec3> points;
     *      /// code to fill the points-vector with data
     *      myPort_.setData(std::move(points));
     * }
     * ```
     */
    template <typename U = T, typename = std::enable_if_t<std::is_move_constructible_v<U>>>
    void setData(T&& data);

    virtual bool hasData() const override;

protected:
    std::shared_ptr<const T> data_;
};

template <typename T>
struct PortTraits<DataOutport<T>> {
    static std::string classIdentifier() {
        return util::appendIfNotEmpty(DataTraits<T>::classIdentifier(), ".outport");
    }
};

template <typename T>
DataOutport<T>::DataOutport(std::string identifier)
    : Outport(identifier), OutportIterableImpl<DataOutport<T>, T>{}, data_() {

    isReady_.setUpdate([this]() {
        return invalidationLevel_ == InvalidationLevel::Valid && data_.get() != nullptr;
    });
}

template <typename T>
std::string DataOutport<T>::getClassIdentifier() const {
    return PortTraits<DataOutport<T>>::classIdentifier();
}

template <typename T>
glm::uvec3 DataOutport<T>::getColorCode() const {
    return DataTraits<T>::colorCode();
}

template <typename T>
std::shared_ptr<const T> DataOutport<T>::getData() const {
    return data_;
}

template <typename T>
void DataOutport<T>::setData(std::shared_ptr<const T> data) {
    data_ = data;
    isReady_.update();
}

template <typename T>
void DataOutport<T>::setData(const T* data) {
    data_.reset(data);
    isReady_.update();
}

template <typename T>
std::shared_ptr<const T> DataOutport<T>::detachData() {
    std::shared_ptr<const T> data(data_);
    data_.reset();
    isReady_.update();
    return data;
}

template <typename T>
template <typename, typename>
void DataOutport<T>::setData(T&& data) {
    setData(std::make_shared<T>(data));
}

template <typename T>
bool DataOutport<T>::hasData() const {
    return data_.get() != nullptr;
}

template <typename T>
void DataOutport<T>::clear() {
    data_.reset();
    isReady_.update();
}

template <typename T>
Document DataOutport<T>::getInfo() const {
    Document doc;
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;
    auto b = doc.append("html").append("body");
    auto p = b.append("p");
    p.append("b", htmlEncode(DataTraits<T>::dataName()) + " Outport", {{"style", "color:white;"}});
    utildoc::TableBuilder tb(p, P::end());
    tb(H("Identifier"), getIdentifier());
    tb(H("Class"), getClassIdentifier());
    tb(H("Ready"), isReady());
    tb(H("Connected"), isConnected());
    tb(H("Connections"), connectedInports_.size());

    if (hasData()) {
        b.append("p").append(DataTraits<T>::info(*getData()));
    } else {
        b.append("p", "Port has no data");
    }
    return doc;
}

}  // namespace inviwo
