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

namespace inviwo {

template <typename T>
class BufferRAMPrecision : public BufferRAM {
public:
    BufferRAMPrecision(size_t size = 0, const DataFormatBase* format = DataFormatBase::get(),
                       BufferType type = POSITION_ATTRIB, BufferUsage usage = STATIC);
    BufferRAMPrecision(const BufferRAMPrecision<T>& rhs);
    BufferRAMPrecision<T>& operator=(const BufferRAMPrecision<T>& that);
    virtual ~BufferRAMPrecision();
    virtual BufferRAMPrecision<T>* clone() const override;

    virtual void setSize(size_t size) override;
    virtual size_t getSize() const override;

    virtual void* getData() override;
    virtual const void* getData() const override;
    std::vector<T>* getDataContainer();
    const std::vector<T>* getDataContainer() const;

    virtual void setValueFromSingleDouble(size_t index, double val) override;
    virtual void setValueFromVec2Double(size_t index, dvec2 val) override;
    virtual void setValueFromVec3Double(size_t index, dvec3 val) override;
    virtual void setValueFromVec4Double(size_t index, dvec4 val) override;

    virtual double getValueAsSingleDouble(size_t index) const override;
    virtual dvec2 getValueAsVec2Double(size_t index) const override;
    virtual dvec3 getValueAsVec3Double(size_t index) const override;
    virtual dvec4 getValueAsVec4Double(size_t index) const override;

    void add(const T& item);
    void append(const std::vector<T>* data);

    void set(size_t index, const T& item);
    T get(size_t index) const;
    T& get(size_t index);

    void clear();

private:
    static const DataFormatBase* defaultformat() { return DataFormat<T>::get(); }
    std::unique_ptr<std::vector<T>> data_;
};


template <typename T>
BufferRAMPrecision<T>::BufferRAMPrecision(size_t size, const DataFormatBase* format,
                                          BufferType type, BufferUsage usage)
    : BufferRAM(format, type, usage), data_(new std::vector<T>(size)) {}

template<typename T>
BufferRAMPrecision<T>::BufferRAMPrecision(const BufferRAMPrecision<T>& rhs)
    : BufferRAM(rhs)
    , data_(new std::vector<T>(rhs.data_->size())) {
    *data_ = *(rhs.data_);
}

template<typename T>
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
inviwo::BufferRAMPrecision<T>::~BufferRAMPrecision() {}

template<typename T>
BufferRAMPrecision<T>* BufferRAMPrecision<T>::clone() const {
    return new BufferRAMPrecision<T>(*this);
}

template<typename T>
void BufferRAMPrecision<T>::setSize(size_t size) {
    return data_->resize(size);
}

template<typename T>
size_t BufferRAMPrecision<T>::getSize() const {
    return data_->size();
}

template<typename T>
void* BufferRAMPrecision<T>::getData() {
    return (data_->empty() ? nullptr : data_->data());
}

template<typename T>
const void* BufferRAMPrecision<T>::getData() const {
    return (data_->empty() ? nullptr : data_->data());
}


template <typename T>
std::vector<T>* inviwo::BufferRAMPrecision<T>::getDataContainer() {
    return data_.get();
}

template<typename T>
const std::vector<T>* BufferRAMPrecision<T>::getDataContainer() const { return data_.get(); }


template<typename T>
void BufferRAMPrecision<T>::setValueFromSingleDouble(size_t index, double val) {
    (*data_)[index] = util::glm_convert<T>(val);
}

template<typename T>
void BufferRAMPrecision<T>::setValueFromVec2Double(size_t index, dvec2 val) {
    (*data_)[index] = util::glm_convert<T>(val);
}

template<typename T>
void BufferRAMPrecision<T>::setValueFromVec3Double(size_t index, dvec3 val) {
    (*data_)[index] = util::glm_convert<T>(val);
}

template<typename T>
void BufferRAMPrecision<T>::setValueFromVec4Double(size_t index, dvec4 val) {
    (*data_)[index] = util::glm_convert<T>(val);
}

template<typename T>
double BufferRAMPrecision<T>::getValueAsSingleDouble(size_t index) const {
    return util::glm_convert_normalized<double>((*data_)[index]);
}

template<typename T>
dvec2 BufferRAMPrecision<T>::getValueAsVec2Double(size_t index) const {
    return util::glm_convert_normalized<dvec2>((*data_)[index]);
}

template<typename T>
dvec3 BufferRAMPrecision<T>::getValueAsVec3Double(size_t index) const {
    return util::glm_convert_normalized<dvec3>((*data_)[index]);
}

template<typename T>
dvec4 BufferRAMPrecision<T>::getValueAsVec4Double(size_t index) const {
    return util::glm_convert_normalized<dvec4>((*data_)[index]);
}

template<typename T>
void BufferRAMPrecision<T>::add(const T& item) {
    data_->push_back(item);
}

template <typename T>
void BufferRAMPrecision<T>::append(const std::vector<T>* data) {
    data_->insert(data_->end(), data->begin(), data->end());
}
    
template<typename T>
void BufferRAMPrecision<T>::set(size_t index, const T& item) {
    data_->at(index) = item;
}

template<typename T>
T BufferRAMPrecision<T>::get(size_t index) const {
    return data_->at(index);
}

template<typename T>
T& BufferRAMPrecision<T>::get(size_t index) {
    return data_->at(index);
}

template<typename T>
void BufferRAMPrecision<T>::clear() {
    data_->clear();
}

typedef BufferRAMPrecision<vec4> ColorBufferRAM;
typedef BufferRAMPrecision<float> CurvatureBufferRAM;
typedef BufferRAMPrecision<std::uint32_t> IndexBufferRAM;
typedef BufferRAMPrecision<vec2> Position2dBufferRAM;
typedef BufferRAMPrecision<vec2> TexCoord2dBufferRAM;
typedef BufferRAMPrecision<vec3> Position3dBufferRAM;
typedef BufferRAMPrecision<vec3> TexCoord3dBufferRAM;
typedef BufferRAMPrecision<vec3> NormalBufferRAM;

} // namespace

#endif // IVW_BUFFERRAMPRECISION_H
