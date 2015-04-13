/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_LAYERRAMPRECISION_H
#define IVW_LAYERRAMPRECISION_H

#include <inviwo/core/datastructures/image/layerram.h>

namespace inviwo {

template <typename T>
class LayerRAMPrecision : public LayerRAM {
public:
    LayerRAMPrecision(uvec2 dimensions = uvec2(8, 8), LayerType type = COLOR_LAYER,
                      const DataFormatBase* format = defaultformat());
    LayerRAMPrecision(T* data, uvec2 dimensions = uvec2(8, 8), LayerType type = COLOR_LAYER,
                      const DataFormatBase* format = defaultformat());
    LayerRAMPrecision(const LayerRAMPrecision<T>& rhs);
    LayerRAMPrecision<T>& operator=(const LayerRAMPrecision<T>& that);
    virtual LayerRAMPrecision<T>* clone() const;
    virtual ~LayerRAMPrecision();

    virtual void initialize();
    virtual void initialize(void*);
    virtual void deinitialize();
    virtual void resize(uvec2 dimensions);

    void setValueFromSingleDouble(const uvec2& pos, double val);
    void setValueFromVec2Double(const uvec2& pos, dvec2 val);
    void setValueFromVec3Double(const uvec2& pos, dvec3 val);
    void setValueFromVec4Double(const uvec2& pos, dvec4 val);

    double getValueAsSingleDouble(const uvec2& pos) const;
    dvec2 getValueAsVec2Double(const uvec2& pos) const;
    dvec3 getValueAsVec3Double(const uvec2& pos) const;
    dvec4 getValueAsVec4Double(const uvec2& pos) const;

private:
    static const DataFormatBase* defaultformat() { return DataFormat<T>::get(); }
};


template <typename T>
LayerRAMPrecision<T>::LayerRAMPrecision(uvec2 dimensions, LayerType type,
                                        const DataFormatBase* format)
    : LayerRAM(dimensions, type, format) {
    initialize();
}
template <typename T>
LayerRAMPrecision<T>::LayerRAMPrecision(T* data, uvec2 dimensions, LayerType type,
                                        const DataFormatBase* format)
    : LayerRAM(dimensions, type, format) {
    initialize(data);
}

template <typename T>
LayerRAMPrecision<T>::LayerRAMPrecision(const LayerRAMPrecision<T>& rhs)
    : LayerRAM(rhs) {
    initialize();
    std::memcpy(data_, rhs.getData(), dimensions_.x * dimensions_.y * sizeof(T));
}

template <typename T>
LayerRAMPrecision<T>& LayerRAMPrecision<T>::operator=(const LayerRAMPrecision<T>& that) {
    if (this != &that) {
        LayerRAM::operator=(that);
        delete[] data_;
        initialize();
        std::memcpy(data_, that.getData(), dimensions_.x*dimensions_.y*sizeof(T));
    }

    return *this;
};

template <typename T>
LayerRAMPrecision<T>* LayerRAMPrecision<T>::clone() const {
    return new LayerRAMPrecision<T>(*this);
}

template <typename T>
LayerRAMPrecision<T>::~LayerRAMPrecision() {
    deinitialize();
};

template<typename T>
void LayerRAMPrecision<T>::initialize() {
    data_ = new T[dimensions_.x*dimensions_.y]();
}

template<typename T>
void LayerRAMPrecision<T>::initialize(void* data) {
    if (data == nullptr)
        data_ = new T[dimensions_.x*dimensions_.y]();
    else
        data_ = data;
}

template<typename T>
void LayerRAMPrecision<T>::deinitialize() {
    if (data_) {
        delete[] static_cast<T*>(data_);
        data_ = nullptr;
    }
}

template<typename T>
void LayerRAMPrecision<T>::resize(uvec2 dimensions) {
    dimensions_ = dimensions;
    //Delete and reallocate data_ to new size
    deinitialize();
    initialize();
}

template<typename T>
void LayerRAMPrecision<T>::setValueFromSingleDouble(const uvec2& pos, double val) {
    T* data = static_cast<T*>(data_);
    getDataFormat()->doubleToValue(val, &(data[posToIndex(pos, dimensions_)]));
}

template<typename T>
void LayerRAMPrecision<T>::setValueFromVec2Double(const uvec2& pos, dvec2 val) {
    T* data = static_cast<T*>(data_);
    getDataFormat()->vec2DoubleToValue(val, &(data[posToIndex(pos, dimensions_)]));
}

template<typename T>
void LayerRAMPrecision<T>::setValueFromVec3Double(const uvec2& pos, dvec3 val) {
    T* data = static_cast<T*>(data_);
    getDataFormat()->vec3DoubleToValue(val, &(data[posToIndex(pos, dimensions_)]));
}

template<typename T>
void LayerRAMPrecision<T>::setValueFromVec4Double(const uvec2& pos, dvec4 val) {
    T* data = static_cast<T*>(data_);
    getDataFormat()->vec4DoubleToValue(val, &(data[posToIndex(pos, dimensions_)]));
}

template<typename T>
double LayerRAMPrecision<T>::getValueAsSingleDouble(const uvec2& pos) const {
    double result;
    T* data = static_cast<T*>(data_);
    T val = data[posToIndex(pos, dimensions_)];
    result = getDataFormat()->valueToNormalizedDouble(&val);
    return result;
}

template<typename T>
dvec2 LayerRAMPrecision<T>::getValueAsVec2Double(const uvec2& pos) const {
    dvec2 result;
    T* data = static_cast<T*>(data_);
    T val = data[posToIndex(pos, dimensions_)];
    result = getDataFormat()->valueToNormalizedVec2Double(&val);
    return result;
}

template<typename T>
dvec3 LayerRAMPrecision<T>::getValueAsVec3Double(const uvec2& pos) const {
    dvec3 result;
    T* data = static_cast<T*>(data_);
    T val = data[posToIndex(pos, dimensions_)];
    result = getDataFormat()->valueToNormalizedVec3Double(&val);
    return result;
}

template<typename T>
dvec4 LayerRAMPrecision<T>::getValueAsVec4Double(const uvec2& pos) const {
    dvec4 result;
    T* data = static_cast<T*>(data_);
    T val = data[posToIndex(pos, dimensions_)];
    result = getDataFormat()->valueToNormalizedVec4Double(&val);
    return result;
}

#define DataFormatIdMacro(i) typedef LayerRAMPrecision<Data##i::type> LayerRAM_##i;
#include <inviwo/core/util/formatsdefinefunc.h>

} // namespace

#endif // IVW_LAYERRAMPRECISION_H
