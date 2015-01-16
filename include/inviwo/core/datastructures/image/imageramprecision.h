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

#ifndef IVW_IMAGERAMPRECISION_H
#define IVW_IMAGERAMPRECISION_H

#include <inviwo/core/datastructures/image/imageram.h>

namespace inviwo {

template<typename T>
class ImageRAMPrecision : public ImageRAM {
public:
    ImageRAMPrecision(uvec2 dimensions = uvec2(128,128), ImageType type = COLOR_DEPTH, const DataFormatBase* format = defaultformat());
    ImageRAMPrecision(T* data, uvec2 dimensions = uvec2(128,128), ImageType type = COLOR_DEPTH, const DataFormatBase* format = defaultformat());
    ImageRAMPrecision(const ImageRAMPrecision<T>& rhs): ImageRAM(rhs.getDimension(), rhs.getImageType(), rhs.getDataFormat()) {
        *this = rhs;
    }
    ImageRAMPrecision<T>& operator=(const ImageRAMPrecision<T>& rhs) {
        if (this != &rhs) {
            delete[] data_;
            dimensions_ = rhs.getDimension();
            initialize();
            memcpy(data_, rhs.getData(), dimensions_.x*dimensions_.y*sizeof(T));
        }

        return *this;
    };
    virtual ~ImageRAMPrecision() {
        deinitialize();
    };
    virtual void initialize();
    virtual void initialize(void*);
    virtual void deinitialize();
    virtual DataRepresentation* clone() const;
    virtual void resize(uvec2 dimensions);

    void setValueFromSingleFloat(const uvec2& pos, float val);
    void setValueFromVec2Float(const uvec2& pos, vec2 val);
    void setValueFromVec3Float(const uvec2& pos, vec3 val);
    void setValueFromVec4Float(const uvec2& pos, vec4 val);

    float getValueAsSingleFloat(const uvec2& pos) const;
    vec2 getValueAsVec2Float(const uvec2& pos) const;
    vec3 getValueAsVec3Float(const uvec2& pos) const;
    vec4 getValueAsVec4Float(const uvec2& pos) const;

    vec4 getPickingValue(const uvec2& pos) const;

protected:
    void allocatePickingData();

private:
    static const DataFormatBase* defaultformat() {
        return GenericDataFormat(T)::get();
    }
};

template<typename T, size_t B>
class ImageRAMCustomPrecision : public ImageRAMPrecision<T> {
public:
    ImageRAMCustomPrecision(uvec2 dimensions = uvec2(128,128), ImageType type = COLOR_DEPTH, const DataFormatBase* format = defaultformat())
        : ImageRAMPrecision<T>(dimensions, type, format) {}
    ImageRAMCustomPrecision(T* data, uvec2 dimensions = uvec2(128,128), ImageType type = COLOR_DEPTH,
                            const DataFormatBase* format = defaultformat())
        : ImageRAMPrecision<T>(data, dimensions, type, format) {}
    virtual ~ImageRAMCustomPrecision() {};

private:
    static const DataFormatBase* defaultformat() {
        return  DataFormat<T, B>::get();
    }
};

template<typename T>
ImageRAMPrecision<T>::ImageRAMPrecision(uvec2 dimensions, ImageType type, const DataFormatBase* format) : ImageRAM(dimensions, type,
            format) {
    initialize();
}
template<typename T>
ImageRAMPrecision<T>::ImageRAMPrecision(T* data, uvec2 dimensions, ImageType type, const DataFormatBase* format) : ImageRAM(dimensions,
            type, format) {
    initialize(data);
}

template<typename T>
void ImageRAMPrecision<T>::initialize() {
    data_ = new T[dimensions_.x*dimensions_.y];
}

template<typename T>
void ImageRAMPrecision<T>::initialize(void* data) {
    if (data == NULL)
        data_ = new T[dimensions_.x*dimensions_.y];
    else
        data_ = data;
}

template<typename T>
void ImageRAMPrecision<T>::deinitialize() {
    if (data_) {
        delete[] static_cast<T*>(data_);
        data_ = NULL;
    }

    if (pickingData_) {
        delete[] static_cast<T*>(pickingData_);
        pickingData_ = NULL;
    }
}

template<typename T>
void ImageRAMPrecision<T>::resize(uvec2 dimensions) {
    dimensions_ = dimensions;
    //Delete and reallocate data_ to new size
    ImageRAM::deinitialize();
    deinitialize();
    ImageRAM::initialize();
    initialize();
}

template<typename T>
DataRepresentation* ImageRAMPrecision<T>::clone() const {
    ImageRAMPrecision* newImageRAM = new ImageRAMPrecision<T>(getDimension());
    //*newImageRAM = *this;
    return newImageRAM;
}

template<typename T>
void ImageRAMPrecision<T>::setValueFromSingleFloat(const uvec2& pos, float val) {
    T* data = static_cast<T*>(data_);
    getDataFormat()->floatToValue(val, &(data[posToIndex(pos, dimensions_)]));
}

template<typename T>
void ImageRAMPrecision<T>::setValueFromVec2Float(const uvec2& pos, vec2 val) {
    T* data = static_cast<T*>(data_);
    getDataFormat()->vec2ToValue(val, &(data[posToIndex(pos, dimensions_)]));
}

template<typename T>
void ImageRAMPrecision<T>::setValueFromVec3Float(const uvec2& pos, vec3 val) {
    T* data = static_cast<T*>(data_);
    getDataFormat()->vec3ToValue(val, &(data[posToIndex(pos, dimensions_)]));
}

template<typename T>
void ImageRAMPrecision<T>::setValueFromVec4Float(const uvec2& pos, vec4 val) {
    T* data = static_cast<T*>(data_);
    getDataFormat()->vec4ToValue(val, &(data[posToIndex(pos, dimensions_)]));
}

template<typename T>
float ImageRAMPrecision<T>::getValueAsSingleFloat(const uvec2& pos) const {
    float result;
    T* data = static_cast<T*>(data_);
    T val = data[posToIndex(pos, dimensions_)];
    result = getDataFormat()->valueToNormalizedFloat(&val);
    return result;
}

template<typename T>
vec2 ImageRAMPrecision<T>::getValueAsVec2Float(const uvec2& pos) const {
    vec2 result;
    T* data = static_cast<T*>(data_);
    T val = data[posToIndex(pos, dimensions_)];
    result = getDataFormat()->valueToNormalizedVec2Float(&val);
    return result;
}

template<typename T>
vec3 ImageRAMPrecision<T>::getValueAsVec3Float(const uvec2& pos) const {
    vec3 result;
    T* data = static_cast<T*>(data_);
    T val = data[posToIndex(pos, dimensions_)];
    result = getDataFormat()->valueToNormalizedVec3Float(&val);
    return result;
}

template<typename T>
vec4 ImageRAMPrecision<T>::getValueAsVec4Float(const uvec2& pos) const {
    vec4 result;
    T* data = static_cast<T*>(data_);
    T val = data[posToIndex(pos, dimensions_)];
    result = getDataFormat()->valueToNormalizedVec4Float(&val);
    return result;
}

template<typename T>
vec4 ImageRAMPrecision<T>::getPickingValue(const uvec2& pos) const {
    vec4 result = vec4(0.f);

    if (pickingData_) {
        T* pickData = static_cast<T*>(pickingData_);
        T val = pickData[posToIndex(pos, dimensions_)];
        result = getDataFormat()->valueToNormalizedVec4Float(&val);
    }

    return result;
}

template<typename T>
void ImageRAMPrecision<T>::allocatePickingData() {
    pickingData_ = new T[dimensions_.x*dimensions_.y];
}

#define DataFormatIdMacro(i) typedef ImageRAMCustomPrecision<Data##i::type, Data##i::bits> ImageRAM_##i;
#include <inviwo/core/util/formatsdefinefunc.h>

} // namespace

#endif // IVW_IMAGERAMPRECISION_H
