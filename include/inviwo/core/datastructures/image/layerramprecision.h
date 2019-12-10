/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <algorithm>

namespace inviwo {

/**
 * \ingroup datastructures
 */
template <typename T>
class LayerRAMPrecision : public LayerRAM {
public:
    using type = T;

    explicit LayerRAMPrecision(size2_t dimensions = size2_t(8, 8),
                               LayerType type = LayerType::Color,
                               const SwizzleMask& swizzleMask = swizzlemasks::rgba);
    LayerRAMPrecision(T* data, size2_t dimensions, LayerType type = LayerType::Color,
                      const SwizzleMask& swizzleMask = swizzlemasks::rgba);
    LayerRAMPrecision(const LayerRAMPrecision<T>& rhs);
    LayerRAMPrecision<T>& operator=(const LayerRAMPrecision<T>& that);
    virtual LayerRAMPrecision<T>* clone() const override;
    virtual ~LayerRAMPrecision() = default;

    T* getDataTyped();
    const T* getDataTyped() const;

    virtual void* getData() override;
    virtual const void* getData() const override;
    virtual void setData(void* data, size2_t dimensions) override;

    /**
     * Resize the representation to dimension. This is destructive, the data will not be
     * preserved. Use copyRepresentationsTo to update the data.
     */
    virtual void setDimensions(size2_t dimensions) override;
    const size2_t& getDimensions() const override;

    /**
     * \brief update the swizzle mask of the channels for sampling the layer
     * Needs to be overloaded by child classes.
     *
     * @param mask    new swizzle mask
     */
    virtual void setSwizzleMask(const SwizzleMask& mask) override;
    virtual SwizzleMask getSwizzleMask() const override;

    virtual double getAsDouble(const size2_t& pos) const override;
    virtual dvec2 getAsDVec2(const size2_t& pos) const override;
    virtual dvec3 getAsDVec3(const size2_t& pos) const override;
    virtual dvec4 getAsDVec4(const size2_t& pos) const override;

    virtual void setFromDouble(const size2_t& pos, double val) override;
    virtual void setFromDVec2(const size2_t& pos, dvec2 val) override;
    virtual void setFromDVec3(const size2_t& pos, dvec3 val) override;
    virtual void setFromDVec4(const size2_t& pos, dvec4 val) override;

    virtual double getAsNormalizedDouble(const size2_t& pos) const override;
    virtual dvec2 getAsNormalizedDVec2(const size2_t& pos) const override;
    virtual dvec3 getAsNormalizedDVec3(const size2_t& pos) const override;
    virtual dvec4 getAsNormalizedDVec4(const size2_t& pos) const override;

    virtual void setFromNormalizedDouble(const size2_t& pos, double val) override;
    virtual void setFromNormalizedDVec2(const size2_t& pos, dvec2 val) override;
    virtual void setFromNormalizedDVec3(const size2_t& pos, dvec3 val) override;
    virtual void setFromNormalizedDVec4(const size2_t& pos, dvec4 val) override;

private:
    size2_t dimensions_;
    std::unique_ptr<T[]> data_;
    SwizzleMask swizzleMask_;
};

/**
 * Factory for layers.
 * Creates an LayerRAM with data type specified by format.
 *
 * @param dimensions of layer to create.
 * @param type of layer to create.
 * @param format of layer to create.
 * @param swizzleMask used in for the layer, defaults to RGB-alpha
 * @return nullptr if no valid format was specified.
 */
IVW_CORE_API std::shared_ptr<LayerRAM> createLayerRAM(
    const size2_t& dimensions, LayerType type, const DataFormatBase* format,
    const SwizzleMask& swizzleMask = swizzlemasks::rgba);

template <typename T>
LayerRAMPrecision<T>::LayerRAMPrecision(size2_t dimensions, LayerType type,
                                        const SwizzleMask& swizzleMask)
    : LayerRAM(type, DataFormat<T>::get())
    , dimensions_(dimensions)
    , data_(new T[dimensions_.x * dimensions_.y]())
    , swizzleMask_(swizzleMask) {
    std::fill(data_.get(), data_.get() + glm::compMul(dimensions_),
              (type == LayerType::Depth) ? T{1} : T{0});
}

template <typename T>
LayerRAMPrecision<T>::LayerRAMPrecision(T* data, size2_t dimensions, LayerType type,
                                        const SwizzleMask& swizzleMask)
    : LayerRAM(type, DataFormat<T>::get())
    , dimensions_(dimensions)
    , data_(data ? data : new T[dimensions_.x * dimensions_.y]())
    , swizzleMask_(swizzleMask) {
    if (!data) {
        std::fill(data_.get(), data_.get() + glm::compMul(dimensions_),
                  (type == LayerType::Depth) ? T{1} : T{0});
    }
}

template <typename T>
LayerRAMPrecision<T>::LayerRAMPrecision(const LayerRAMPrecision<T>& rhs)
    : LayerRAM(rhs)
    , dimensions_(rhs.dimensions_)
    , data_(new T[dimensions_.x * dimensions_.y])
    , swizzleMask_(rhs.swizzleMask_) {
    std::memcpy(data_.get(), rhs.data_.get(), dimensions_.x * dimensions_.y * sizeof(T));
}

template <typename T>
LayerRAMPrecision<T>& LayerRAMPrecision<T>::operator=(const LayerRAMPrecision<T>& that) {
    if (this != &that) {
        LayerRAM::operator=(that);

        const auto dim = that.dimensions_;
        auto data = std::make_unique<T[]>(dim.x * dim.y);
        std::memcpy(data.get(), that.data_.get(), dim.x * dim.y * sizeof(T));
        data_.swap(data);

        dimensions_ = that.dimensions_;
        swizzleMask_ = that.swizzleMask_;
    }
    return *this;
}

template <typename T>
LayerRAMPrecision<T>* LayerRAMPrecision<T>::clone() const {
    return new LayerRAMPrecision<T>(*this);
}

template <typename T>
T* inviwo::LayerRAMPrecision<T>::getDataTyped() {
    return data_.get();
}

template <typename T>
const T* inviwo::LayerRAMPrecision<T>::getDataTyped() const {
    return data_.get();
}

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
    if (dimensions != dimensions_) {
        auto data = std::make_unique<T[]>(dimensions.x * dimensions.y);
        data_.swap(data);
        std::swap(dimensions, dimensions_);
    }
}

template <typename T>
const size2_t& LayerRAMPrecision<T>::getDimensions() const {
    return dimensions_;
}

template <typename T>
void LayerRAMPrecision<T>::setSwizzleMask(const SwizzleMask& mask) {
    swizzleMask_ = mask;
}

template <typename T>
SwizzleMask LayerRAMPrecision<T>::getSwizzleMask() const {
    return swizzleMask_;
}

template <typename T>
double LayerRAMPrecision<T>::getAsDouble(const size2_t& pos) const {
    return util::glm_convert<double>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec2 LayerRAMPrecision<T>::getAsDVec2(const size2_t& pos) const {
    return util::glm_convert<dvec2>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec3 LayerRAMPrecision<T>::getAsDVec3(const size2_t& pos) const {
    return util::glm_convert<dvec3>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec4 LayerRAMPrecision<T>::getAsDVec4(const size2_t& pos) const {
    return util::glm_convert<dvec4>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
void LayerRAMPrecision<T>::setFromDouble(const size2_t& pos, double val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
void LayerRAMPrecision<T>::setFromDVec2(const size2_t& pos, dvec2 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
void LayerRAMPrecision<T>::setFromDVec3(const size2_t& pos, dvec3 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
void LayerRAMPrecision<T>::setFromDVec4(const size2_t& pos, dvec4 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
double LayerRAMPrecision<T>::getAsNormalizedDouble(const size2_t& pos) const {
    return util::glm_convert_normalized<double>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec2 LayerRAMPrecision<T>::getAsNormalizedDVec2(const size2_t& pos) const {
    return util::glm_convert_normalized<dvec2>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec3 LayerRAMPrecision<T>::getAsNormalizedDVec3(const size2_t& pos) const {
    return util::glm_convert_normalized<dvec3>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec4 LayerRAMPrecision<T>::getAsNormalizedDVec4(const size2_t& pos) const {
    return util::glm_convert_normalized<dvec4>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
void LayerRAMPrecision<T>::setFromNormalizedDouble(const size2_t& pos, double val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert_normalized<T>(val);
}

template <typename T>
void LayerRAMPrecision<T>::setFromNormalizedDVec2(const size2_t& pos, dvec2 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert_normalized<T>(val);
}

template <typename T>
void LayerRAMPrecision<T>::setFromNormalizedDVec3(const size2_t& pos, dvec3 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert_normalized<T>(val);
}

template <typename T>
void LayerRAMPrecision<T>::setFromNormalizedDVec4(const size2_t& pos, dvec4 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert_normalized<T>(val);
}

}  // namespace inviwo

#endif  // IVW_LAYERRAMPRECISION_H
