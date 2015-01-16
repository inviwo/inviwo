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

#ifndef IVW_BUFFERRAMPRECISION_H
#define IVW_BUFFERRAMPRECISION_H

#include <inviwo/core/datastructures/buffer/bufferram.h>

namespace inviwo {

template <typename T>
class BufferRAMPrecision : public BufferRAM {
public:
    BufferRAMPrecision(size_t size = 0, const DataFormatBase* format = DataFormatBase::get(),
                       BufferType type = POSITION_ATTRIB, BufferUsage usage = STATIC);
    BufferRAMPrecision(T* data, size_t size, const DataFormatBase* format = DataFormatBase::get(),
                       BufferType type = POSITION_ATTRIB, BufferUsage usage = STATIC);
    BufferRAMPrecision(const BufferRAMPrecision<T>& rhs);
    BufferRAMPrecision<T>& operator=(const BufferRAMPrecision<T>& that);
    virtual ~BufferRAMPrecision();
    virtual void initialize();
    virtual void initialize(void*);
    virtual void deinitialize();
    virtual BufferRAMPrecision<T>* clone() const;

    virtual void* getData() { return (data_->empty() ? NULL : &data_->front()); }
    virtual const void* getData() const { return (data_->empty() ? NULL : &data_->front()); }

    const std::vector<T>* getDataContainer() const { return data_; }

    void setValueFromSingleDouble(size_t index, double val);
    void setValueFromVec2Double(size_t index, dvec2 val);
    void setValueFromVec3Double(size_t index, dvec3 val);
    void setValueFromVec4Double(size_t index, dvec4 val);

    double getValueAsSingleDouble(size_t index) const;
    dvec2 getValueAsVec2Double(size_t index) const;
    dvec3 getValueAsVec3Double(size_t index) const;
    dvec4 getValueAsVec4Double(size_t index) const;

    void add(const T& item);
    void append(const std::vector<T>* data);

    void set(size_t index, const T& item);
    T get(size_t index) const;

    size_t size() const;

    void clear();

private:
    static const DataFormatBase* defaultformat() { return GenericDataFormat(T)::get(); }
    std::vector<T>* data_;
};

template <typename T, size_t B>
class BufferRAMCustomPrecision : public BufferRAMPrecision<T> {
public:
    BufferRAMCustomPrecision(size_t size = 0,
                             const DataFormatBase* format = DataFormat<T, B>::get(),
                             BufferType type = POSITION_ATTRIB, BufferUsage usage = STATIC)
        : BufferRAMPrecision<T>(size, format, type, usage) {}
    virtual ~BufferRAMCustomPrecision(){};

    BufferRAMCustomPrecision(const BufferRAMCustomPrecision<T, B>& rhs);

    BufferRAMCustomPrecision<T, B>& operator=(const BufferRAMCustomPrecision<T, B>& rhs) {
        if (this != &rhs) {
            BufferRAMPrecision<T>::operator=(rhs);
        }
        return *this;
    };

    virtual BufferRAMCustomPrecision<T, B>* clone() const;

private:
    static const DataFormatBase* defaultformat() { return DataFormat<T, B>::get(); }
};

template<typename T, size_t B>
inviwo::BufferRAMCustomPrecision<T, B>::BufferRAMCustomPrecision(const BufferRAMCustomPrecision<T, B>& rhs)
    : BufferRAMPrecision<T>(rhs) {}

template<typename T, size_t B>
BufferRAMCustomPrecision<T, B>* inviwo::BufferRAMCustomPrecision<T, B>::clone() const {
    return new BufferRAMCustomPrecision<T,B>(*this);
}

template<typename T>
BufferRAMPrecision<T>::BufferRAMPrecision(size_t size, const DataFormatBase* format, BufferType type, BufferUsage usage) :
    BufferRAM(size, format, type, usage)
    ,data_(0) {
    initialize();
}

template<typename T>
BufferRAMPrecision<T>::BufferRAMPrecision(const BufferRAMPrecision<T>& rhs) :
    BufferRAM(rhs)
    ,data_(0) {
    initialize();
    *data_ = *(rhs.data_);
}

template<typename T>
BufferRAMPrecision<T>& inviwo::BufferRAMPrecision<T>::operator=(const BufferRAMPrecision<T>& that) {
    if (this != &that) {
        size_ = that.getSize();
        initialize();
        memcpy(data_, that.getData(), size_*sizeof(T));
    }
    return *this;
}

template <typename T>
inviwo::BufferRAMPrecision<T>::~BufferRAMPrecision() {
    deinitialize();
}

template<typename T>
void BufferRAMPrecision<T>::initialize() {
    initialize(NULL);
}

template<typename T>
void BufferRAMPrecision<T>::initialize(void* data) {
    if (data_!=0)
        delete data_;

    if (data == NULL)
        data_ = new std::vector<T>(size_);
    else
        data_ = new std::vector<T>(static_cast<T*>(data), static_cast<T*>(data)+size_);
}

template<typename T>
void inviwo::BufferRAMPrecision<T>::deinitialize() {
    if (data_) {
        delete data_;
        data_ = NULL;
    }
}

template<typename T>
BufferRAMPrecision<T>* BufferRAMPrecision<T>::clone() const {
    return new BufferRAMPrecision<T>(*this);
}

template<typename T>
void BufferRAMPrecision<T>::setValueFromSingleDouble(size_t index, double val) {
    T* data = static_cast<T*>(&data_->front());
    getDataFormat()->doubleToValue(val, &(data[index]));
}

template<typename T>
void BufferRAMPrecision<T>::setValueFromVec2Double(size_t index, dvec2 val) {
    T* data = static_cast<T*>(&data_->front());
    getDataFormat()->vec2DoubleToValue(val, &(data[index]));
}

template<typename T>
void BufferRAMPrecision<T>::setValueFromVec3Double(size_t index, dvec3 val) {
    T* data = static_cast<T*>(&data_->front());
    getDataFormat()->vec3DoubleToValue(val, &(data[index]));
}

template<typename T>
void BufferRAMPrecision<T>::setValueFromVec4Double(size_t index, dvec4 val) {
    T* data = static_cast<T*>(&data_->front());
    getDataFormat()->vec4DoubleToValue(val, &(data[index]));
}

template<typename T>
double BufferRAMPrecision<T>::getValueAsSingleDouble(size_t index) const {
    double result;
    T* data = static_cast<T*>(&data_->front());
    T val = data[index];
    result = getDataFormat()->valueToNormalizedDouble(&val);
    return result;
}

template<typename T>
dvec2 BufferRAMPrecision<T>::getValueAsVec2Double(size_t index) const {
    dvec2 result;
    T* data = static_cast<T*>(&data_->front());
    T val = data[index];
    result = getDataFormat()->valueToNormalizedVec2Double(&val);
    return result;
}

template<typename T>
dvec3 BufferRAMPrecision<T>::getValueAsVec3Double(size_t index) const {
    dvec3 result;
    T* data = static_cast<T*>(&data_->front());
    T val = data[index];
    result = getDataFormat()->valueToNormalizedVec3Double(&val);
    return result;
}

template<typename T>
dvec4 BufferRAMPrecision<T>::getValueAsVec4Double(size_t index) const {
    dvec4 result;
    T* data = static_cast<T*>(&data_->front());
    T val = data[index];
    result = getDataFormat()->valueToNormalizedVec4Double(&val);
    return result;
}

template<typename T>
void BufferRAMPrecision<T>::add(const T& item) {
    data_->push_back(item);
    size_ = data_->size();
}

template<typename T>
void BufferRAMPrecision<T>::append(const std::vector<T>* data) {
    data_->insert(data_->end(), data->begin(),data->end());
    size_ = data_->size();
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
size_t BufferRAMPrecision<T>::size() const {
    return data_->size();
}

template<typename T>
void BufferRAMPrecision<T>::clear() {
    data_->clear();
    size_ = 0;
}

#define DataFormatIdMacro(i) typedef BufferRAMCustomPrecision<Data##i::type, Data##i::bits> BufferRAM_##i;
#include <inviwo/core/util/formatsdefinefunc.h>
#undef DataFormatIdMacro

typedef BufferRAM_Vec4FLOAT32 ColorBufferRAM;
typedef BufferRAM_FLOAT32 CurvatureBufferRAM;
typedef BufferRAM_UINT32 IndexBufferRAM;
typedef BufferRAM_Vec2FLOAT32 Position2dBufferRAM;
typedef BufferRAM_Vec2FLOAT32 TexCoord2dBufferRAM;
typedef BufferRAM_Vec3FLOAT32 Position3dBufferRAM;
typedef BufferRAM_Vec3FLOAT32 TexCoord3dBufferRAM;
typedef BufferRAM_Vec3FLOAT32 NormalBufferRAM;

} // namespace

#endif // IVW_BUFFERRAMPRECISION_H
