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

#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

/**
 * \ingroup datastructures
 */
template <typename T>
class VolumeRAMPrecision : public VolumeRAM {
public:
    using type = T;

    explicit VolumeRAMPrecision(size3_t dimensions = size3_t(128, 128, 128),
                                const SwizzleMask& swizzleMask = swizzlemasks::rgba,
                                InterpolationType interpolation = InterpolationType::Linear,
                                const Wrapping3D& wrapping = wrapping3d::clampAll);
    VolumeRAMPrecision(T* data, size3_t dimensions,
                       const SwizzleMask& swizzleMask = swizzlemasks::rgba,
                       InterpolationType interpolation = InterpolationType::Linear,
                       const Wrapping3D& wrapping = wrapping3d::clampAll);
    VolumeRAMPrecision(const VolumeRAMPrecision<T>& rhs);
    VolumeRAMPrecision<T>& operator=(const VolumeRAMPrecision<T>& that);
    virtual VolumeRAMPrecision<T>* clone() const override;
    virtual ~VolumeRAMPrecision();

    T* getDataTyped();
    const T* getDataTyped() const;

    virtual void* getData() override;
    virtual const void* getData() const override;

    virtual void* getData(size_t) override;
    virtual const void* getData(size_t) const override;

    virtual void setData(void* data, size3_t dimensions) override;

    virtual void removeDataOwnership() override;

    virtual const size3_t& getDimensions() const override;
    virtual void setDimensions(size3_t dimensions) override;

    /**
     * \brief update the swizzle mask of the color channels when sampling the volume
     *
     * @param mask new swizzle mask
     */
    virtual void setSwizzleMask(const SwizzleMask& mask) override;
    virtual SwizzleMask getSwizzleMask() const override;

    virtual void setInterpolation(InterpolationType interpolation) override;
    virtual InterpolationType getInterpolation() const override;

    virtual void setWrapping(const Wrapping3D& wrapping) override;
    virtual Wrapping3D getWrapping() const override;

    virtual double getAsDouble(const size3_t& pos) const override;
    virtual dvec2 getAsDVec2(const size3_t& pos) const override;
    virtual dvec3 getAsDVec3(const size3_t& pos) const override;
    virtual dvec4 getAsDVec4(const size3_t& pos) const override;

    virtual void setFromDouble(const size3_t& pos, double val) override;
    virtual void setFromDVec2(const size3_t& pos, dvec2 val) override;
    virtual void setFromDVec3(const size3_t& pos, dvec3 val) override;
    virtual void setFromDVec4(const size3_t& pos, dvec4 val) override;

    virtual double getAsNormalizedDouble(const size3_t& pos) const override;
    virtual dvec2 getAsNormalizedDVec2(const size3_t& pos) const override;
    virtual dvec3 getAsNormalizedDVec3(const size3_t& pos) const override;
    virtual dvec4 getAsNormalizedDVec4(const size3_t& pos) const override;

    virtual void setFromNormalizedDouble(const size3_t& pos, double val) override;
    virtual void setFromNormalizedDVec2(const size3_t& pos, dvec2 val) override;
    virtual void setFromNormalizedDVec3(const size3_t& pos, dvec3 val) override;
    virtual void setFromNormalizedDVec4(const size3_t& pos, dvec4 val) override;

    virtual size_t getNumberOfBytes() const override;

private:
    size3_t dimensions_;
    bool ownsDataPtr_;
    std::unique_ptr<T[]> data_;
    SwizzleMask swizzleMask_;
    InterpolationType interpolation_;
    Wrapping3D wrapping_;
};

/**
 * Factory for volumes.
 * Creates an VolumeRAM with data type specified by format.
 *
 * @param dimensions of volume to create.
 * @param format of volume to create.
 * @param dataPtr optional pointer to data to be handed into the volume.
 * @return nullptr if no valid format was specified.
 */
IVW_CORE_API std::shared_ptr<VolumeRAM> createVolumeRAM(
    const size3_t& dimensions, const DataFormatBase* format, void* dataPtr = nullptr,
    const SwizzleMask& swizzleMask = swizzlemasks::rgba,
    InterpolationType interpolation = InterpolationType::Linear,
    const Wrapping3D& wrapping = wrapping3d::clampAll);

template <typename T>
VolumeRAMPrecision<T>::VolumeRAMPrecision(size3_t dimensions, const SwizzleMask& swizzleMask,
                                          InterpolationType interpolation,
                                          const Wrapping3D& wrapping)
    : VolumeRAM(DataFormat<T>::get())
    , dimensions_(dimensions)
    , ownsDataPtr_(true)
    , data_(new T[dimensions_.x * dimensions_.y * dimensions_.z]())
    , swizzleMask_(swizzleMask)
    , interpolation_{interpolation}
    , wrapping_{wrapping} {}

template <typename T>
VolumeRAMPrecision<T>::VolumeRAMPrecision(T* data, size3_t dimensions,
                                          const SwizzleMask& swizzleMask,
                                          InterpolationType interpolation,
                                          const Wrapping3D& wrapping)
    : VolumeRAM(DataFormat<T>::get())
    , dimensions_(dimensions)
    , ownsDataPtr_(true)
    , data_(data ? data : new T[dimensions_.x * dimensions_.y * dimensions_.z]())
    , swizzleMask_(swizzleMask)
    , interpolation_{interpolation}
    , wrapping_{wrapping} {}

template <typename T>
VolumeRAMPrecision<T>::VolumeRAMPrecision(const VolumeRAMPrecision<T>& rhs)
    : VolumeRAM(rhs)
    , dimensions_(rhs.dimensions_)
    , ownsDataPtr_(true)
    , data_(new T[dimensions_.x * dimensions_.y * dimensions_.z])
    , swizzleMask_(rhs.swizzleMask_)
    , interpolation_{rhs.interpolation_}
    , wrapping_{rhs.wrapping_} {
    std::memcpy(data_.get(), rhs.data_.get(),
                dimensions_.x * dimensions_.y * dimensions_.z * sizeof(T));
}

template <typename T>
VolumeRAMPrecision<T>& VolumeRAMPrecision<T>::operator=(const VolumeRAMPrecision<T>& that) {
    if (this != &that) {
        VolumeRAM::operator=(that);
        auto dim = that.dimensions_;
        auto data = std::make_unique<T[]>(dim.x * dim.y * dim.z);
        std::memcpy(data.get(), that.data_.get(), dim.x * dim.y * dim.z * sizeof(T));
        data_.swap(data);
        std::swap(dim, dimensions_);
        ownsDataPtr_ = true;
        swizzleMask_ = that.swizzleMask_;
        interpolation_ = that.interpolation_;
        wrapping_ = that.wrapping_;
    }
    return *this;
}

template <typename T>
VolumeRAMPrecision<T>::~VolumeRAMPrecision() {
    if (!ownsDataPtr_) data_.release();
}

template <typename T>
VolumeRAMPrecision<T>* VolumeRAMPrecision<T>::clone() const {
    return new VolumeRAMPrecision<T>(*this);
}

template <typename T>
const T* inviwo::VolumeRAMPrecision<T>::getDataTyped() const {
    return data_.get();
}

template <typename T>
T* inviwo::VolumeRAMPrecision<T>::getDataTyped() {
    return data_.get();
}

template <typename T>
void* VolumeRAMPrecision<T>::getData() {
    return data_.get();
}
template <typename T>
const void* VolumeRAMPrecision<T>::getData() const {
    return const_cast<const T*>(data_.get());
}

template <typename T>
void* VolumeRAMPrecision<T>::getData(size_t pos) {
    return data_.get() + pos;
}

template <typename T>
const void* VolumeRAMPrecision<T>::getData(size_t pos) const {
    return const_cast<const T*>(data_.get()) + pos;
}

template <typename T>
void VolumeRAMPrecision<T>::setData(void* d, size3_t dimensions) {
    std::unique_ptr<T[]> data(static_cast<T*>(d));
    data_.swap(data);
    std::swap(dimensions_, dimensions);

    if (!ownsDataPtr_) data.release();
    ownsDataPtr_ = true;
}

template <typename T>
void VolumeRAMPrecision<T>::removeDataOwnership() {
    ownsDataPtr_ = false;
}

template <typename T>
const size3_t& VolumeRAMPrecision<T>::getDimensions() const {
    return dimensions_;
}

template <typename T>
size_t VolumeRAMPrecision<T>::getNumberOfBytes() const {
    return dimensions_.x * dimensions_.y * dimensions_.z * sizeof(T);
}

template <typename T>
void VolumeRAMPrecision<T>::setDimensions(size3_t dimensions) {
    if (dimensions_ != dimensions) {
        auto data = std::make_unique<T[]>(dimensions.x * dimensions.y * dimensions.z);
        data_.swap(data);
        dimensions_ = dimensions;
        if (!ownsDataPtr_) data.release();
        ownsDataPtr_ = true;
    }
}

template <typename T>
void VolumeRAMPrecision<T>::setSwizzleMask(const SwizzleMask& mask) {
    swizzleMask_ = mask;
}

template <typename T>
SwizzleMask VolumeRAMPrecision<T>::getSwizzleMask() const {
    return swizzleMask_;
}

template <typename T>
void VolumeRAMPrecision<T>::setInterpolation(InterpolationType interpolation) {
    interpolation_ = interpolation;
}

template <typename T>
InterpolationType VolumeRAMPrecision<T>::getInterpolation() const {
    return interpolation_;
}

template <typename T>
void VolumeRAMPrecision<T>::setWrapping(const Wrapping3D& wrapping) {
    wrapping_ = wrapping;
}

template <typename T>
Wrapping3D VolumeRAMPrecision<T>::getWrapping() const {
    return wrapping_;
}

template <typename T>
double VolumeRAMPrecision<T>::getAsDouble(const size3_t& pos) const {
    return util::glm_convert<double>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec2 VolumeRAMPrecision<T>::getAsDVec2(const size3_t& pos) const {
    return util::glm_convert<dvec2>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec3 VolumeRAMPrecision<T>::getAsDVec3(const size3_t& pos) const {
    return util::glm_convert<dvec3>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec4 VolumeRAMPrecision<T>::getAsDVec4(const size3_t& pos) const {
    return util::glm_convert<dvec4>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
void VolumeRAMPrecision<T>::setFromDouble(const size3_t& pos, double val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
void VolumeRAMPrecision<T>::setFromDVec2(const size3_t& pos, dvec2 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
void VolumeRAMPrecision<T>::setFromDVec3(const size3_t& pos, dvec3 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
void VolumeRAMPrecision<T>::setFromDVec4(const size3_t& pos, dvec4 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
double VolumeRAMPrecision<T>::getAsNormalizedDouble(const size3_t& pos) const {
    return util::glm_convert_normalized<double>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec2 VolumeRAMPrecision<T>::getAsNormalizedDVec2(const size3_t& pos) const {
    return util::glm_convert_normalized<dvec2>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec3 VolumeRAMPrecision<T>::getAsNormalizedDVec3(const size3_t& pos) const {
    return util::glm_convert_normalized<dvec3>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec4 VolumeRAMPrecision<T>::getAsNormalizedDVec4(const size3_t& pos) const {
    return util::glm_convert_normalized<dvec4>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
void VolumeRAMPrecision<T>::setFromNormalizedDouble(const size3_t& pos, double val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert_normalized<T>(val);
}

template <typename T>
void VolumeRAMPrecision<T>::setFromNormalizedDVec2(const size3_t& pos, dvec2 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert_normalized<T>(val);
}

template <typename T>
void VolumeRAMPrecision<T>::setFromNormalizedDVec3(const size3_t& pos, dvec3 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert_normalized<T>(val);
}

template <typename T>
void VolumeRAMPrecision<T>::setFromNormalizedDVec4(const size3_t& pos, dvec4 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert_normalized<T>(val);
}

}  // namespace inviwo
