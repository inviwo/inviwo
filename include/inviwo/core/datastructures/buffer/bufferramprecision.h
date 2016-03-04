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

#ifndef IVW_BUFFERRAMPRECISION_H
#define IVW_BUFFERRAMPRECISION_H

#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/util/stdextensions.h>
#include <initializer_list>

namespace inviwo {

template <typename T>
class BufferRAMPrecision : public BufferRAM {
public:
    BufferRAMPrecision(BufferUsage usage = BufferUsage::Static);
    BufferRAMPrecision(size_t size, BufferUsage usage = BufferUsage::Static);
    BufferRAMPrecision(std::vector<T> data, BufferUsage usage = BufferUsage::Static);
    BufferRAMPrecision(const BufferRAMPrecision<T>& rhs);
    BufferRAMPrecision<T>& operator=(const BufferRAMPrecision<T>& that);
    virtual ~BufferRAMPrecision() = default;
    virtual BufferRAMPrecision<T>* clone() const override;

    virtual void setSize(size_t size) override;
    virtual size_t getSize() const override;

    virtual void* getData() override;
    virtual const void* getData() const override;
    std::vector<T>* getDataContainer();
    const std::vector<T>* getDataContainer() const;

    virtual double getAsDouble(const size_t& pos) const override;
    virtual dvec2 getAsDVec2(const size_t& pos) const override;
    virtual dvec3 getAsDVec3(const size_t& pos) const override;
    virtual dvec4 getAsDVec4(const size_t& pos) const override;

    virtual void setFromDouble(const size_t& pos, double val) override;
    virtual void setFromDVec2(const size_t& pos, dvec2 val) override;
    virtual void setFromDVec3(const size_t& pos, dvec3 val) override;
    virtual void setFromDVec4(const size_t& pos, dvec4 val) override;

    virtual double getAsNormalizedDouble(const size_t& pos) const override;
    virtual dvec2 getAsNormalizedDVec2(const size_t& pos) const override;
    virtual dvec3 getAsNormalizedDVec3(const size_t& pos) const override;
    virtual dvec4 getAsNormalizedDVec4(const size_t& pos) const override;
 
    virtual void setFromNormalizedDouble(const size_t& pos, double val) override;
    virtual void setFromNormalizedDVec2(const size_t& pos, dvec2 val) override;
    virtual void setFromNormalizedDVec3(const size_t& pos, dvec3 val) override;
    virtual void setFromNormalizedDVec4(const size_t& pos, dvec4 val) override;

    void add(const T& item);
    void add(std::initializer_list<T> data);
    void append(const std::vector<T>* data);

    T& operator[](size_t i);
    const T& operator[](size_t i) const;

    void set(size_t index, const T& item);
    T get(size_t index) const;
    T& get(size_t index);

    void clear();

private:
    std::unique_ptr<std::vector<T>> data_;
};

using FloatBufferRAM = BufferRAMPrecision<float>;
using Vec2BufferRAM = BufferRAMPrecision<vec2>;
using Vec3BufferRAM = BufferRAMPrecision<vec3>;
using Vec4BufferRAM = BufferRAMPrecision<vec4>;
// Used for index buffers
using IndexBufferRAM = BufferRAMPrecision<std::uint32_t>;

template <typename T>
const T& inviwo::BufferRAMPrecision<T>::operator[](size_t i) const {
    return (*data_)[i];
}

template <typename T>
T& inviwo::BufferRAMPrecision<T>::operator[](size_t i) {
    return (*data_)[i];
}

template <typename T>
BufferRAMPrecision<T>::BufferRAMPrecision(BufferUsage usage)
    : BufferRAMPrecision(0, usage) {}

template <typename T>
BufferRAMPrecision<T>::BufferRAMPrecision(size_t size, BufferUsage usage)
    : BufferRAM(DataFormat<T>::get(), usage), data_(new std::vector<T>(size)) {}

template <typename T>
inviwo::BufferRAMPrecision<T>::BufferRAMPrecision(std::vector<T> data, BufferUsage usage)
    : BufferRAM(DataFormat<T>::get(), usage)
    , data_(util::make_unique<std::vector<T>>(std::move(data))) {}

template <typename T>
BufferRAMPrecision<T>::BufferRAMPrecision(const BufferRAMPrecision<T>& rhs)
    : BufferRAM(rhs), data_(new std::vector<T>(rhs.data_->size())) {
    *data_ = *(rhs.data_);
}

template <typename T>
BufferRAMPrecision<T>& inviwo::BufferRAMPrecision<T>::operator=(const BufferRAMPrecision<T>& that) {
    if (this != &that) {
        BufferRAM::operator=(that);
        auto data = util::make_unique<std::vector<T>>(that.data_->size());
        *data = *(that.data_);
        std::swap(data, data_);
    }
    return *this;
}

template <typename T>
BufferRAMPrecision<T>* BufferRAMPrecision<T>::clone() const {
    return new BufferRAMPrecision<T>(*this);
}

template <typename T>
void BufferRAMPrecision<T>::setSize(size_t size) {
    return data_->resize(size);
}

template <typename T>
size_t BufferRAMPrecision<T>::getSize() const {
    return data_->size();
}

template <typename T>
void* BufferRAMPrecision<T>::getData() {
    return (data_->empty() ? nullptr : data_->data());
}

template <typename T>
const void* BufferRAMPrecision<T>::getData() const {
    return (data_->empty() ? nullptr : data_->data());
}

template <typename T>
std::vector<T>* inviwo::BufferRAMPrecision<T>::getDataContainer() {
    return data_.get();
}

template <typename T>
const std::vector<T>* BufferRAMPrecision<T>::getDataContainer() const {
    return data_.get();
}

template <typename T>
double BufferRAMPrecision<T>::getAsDouble(const size_t& pos) const {
    return util::glm_convert<double>((*data_)[pos]);
}

template <typename T>
dvec2 BufferRAMPrecision<T>::getAsDVec2(const size_t& pos) const {
    return util::glm_convert<dvec2>((*data_)[pos]);
}

template <typename T>
dvec3 BufferRAMPrecision<T>::getAsDVec3(const size_t& pos) const {
    return util::glm_convert<dvec3>((*data_)[pos]);
}

template <typename T>
dvec4 BufferRAMPrecision<T>::getAsDVec4(const size_t& pos) const {
    return util::glm_convert<dvec4>((*data_)[pos]);
}

template <typename T>
void BufferRAMPrecision<T>::setFromDouble(const size_t& pos, double val) {
    (*data_)[pos] = util::glm_convert<T>(val);
}

template <typename T>
void BufferRAMPrecision<T>::setFromDVec2(const size_t& pos, dvec2 val) {
    (*data_)[pos] = util::glm_convert<T>(val);
}

template <typename T>
void BufferRAMPrecision<T>::setFromDVec3(const size_t& pos, dvec3 val) {
    (*data_)[pos] = util::glm_convert<T>(val);
}

template <typename T>
void BufferRAMPrecision<T>::setFromDVec4(const size_t& pos, dvec4 val) {
    (*data_)[pos] = util::glm_convert<T>(val);
}

template <typename T>
double BufferRAMPrecision<T>::getAsNormalizedDouble(const size_t& pos) const {
    return util::glm_convert_normalized<double>((*data_)[pos]);
}

template <typename T>
dvec2 BufferRAMPrecision<T>::getAsNormalizedDVec2(const size_t& pos) const {
    return util::glm_convert_normalized<dvec2>((*data_)[pos]);
}

template <typename T>
dvec3 BufferRAMPrecision<T>::getAsNormalizedDVec3(const size_t& pos) const {
    return util::glm_convert_normalized<dvec3>((*data_)[pos]);
}

template <typename T>
dvec4 BufferRAMPrecision<T>::getAsNormalizedDVec4(const size_t& pos) const {
    return util::glm_convert_normalized<dvec4>((*data_)[pos]);
}

template <typename T>
void BufferRAMPrecision<T>::setFromNormalizedDouble(const size_t& pos, double val) {
    (*data_)[pos] = util::glm_convert_normalized<T>(val);
}

template <typename T>
void BufferRAMPrecision<T>::setFromNormalizedDVec2(const size_t& pos, dvec2 val) {
    (*data_)[pos] = util::glm_convert_normalized<T>(val);
}

template <typename T>
void BufferRAMPrecision<T>::setFromNormalizedDVec3(const size_t& pos, dvec3 val) {
    (*data_)[pos] = util::glm_convert_normalized<T>(val);
}

template <typename T>
void BufferRAMPrecision<T>::setFromNormalizedDVec4(const size_t& pos, dvec4 val) {
    (*data_)[pos] = util::glm_convert_normalized<T>(val);
}

template <typename T>
void BufferRAMPrecision<T>::add(const T& item) {
    data_->push_back(item);
}

template <typename T>
void BufferRAMPrecision<T>::add(std::initializer_list<T> data) {
    for (auto& elem : data) {
        data_->push_back(elem);
    }
}

template <typename T>
void BufferRAMPrecision<T>::append(const std::vector<T>* data) {
    data_->insert(data_->end(), data->begin(), data->end());
}

template <typename T>
void BufferRAMPrecision<T>::set(size_t index, const T& item) {
    data_->at(index) = item;
}

template <typename T>
T BufferRAMPrecision<T>::get(size_t index) const {
    return data_->at(index);
}

template <typename T>
T& BufferRAMPrecision<T>::get(size_t index) {
    return data_->at(index);
}

template <typename T>
void BufferRAMPrecision<T>::clear() {
    data_->clear();
}

}  // namespace

#endif  // IVW_BUFFERRAMPRECISION_H
