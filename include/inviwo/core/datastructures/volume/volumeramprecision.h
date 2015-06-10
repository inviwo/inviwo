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

#ifndef IVW_VOLUMERAMPRECISION_H
#define IVW_VOLUMERAMPRECISION_H

#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramhistogram.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

template <typename T>
class VolumeRAMPrecision : public VolumeRAM {
public:
    VolumeRAMPrecision(size3_t dimensions = size3_t(128, 128, 128),
                       const DataFormatBase* format = DataFormat<T>::get());
    VolumeRAMPrecision(T* data, size3_t dimensions = size3_t(128, 128, 128),
                       const DataFormatBase* format = DataFormat<T>::get());
    VolumeRAMPrecision(const VolumeRAMPrecision<T>& rhs);
    VolumeRAMPrecision<T>& operator=(const VolumeRAMPrecision<T>& that);
    virtual VolumeRAMPrecision<T>* clone() const override;
    virtual ~VolumeRAMPrecision();

    virtual void* getData() override;
    virtual const void* getData() const override;

    virtual void* getData(size_t) override;
    virtual const void* getData(size_t) const override;

    virtual void setData(void* data, size3_t dimensions) override;

    virtual void removeDataOwnership() override;

    virtual const size3_t& getDimensions() const override;
    virtual void setDimensions(size3_t dimensions) override;

    virtual bool hasHistograms() const override;
    virtual HistogramContainer* getHistograms(size_t bins = 2048u,
                                              size3_t sampleRate = size3_t(1)) override;
    virtual const HistogramContainer* getHistograms(size_t bins = 2048u,
                                                    size3_t sampleRate = size3_t(1)) const override;
    virtual void calculateHistograms(size_t bins, size3_t sampleRate, const bool& stop) const override;

    virtual void setValueFromSingleDouble(const size3_t& pos, double val) override;
    virtual void setValueFromVec2Double(const size3_t& pos, dvec2 val) override;
    virtual void setValueFromVec3Double(const size3_t& pos, dvec3 val) override;
    virtual void setValueFromVec4Double(const size3_t& pos, dvec4 val) override;

    void setValuesFromVolume(const VolumeRAM* src, const size3_t& dstOffset, const size3_t& subSize,
                             const size3_t& subOffset);

    virtual double getValueAsSingleDouble(const size3_t& pos) const override;
    virtual dvec2 getValueAsVec2Double(const size3_t& pos) const override;
    virtual dvec3 getValueAsVec3Double(const size3_t& pos) const override;
    virtual dvec4 getValueAsVec4Double(const size3_t& pos) const override;

    virtual size_t getNumberOfBytes() const override;

private:
    size3_t dimensions_;
    bool ownsDataPtr_;
    std::unique_ptr<T[]> data_;
    mutable HistogramContainer histCont_;
};

/**
 * Factory for volumes.
 * Creates an VolumeRAM with data type specified by format.
 *
 * @param dimensions of volume to create.
 * @param format of volume to create.
 * @return nullptr if no valid format was specified.
 */
IVW_CORE_API VolumeRAM* createVolumeRAM(const size3_t& dimensions, const DataFormatBase* format,
                                        void* dataPtr = nullptr);

struct VolumeRamDispatcher {
    using type = VolumeRAM*;
    template <class T>
    VolumeRAM* dispatch(void* dataPtr, const size3_t& dimensions) {
        typedef typename T::type F;
        return new VolumeRAMPrecision<F>(static_cast<F*>(dataPtr), dimensions);
    }
};


template <typename T>
VolumeRAMPrecision<T>::VolumeRAMPrecision(size3_t dimensions, const DataFormatBase* format)
    : VolumeRAM(format)
    , dimensions_(dimensions)
    , ownsDataPtr_(true)
    , data_(new T[dimensions_.x * dimensions_.y * dimensions_.z]()) {}

template <typename T>
VolumeRAMPrecision<T>::VolumeRAMPrecision(T* data, size3_t dimensions, const DataFormatBase* format)
    : VolumeRAM(format)
    , dimensions_(dimensions)
    , ownsDataPtr_(true)
    , data_(data ? data : new T[dimensions_.x * dimensions_.y * dimensions_.z]()) {}

template <typename T>
VolumeRAMPrecision<T>::VolumeRAMPrecision(const VolumeRAMPrecision<T>& rhs)
    : VolumeRAM(rhs)
    , dimensions_(rhs.dimensions_)
    , ownsDataPtr_(true)
    , data_(new T[dimensions_.x * dimensions_.y * dimensions_.z]) {
    std::memcpy(data_.get(), rhs.data_.get(),
                dimensions_.x * dimensions_.y * dimensions_.z * sizeof(T));
}

template <typename T>
VolumeRAMPrecision<T>& VolumeRAMPrecision<T>::operator=(const VolumeRAMPrecision<T>& that) {
    if (this != &that) {
        VolumeRAM::operator=(that);
        auto dim = that.dimensions_;
        auto data = util::make_unique<T[]>(dim.x * dim.y * dim.z);
        std::memcpy(data.get(), that.data_.get(), dim.x * dim.y * dim.z * sizeof(T));
        data_.swap(data);
        std::swap(dim, dimensions_);
        ownsDataPtr_ = true;
    }
    return *this;
};

template <typename T>
VolumeRAMPrecision<T>::~VolumeRAMPrecision() {
    if (!ownsDataPtr_) data_.release();
};

template <typename T>
VolumeRAMPrecision<T>* VolumeRAMPrecision<T>::clone() const {
    return new VolumeRAMPrecision<T>(*this);
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
void inviwo::VolumeRAMPrecision<T>::setData(void* d, size3_t dimensions) {
    std::unique_ptr<T[]> data(static_cast<T*>(d));
    data_.swap(data);
    std::swap(dimensions_, dimensions);

    if (!ownsDataPtr_) data.release();
    ownsDataPtr_ = true;
}

template <typename T>
void inviwo::VolumeRAMPrecision<T>::removeDataOwnership() {
    ownsDataPtr_ = false;
}

template <typename T>
const size3_t& inviwo::VolumeRAMPrecision<T>::getDimensions() const {
    return dimensions_;
}

template <typename T>
size_t VolumeRAMPrecision<T>::getNumberOfBytes() const {
    return dimensions_.x * dimensions_.y * dimensions_.z * sizeof(T);
}

template <typename T>
void VolumeRAMPrecision<T>::setDimensions(size3_t dimensions) {
    auto data = util::make_unique<T[]>(dimensions.x * dimensions.y * dimensions.z);
    data_.swap(data);
    dimensions_ = dimensions;
    if (!ownsDataPtr_) data.release();
    ownsDataPtr_ = true;
}

template <typename T>
void VolumeRAMPrecision<T>::setValueFromSingleDouble(const size3_t& pos, double val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
void VolumeRAMPrecision<T>::setValueFromVec2Double(const size3_t& pos, dvec2 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
void VolumeRAMPrecision<T>::setValueFromVec3Double(const size3_t& pos, dvec3 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
void VolumeRAMPrecision<T>::setValueFromVec4Double(const size3_t& pos, dvec4 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
void VolumeRAMPrecision<T>::setValuesFromVolume(const VolumeRAM* src, const size3_t& dstOffset,
                                                const size3_t& subSize, const size3_t& subOffset) {
    const T* srcData = reinterpret_cast<const T*>(src->getData());

    size_t initialStartPos = (dstOffset.z * (dimensions_.x * dimensions_.y)) +
                             (dstOffset.y * dimensions_.x) + dstOffset.x;

    size3_t srcDims = src->getDimensions();
    size_t dataSize = subSize.x * getDataFormat()->getSize();

    size_t volumePos;
    size_t subVolumePos;
    ivec3 subSizeI = ivec3(subSize);
#pragma omp parallel for
    for (int zy = 0; zy < subSizeI.z*subSizeI.y; ++zy) {
        int z = zy / subSizeI.y;
        int y = zy % subSizeI.y;
        volumePos = (y * dimensions_.x) + (z * dimensions_.x * dimensions_.y);
        subVolumePos = ((y + subOffset.y) * srcDims.x) +
            ((z + subOffset.z) * srcDims.x * srcDims.y) + subOffset.x;
        std::memcpy((data_.get() + volumePos + initialStartPos), (srcData + subVolumePos), dataSize);
    }
}

template <typename T>
double VolumeRAMPrecision<T>::getValueAsSingleDouble(const size3_t& pos) const {
    return util::glm_convert_normalized<double>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec2 VolumeRAMPrecision<T>::getValueAsVec2Double(const size3_t& pos) const {
    return util::glm_convert_normalized<dvec2>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec3 VolumeRAMPrecision<T>::getValueAsVec3Double(const size3_t& pos) const {
    return util::glm_convert_normalized<dvec3>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec4 VolumeRAMPrecision<T>::getValueAsVec4Double(const size3_t& pos) const {
    return util::glm_convert_normalized<dvec4>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
const HistogramContainer* inviwo::VolumeRAMPrecision<T>::getHistograms(size_t bins,
                                                                       size3_t sampleRate) const {
    if (!hasHistograms()) {
        bool stop = false;
        calculateHistograms(bins, sampleRate, stop);
    }

    return &histCont_;
}

template <typename T>
HistogramContainer* inviwo::VolumeRAMPrecision<T>::getHistograms(size_t bins,
                                                                 size3_t sampleRate) {
    if (!hasHistograms()) {
        bool stop = false;
        calculateHistograms(bins, sampleRate, stop);
    }

    return &histCont_;

}

template <typename T>
void inviwo::VolumeRAMPrecision<T>::calculateHistograms(size_t bins, size3_t sampleRate,
                                                        const bool& stop) const {
    const Volume* volume = dynamic_cast<const Volume*>(getOwner());
    if (volume) {
        dvec2 dataRange = volume->dataMap_.dataRange;
        histCont_ = util::calculateVolumeHistogram(data_.get(), dimensions_, dataRange, stop, bins,
                                                   sampleRate);
    }
}

template <typename T>
bool inviwo::VolumeRAMPrecision<T>::hasHistograms() const {
    return !histCont_.empty() && histCont_.isValid();
}

}  // namespace

#endif  // IVW_VOLUMERAMPRECISION_H
