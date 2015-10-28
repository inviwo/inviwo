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
    LayerRAMPrecision(size2_t dimensions = size2_t(8, 8), LayerType type = LayerType::Color);
    LayerRAMPrecision(T* data, size2_t dimensions = size2_t(8, 8), LayerType type = LayerType::Color);
    LayerRAMPrecision(const LayerRAMPrecision<T>& rhs);
    LayerRAMPrecision<T>& operator=(const LayerRAMPrecision<T>& that);
    virtual LayerRAMPrecision<T>* clone() const override;
    virtual ~LayerRAMPrecision();

    virtual void* getData() override;
    virtual const void* getData() const override;
    virtual void setData(void* data, size2_t dimensions) override;

    /**
     * Resize the representation to dimension. This is destructive, the data will not be
     * preserved. Use copyRepresentationsTo to update the data.
     */
    virtual void setDimensions(size2_t dimensions) override;

    void setValueFromSingleDouble(const size2_t& pos, double val) override;
    void setValueFromVec2Double(const size2_t& pos, dvec2 val) override;
    void setValueFromVec3Double(const size2_t& pos, dvec3 val) override;
    void setValueFromVec4Double(const size2_t& pos, dvec4 val) override;

    double getValueAsSingleDouble(const size2_t& pos) const override;
    dvec2 getValueAsVec2Double(const size2_t& pos) const override;
    dvec3 getValueAsVec3Double(const size2_t& pos) const override;
    dvec4 getValueAsVec4Double(const size2_t& pos) const override;

private:
    std::unique_ptr<T[]> data_;
};

/**
 * Factory for layers.
 * Creates an LayerRAM with data type specified by format.
 *
 * @param dimensions of layer to create.
 * @param format of layer to create.
 * @return nullptr if no valid format was specified.
 */
IVW_CORE_API std::shared_ptr<LayerRAM> createLayerRAM(const size2_t& dimensions, LayerType type,
                                      const DataFormatBase* format);

struct IVW_CORE_API LayerRAMDispatcher {
    using type = std::shared_ptr<LayerRAM>;
    template <class T>
    std::shared_ptr<LayerRAM> dispatch(const size2_t& dimensions, LayerType type) {
        using F = typename T::type;
        return std::make_shared<LayerRAMPrecision<F>>(dimensions, type);
    }
};

template <typename T>
LayerRAMPrecision<T>::LayerRAMPrecision(size2_t dimensions, LayerType type)
    : LayerRAM(dimensions, type, DataFormat<T>::get())
    , data_(new T[dimensions_.x * dimensions_.y]()) {}

template <typename T>
LayerRAMPrecision<T>::LayerRAMPrecision(T* data, size2_t dimensions, LayerType type)
    : LayerRAM(dimensions, type, DataFormat<T>::get())
    , data_(data ? data : new T[dimensions_.x * dimensions_.y]()) {}

template <typename T>
LayerRAMPrecision<T>::LayerRAMPrecision(const LayerRAMPrecision<T>& rhs)
    : LayerRAM(rhs), data_(new T[dimensions_.x * dimensions_.y]) {
    std::memcpy(data_.get(), rhs.data_.get(), dimensions_.x * dimensions_.y * sizeof(T));
}

template <typename T>
LayerRAMPrecision<T>& LayerRAMPrecision<T>::operator=(const LayerRAMPrecision<T>& that) {
    if (this != &that) {
        LayerRAM::operator=(that);

        auto dim = that.dimensions_;
        auto data = util::make_unique<T[]>(dim.x * dim.y);
        std::memcpy(data.get(), that.data_.get(), dim.x * dim.y * sizeof(T));
        data_.swap(data);
        std::swap(dim, dimensions_);
    }

    return *this;
};

template <typename T>
LayerRAMPrecision<T>* LayerRAMPrecision<T>::clone() const {
    return new LayerRAMPrecision<T>(*this);
}

template <typename T>
LayerRAMPrecision<T>::~LayerRAMPrecision(){};

template <typename T>
void* LayerRAMPrecision<T>::getData() {
    return data_.get();
}
template <typename T>
const void* LayerRAMPrecision<T>::getData() const {
    return const_cast<const T*>(data_.get());
}

template <typename T>
void inviwo::LayerRAMPrecision<T>::setData(void* d, size2_t dimensions) {
    std::unique_ptr<T[]> data(static_cast<T*>(d));
    data_.swap(data);
    std::swap(dimensions_, dimensions);
}

template <typename T>
void LayerRAMPrecision<T>::setDimensions(size2_t dimensions) {
    auto data = util::make_unique<T[]>(dimensions.x * dimensions.y);
    data_.swap(data);
    std::swap(dimensions, dimensions_);
}

template <typename T>
void LayerRAMPrecision<T>::setValueFromSingleDouble(const size2_t& pos, double val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
void LayerRAMPrecision<T>::setValueFromVec2Double(const size2_t& pos, dvec2 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
void LayerRAMPrecision<T>::setValueFromVec3Double(const size2_t& pos, dvec3 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
void LayerRAMPrecision<T>::setValueFromVec4Double(const size2_t& pos, dvec4 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
double LayerRAMPrecision<T>::getValueAsSingleDouble(const size2_t& pos) const {
    return util::glm_convert_normalized<double>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec2 LayerRAMPrecision<T>::getValueAsVec2Double(const size2_t& pos) const {
    return util::glm_convert_normalized<dvec2>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec3 LayerRAMPrecision<T>::getValueAsVec3Double(const size2_t& pos) const {
    return util::glm_convert_normalized<dvec3>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec4 LayerRAMPrecision<T>::getValueAsVec4Double(const size2_t& pos) const {
    return util::glm_convert_normalized<dvec4>(data_[posToIndex(pos, dimensions_)]);
}

//#define DataFormatIdMacro(i) typedef LayerRAMPrecision<Data##i::type> LayerRAM_##i;
//#include <inviwo/core/util/formatsdefinefunc.h>

}  // namespace

#endif  // IVW_LAYERRAMPRECISION_H
