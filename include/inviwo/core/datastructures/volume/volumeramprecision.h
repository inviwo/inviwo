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

#ifndef IVW_VOLUMERAMPRECISION_H
#define IVW_VOLUMERAMPRECISION_H

#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramoperationexecuter.h>

namespace inviwo {

template <typename T>
class VolumeRAMPrecision : public VolumeRAM {
public:
    VolumeRAMPrecision(uvec3 dimensions = uvec3(128, 128, 128),
                       const DataFormatBase* format = defaultformat());
    VolumeRAMPrecision(T* data, uvec3 dimensions = uvec3(128, 128, 128),
                       const DataFormatBase* format = defaultformat());
    VolumeRAMPrecision(const VolumeRAMPrecision<T>& rhs);
    VolumeRAMPrecision<T>& operator=(const VolumeRAMPrecision<T>& that);
    virtual VolumeRAMPrecision<T>* clone() const;
    virtual ~VolumeRAMPrecision();

    virtual void performOperation(DataOperation* dop) const;

    virtual void initialize();
    virtual void initialize(void*);
    virtual void deinitialize();

    using VolumeRAM::getData;
    void* getData(size_t);
    const void* getData(size_t) const;

    void setDimensions(uvec3 dimensions);

    void setValueFromSingleDouble(const uvec3& pos, double val);
    void setValueFromVec2Double(const uvec3& pos, dvec2 val);
    void setValueFromVec3Double(const uvec3& pos, dvec3 val);
    void setValueFromVec4Double(const uvec3& pos, dvec4 val);

    void setValuesFromVolume(const VolumeRAM* src, const uvec3& dstOffset, const uvec3& subSize,
                             const uvec3& subOffset);

    double getValueAsSingleDouble(const uvec3& pos) const;
    dvec2 getValueAsVec2Double(const uvec3& pos) const;
    dvec3 getValueAsVec3Double(const uvec3& pos) const;
    dvec4 getValueAsVec4Double(const uvec3& pos) const;

private:
    static const DataFormatBase* defaultformat() { return GenericDataFormat(T)::get(); }
};

template <typename T, size_t B>
class VolumeRAMCustomPrecision : public VolumeRAMPrecision<T> {
public:
    VolumeRAMCustomPrecision(uvec3 dimensions = uvec3(128, 128, 128),
                             const DataFormatBase* format = defaultformat())
        : VolumeRAMPrecision<T>(dimensions, format) {}
    VolumeRAMCustomPrecision(T* data, uvec3 dimensions = uvec3(128, 128, 128),
                             const DataFormatBase* format = defaultformat())
        : VolumeRAMPrecision<T>(data, dimensions, format) {}

    VolumeRAMCustomPrecision(const VolumeRAMCustomPrecision<T, B>& rhs)
        : VolumeRAMPrecision<T>(rhs) {}
    VolumeRAMCustomPrecision<T, B>& operator=(const VolumeRAMCustomPrecision<T, B>& that) {
        if (this != &that) {
            VolumeRAMPrecision<T>::operator=(that); 
        }
        return *this;
    }
    virtual VolumeRAMCustomPrecision<T, B>* clone() const {
        return new VolumeRAMCustomPrecision<T, B>(*this);
    }

    virtual ~VolumeRAMCustomPrecision() {};
    void performOperation(DataOperation*) const;

private:
    static const DataFormatBase* defaultformat() { return DataFormat<T, B>::get(); }
};

template<typename T>
VolumeRAMPrecision<T>::VolumeRAMPrecision(uvec3 dimensions, const DataFormatBase* format) 
    : VolumeRAM(dimensions, format) {
    initialize();
}

template<typename T>
VolumeRAMPrecision<T>::VolumeRAMPrecision(T* data, uvec3 dimensions, const DataFormatBase* format) 
    : VolumeRAM(dimensions, format) {
    initialize(data);
}

template <typename T>
VolumeRAMPrecision<T>::VolumeRAMPrecision(const VolumeRAMPrecision<T>& rhs)
    : VolumeRAM(rhs) {
    initialize();
    memcpy(data_, rhs.getData(), dimensions_.x * dimensions_.y * dimensions_.z * sizeof(T));
}

template <typename T>
VolumeRAMPrecision<T>& VolumeRAMPrecision<T>::operator=(const VolumeRAMPrecision<T>& that) {
    if (this != &that) {
        VolumeRAM::operator=(that);
        delete[] data_;
        dimensions_ = that.getDimensions();
        initialize();
        memcpy(data_, that.getData(), dimensions_.x*dimensions_.y*dimensions_.z*sizeof(T));
    }
    return *this;
};

template <typename T>
VolumeRAMPrecision<T>::~VolumeRAMPrecision() {
    deinitialize();
};

template<typename T>
void VolumeRAMPrecision<T>::initialize() {
    data_ = new T[dimensions_.x*dimensions_.y*dimensions_.z];
}

template<typename T>
void VolumeRAMPrecision<T>::initialize(void* data) {
    if (!data)
        data_ = new T[dimensions_.x*dimensions_.y*dimensions_.z];
    else
        data_ = data;
}

template<typename T>
void inviwo::VolumeRAMPrecision<T>::deinitialize() {
    if (data_ && ownsDataPtr_) {
        delete[] static_cast<T*>(data_);
        data_ = NULL;
    }
}

template<typename T>
VolumeRAMPrecision<T>* VolumeRAMPrecision<T>::clone() const {
    return new VolumeRAMPrecision<T>(*this);
}

template<typename T>
void VolumeRAMPrecision<T>::performOperation(DataOperation* dop) const {
    executeOperationOnVolumeRAMPrecision<T, GenericDataBits(T)>(dop);
}

template<typename T, size_t B>
void VolumeRAMCustomPrecision<T,B>::performOperation(DataOperation* dop) const {
    executeOperationOnVolumeRAMPrecision<T, B>(dop);
}

template<typename T>
void* VolumeRAMPrecision<T>::getData(size_t pos){
    return static_cast<T*>(data_)+pos;
}

template<typename T>
const void* VolumeRAMPrecision<T>::getData(size_t pos) const{
    return const_cast<const T*>(static_cast<T*>(data_))+pos;
}

template<typename T>
void VolumeRAMPrecision<T>::setDimensions(uvec3 dimensions) { 
    dimensions_ = dimensions; 
    deinitialize(); 
    initialize(); 
}

template<typename T>
void VolumeRAMPrecision<T>::setValueFromSingleDouble(const uvec3& pos, double val) {
    T* data = static_cast<T*>(data_);
    getDataFormat()->doubleToValue(val, &(data[posToIndex(pos, dimensions_)]));
}

template<typename T>
void VolumeRAMPrecision<T>::setValueFromVec2Double(const uvec3& pos, dvec2 val) {
    T* data = static_cast<T*>(data_);
    getDataFormat()->vec2DoubleToValue(val, &(data[posToIndex(pos, dimensions_)]));
}

template<typename T>
void VolumeRAMPrecision<T>::setValueFromVec3Double(const uvec3& pos, dvec3 val) {
    T* data = static_cast<T*>(data_);
    getDataFormat()->vec3DoubleToValue(val, &(data[posToIndex(pos, dimensions_)]));
}

template<typename T>
void VolumeRAMPrecision<T>::setValueFromVec4Double(const uvec3& pos, dvec4 val) {
    T* data = static_cast<T*>(data_);
    getDataFormat()->vec4DoubleToValue(val, &(data[posToIndex(pos, dimensions_)]));
}

template<typename T>
void VolumeRAMPrecision<T>::setValuesFromVolume(const VolumeRAM* src, const uvec3& dstOffset, const uvec3& subSize, const uvec3& subOffset) {
    const T* srcData = reinterpret_cast<const T*>(src->getData());
    T* dstData = static_cast<T*>(data_);

    uvec3 dataDims = getDimensions();
    size_t initialStartPos = (dstOffset.z * (dataDims.x*dataDims.y))+(dstOffset.y * dataDims.x) + dstOffset.x;

    uvec3 srcDims = src->getDimensions();
    size_t dataSize = subSize.x*getDataFormat()->getBytesAllocated();

    size_t volumePos;
    size_t subVolumePos;
    for (size_t i=0; i < subSize.z; i++) {
        for (size_t j=0; j < subSize.y; j++) {
            volumePos =  (j*dataDims.x) + (i*dataDims.x*dataDims.y);
            subVolumePos = ((j+subOffset.y)*srcDims.x) + ((i+subOffset.z)*srcDims.x*srcDims.y) + subOffset.x;
            memcpy((dstData + volumePos + initialStartPos), (srcData + subVolumePos), dataSize);
        }
    }
}

template<typename T>
double VolumeRAMPrecision<T>::getValueAsSingleDouble(const uvec3& pos) const {
    double result;
    T* data = static_cast<T*>(data_);
    T val = data[posToIndex(pos, dimensions_)];
    result = getDataFormat()->valueToNormalizedDouble(&val);
    return result;
}

template<typename T>
dvec2 VolumeRAMPrecision<T>::getValueAsVec2Double(const uvec3& pos) const {
    dvec2 result;
    T* data = static_cast<T*>(data_);
    T val = data[posToIndex(pos, dimensions_)];
    result = getDataFormat()->valueToNormalizedVec2Double(&val);
    return result;
}

template<typename T>
dvec3 VolumeRAMPrecision<T>::getValueAsVec3Double(const uvec3& pos) const {
    dvec3 result;
    T* data = static_cast<T*>(data_);
    T val = data[posToIndex(pos, dimensions_)];
    result = getDataFormat()->valueToNormalizedVec3Double(&val);
    return result;
}

template<typename T>
dvec4 VolumeRAMPrecision<T>::getValueAsVec4Double(const uvec3& pos) const {
    dvec4 result;
    T* data = static_cast<T*>(data_);
    T val = data[posToIndex(pos, dimensions_)];
    result = getDataFormat()->valueToNormalizedVec4Double(&val);
    return result;
}

#define DataFormatIdMacro(i) typedef VolumeRAMCustomPrecision<Data##i::type, Data##i::bits> VolumeRAM_##i;
#include <inviwo/core/util/formatsdefinefunc.h>

} // namespace

#endif // IVW_VOLUMERAMPRECISION_H
