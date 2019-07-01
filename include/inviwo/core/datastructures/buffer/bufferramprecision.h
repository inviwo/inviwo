/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

/**
 * \ingroup datastructures
 */
template <typename T, BufferTarget Target = BufferTarget::Data>
class BufferRAMPrecision : public BufferRAM {
public:
    using type = T;
    static const BufferTarget target = Target;

    explicit BufferRAMPrecision(BufferUsage usage = BufferUsage::Static);
    explicit BufferRAMPrecision(size_t size, BufferUsage usage = BufferUsage::Static);
    explicit BufferRAMPrecision(std::vector<T> data, BufferUsage usage = BufferUsage::Static);
    BufferRAMPrecision(const BufferRAMPrecision<T, Target>& rhs) = default;
    BufferRAMPrecision<T, Target>& operator=(const BufferRAMPrecision<T, Target>& that) = default;
    virtual ~BufferRAMPrecision() = default;
    virtual BufferRAMPrecision<T, Target>* clone() const override;

    virtual void setSize(size_t size) override;
    virtual size_t getSize() const override;

    virtual void* getData() override;
    virtual const void* getData() const override;
    std::vector<T>& getDataContainer();
    const std::vector<T>& getDataContainer() const;

    virtual void reserve(size_t size) override;

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
    void append(const std::vector<T>& data);

    T& operator[](size_t i);
    const T& operator[](size_t i) const;

    void set(size_t index, const T& item);
    T get(size_t index) const;
    T& get(size_t index);

    virtual void clear() override;

private:
    std::vector<T> data_;
};

using FloatBufferRAM = BufferRAMPrecision<float>;
using Vec2BufferRAM = BufferRAMPrecision<vec2>;
using Vec3BufferRAM = BufferRAMPrecision<vec3>;
using Vec4BufferRAM = BufferRAMPrecision<vec4>;
// Used for index buffers
using IndexBufferRAM = BufferRAMPrecision<std::uint32_t, BufferTarget::Index>;

template <typename T, BufferTarget Target>
const T& inviwo::BufferRAMPrecision<T, Target>::operator[](size_t i) const {
    return data_[i];
}

template <typename T, BufferTarget Target>
T& inviwo::BufferRAMPrecision<T, Target>::operator[](size_t i) {
    return data_[i];
}

template <typename T, BufferTarget Target>
BufferRAMPrecision<T, Target>::BufferRAMPrecision(BufferUsage usage)
    : BufferRAMPrecision(0, usage) {}

template <typename T, BufferTarget Target>
BufferRAMPrecision<T, Target>::BufferRAMPrecision(size_t size, BufferUsage usage)
    : BufferRAM(DataFormat<T>::get(), usage, Target), data_(size) {}

template <typename T, BufferTarget Target>
inviwo::BufferRAMPrecision<T, Target>::BufferRAMPrecision(std::vector<T> data, BufferUsage usage)
    : BufferRAM(DataFormat<T>::get(), usage, Target), data_(std::move(data)) {}

template <typename T, BufferTarget Target>
BufferRAMPrecision<T, Target>* BufferRAMPrecision<T, Target>::clone() const {
    return new BufferRAMPrecision<T, Target>(*this);
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::setSize(size_t size) {
    return data_.resize(size);
}

template <typename T, BufferTarget Target>
size_t BufferRAMPrecision<T, Target>::getSize() const {
    return data_.size();
}

template <typename T, BufferTarget Target>
void* BufferRAMPrecision<T, Target>::getData() {
    return (data_.empty() ? nullptr : data_.data());
}

template <typename T, BufferTarget Target>
const void* BufferRAMPrecision<T, Target>::getData() const {
    return (data_.empty() ? nullptr : data_.data());
}

template <typename T, BufferTarget Target>
std::vector<T>& inviwo::BufferRAMPrecision<T, Target>::getDataContainer() {
    return data_;
}

template <typename T, BufferTarget Target>
const std::vector<T>& BufferRAMPrecision<T, Target>::getDataContainer() const {
    return data_;
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::reserve(size_t size) {
    data_.reserve(size);
}

template <typename T, BufferTarget Target>
double BufferRAMPrecision<T, Target>::getAsDouble(const size_t& pos) const {
    return util::glm_convert<double>(data_[pos]);
}

template <typename T, BufferTarget Target>
dvec2 BufferRAMPrecision<T, Target>::getAsDVec2(const size_t& pos) const {
    return util::glm_convert<dvec2>(data_[pos]);
}

template <typename T, BufferTarget Target>
dvec3 BufferRAMPrecision<T, Target>::getAsDVec3(const size_t& pos) const {
    return util::glm_convert<dvec3>(data_[pos]);
}

template <typename T, BufferTarget Target>
dvec4 BufferRAMPrecision<T, Target>::getAsDVec4(const size_t& pos) const {
    return util::glm_convert<dvec4>(data_[pos]);
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::setFromDouble(const size_t& pos, double val) {
    data_[pos] = util::glm_convert<T>(val);
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::setFromDVec2(const size_t& pos, dvec2 val) {
    data_[pos] = util::glm_convert<T>(val);
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::setFromDVec3(const size_t& pos, dvec3 val) {
    data_[pos] = util::glm_convert<T>(val);
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::setFromDVec4(const size_t& pos, dvec4 val) {
    data_[pos] = util::glm_convert<T>(val);
}

template <typename T, BufferTarget Target>
double BufferRAMPrecision<T, Target>::getAsNormalizedDouble(const size_t& pos) const {
    return util::glm_convert_normalized<double>(data_[pos]);
}

template <typename T, BufferTarget Target>
dvec2 BufferRAMPrecision<T, Target>::getAsNormalizedDVec2(const size_t& pos) const {
    return util::glm_convert_normalized<dvec2>(data_[pos]);
}

template <typename T, BufferTarget Target>
dvec3 BufferRAMPrecision<T, Target>::getAsNormalizedDVec3(const size_t& pos) const {
    return util::glm_convert_normalized<dvec3>(data_[pos]);
}

template <typename T, BufferTarget Target>
dvec4 BufferRAMPrecision<T, Target>::getAsNormalizedDVec4(const size_t& pos) const {
    return util::glm_convert_normalized<dvec4>(data_[pos]);
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::setFromNormalizedDouble(const size_t& pos, double val) {
    data_[pos] = util::glm_convert_normalized<T>(val);
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::setFromNormalizedDVec2(const size_t& pos, dvec2 val) {
    data_[pos] = util::glm_convert_normalized<T>(val);
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::setFromNormalizedDVec3(const size_t& pos, dvec3 val) {
    data_[pos] = util::glm_convert_normalized<T>(val);
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::setFromNormalizedDVec4(const size_t& pos, dvec4 val) {
    data_[pos] = util::glm_convert_normalized<T>(val);
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::add(const T& item) {
    data_.push_back(item);
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::add(std::initializer_list<T> data) {
    for (auto& elem : data) {
        data_.push_back(elem);
    }
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::append(const std::vector<T>* data) {
    data_.insert(data_.end(), data->begin(), data->end());
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::append(const std::vector<T>& data) {
    data_.insert(data_.end(), data.begin(), data.end());
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::set(size_t index, const T& item) {
    data_[index] = item;
}

template <typename T, BufferTarget Target>
T BufferRAMPrecision<T, Target>::get(size_t index) const {
    return data_[index];
}

template <typename T, BufferTarget Target>
T& BufferRAMPrecision<T, Target>::get(size_t index) {
    return data_[index];
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::clear() {
    data_.clear();
}

}  // namespace inviwo

#endif  // IVW_BUFFERRAMPRECISION_H
