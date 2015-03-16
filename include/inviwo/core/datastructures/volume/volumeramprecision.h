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
#include <inviwo/core/datastructures/volume/volumeramoperationexecuter.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

template <typename T>
class VolumeRAMPrecision : public VolumeRAM {
public:
    VolumeRAMPrecision(uvec3 dimensions = uvec3(128, 128, 128),
                       const DataFormatBase* format = DataFormat<T>::get());
    VolumeRAMPrecision(T* data, uvec3 dimensions = uvec3(128, 128, 128),
                       const DataFormatBase* format = DataFormat<T>::get());
    VolumeRAMPrecision(const VolumeRAMPrecision<T>& rhs);
    VolumeRAMPrecision<T>& operator=(const VolumeRAMPrecision<T>& that);
    virtual VolumeRAMPrecision<T>* clone() const override;
    virtual ~VolumeRAMPrecision();

    virtual void performOperation(DataOperation* dop) const override;

    virtual void* getData() override;
    virtual const void* getData() const override;

    virtual void* getData(size_t) override;
    virtual const void* getData(size_t) const override;

    virtual void setData(void* data) override;

    virtual void removeDataOwnership() override;

    virtual const uvec3& getDimensions() const override;
    virtual void setDimensions(uvec3 dimensions) override;

    virtual bool hasHistograms() const override;
    virtual HistogramContainer* getHistograms(size_t bins = 2048u,
                                              uvec3 sampleRate = uvec3(1)) override;
    virtual const HistogramContainer* getHistograms(size_t bins = 2048u,
                                                    uvec3 sampleRate = uvec3(1)) const override;
    virtual void calculateHistograms(size_t bins, uvec3 sampleRate, const bool& stop) const override;

    virtual void setValueFromSingleDouble(const uvec3& pos, double val) override;
    virtual void setValueFromVec2Double(const uvec3& pos, dvec2 val) override;
    virtual void setValueFromVec3Double(const uvec3& pos, dvec3 val) override;
    virtual void setValueFromVec4Double(const uvec3& pos, dvec4 val) override;

    void setValuesFromVolume(const VolumeRAM* src, const uvec3& dstOffset, const uvec3& subSize,
                             const uvec3& subOffset);

    virtual double getValueAsSingleDouble(const uvec3& pos) const override;
    virtual dvec2 getValueAsVec2Double(const uvec3& pos) const override;
    virtual dvec3 getValueAsVec3Double(const uvec3& pos) const override;
    virtual dvec4 getValueAsVec4Double(const uvec3& pos) const override;

    virtual size_t getNumberOfBytes() const override;

private:
    uvec3 dimensions_;
    bool ownsDataPtr_;
    std::unique_ptr<T[]> data_;
    mutable HistogramContainer histCont_;
};

template <typename T>
VolumeRAMPrecision<T>::VolumeRAMPrecision(uvec3 dimensions, const DataFormatBase* format)
    : VolumeRAM(format)
    , dimensions_(dimensions)
    , ownsDataPtr_(true)
    , data_(new T[dimensions_.x * dimensions_.y * dimensions_.z]) {}

template <typename T>
VolumeRAMPrecision<T>::VolumeRAMPrecision(T* data, uvec3 dimensions, const DataFormatBase* format)
    : VolumeRAM(format)
    , dimensions_(dimensions)
    , ownsDataPtr_(true)
    , data_(data ? data : new T[dimensions_.x * dimensions_.y * dimensions_.z]) {}

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
        dimensions_ = that.dimensions_;
        auto data = util::make_unique<T[]>(dimensions_.x * dimensions_.y * dimensions_.z);
        std::memcpy(data.get(), that.data_.get(),
                    dimensions_.x * dimensions_.y * dimensions_.z * sizeof(T));
        data_.swap(data);
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
void VolumeRAMPrecision<T>::performOperation(DataOperation* dop) const {
    executeOperationOnVolumeRAMPrecision<T>(dop);
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
void inviwo::VolumeRAMPrecision<T>::setData(void* d) {
    std::unique_ptr<T[]> data(static_cast<T*>(d));
    data_.swap(data);

    if (!ownsDataPtr_) data.release();
    ownsDataPtr_ = true;
}

template <typename T>
void inviwo::VolumeRAMPrecision<T>::removeDataOwnership() {
    ownsDataPtr_ = false;
}

template <typename T>
const uvec3& inviwo::VolumeRAMPrecision<T>::getDimensions() const {
    return dimensions_;
}

template <typename T>
size_t VolumeRAMPrecision<T>::getNumberOfBytes() const {
    return dimensions_.x * dimensions_.y * dimensions_.z * sizeof(T);
}

template <typename T>
void VolumeRAMPrecision<T>::setDimensions(uvec3 dimensions) {
    auto data = util::make_unique<T[]>(dimensions.x * dimensions.y * dimensions.z);
    data_.swap(data);
    dimensions_ = dimensions;
    if (!ownsDataPtr_) data.release();
    ownsDataPtr_ = true;
}

template <typename T>
void VolumeRAMPrecision<T>::setValueFromSingleDouble(const uvec3& pos, double val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
void VolumeRAMPrecision<T>::setValueFromVec2Double(const uvec3& pos, dvec2 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
void VolumeRAMPrecision<T>::setValueFromVec3Double(const uvec3& pos, dvec3 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
void VolumeRAMPrecision<T>::setValueFromVec4Double(const uvec3& pos, dvec4 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
void VolumeRAMPrecision<T>::setValuesFromVolume(const VolumeRAM* src, const uvec3& dstOffset,
                                                const uvec3& subSize, const uvec3& subOffset) {
    const T* srcData = reinterpret_cast<const T*>(src->getData());

    size_t initialStartPos = (dstOffset.z * (dimensions_.x * dimensions_.y)) +
                             (dstOffset.y * dimensions_.x) + dstOffset.x;

    uvec3 srcDims = src->getDimensions();
    size_t dataSize = subSize.x * getDataFormat()->getSize();

    size_t volumePos;
    size_t subVolumePos;
    for (size_t i = 0; i < subSize.z; i++) {
        for (size_t j = 0; j < subSize.y; j++) {
            volumePos = (j * dimensions_.x) + (i * dimensions_.x * dimensions_.y);
            subVolumePos = ((j + subOffset.y) * srcDims.x) +
                           ((i + subOffset.z) * srcDims.x * srcDims.y) + subOffset.x;
            memcpy((data_.get() + volumePos + initialStartPos), (srcData + subVolumePos), dataSize);
        }
    }
}

template <typename T>
double VolumeRAMPrecision<T>::getValueAsSingleDouble(const uvec3& pos) const {
    return util::glm_convert_normalized<double>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec2 VolumeRAMPrecision<T>::getValueAsVec2Double(const uvec3& pos) const {
    return util::glm_convert_normalized<dvec2>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec3 VolumeRAMPrecision<T>::getValueAsVec3Double(const uvec3& pos) const {
    return util::glm_convert_normalized<dvec3>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec4 VolumeRAMPrecision<T>::getValueAsVec4Double(const uvec3& pos) const {
    return util::glm_convert_normalized<dvec4>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
const HistogramContainer* inviwo::VolumeRAMPrecision<T>::getHistograms(size_t bins,
                                                                       uvec3 sampleRate) const {
    if (!hasHistograms()) {
        bool stop = false;
        calculateHistograms(bins, sampleRate, stop);
    }

    return &histCont_;
}

template <typename T>
HistogramContainer* inviwo::VolumeRAMPrecision<T>::getHistograms(size_t bins,
                                                                 uvec3 sampleRate) {
    if (!hasHistograms()) {
        bool stop = false;
        calculateHistograms(bins, sampleRate, stop);
    }

    return &histCont_;

}

template <typename T>
void inviwo::VolumeRAMPrecision<T>::calculateHistograms(size_t bins, uvec3 sampleRate,
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

#define DataFormatIdMacro(i) typedef VolumeRAMPrecision<Data##i::type> VolumeRAM_##i;
#include <inviwo/core/util/formatsdefinefunc.h>

}  // namespace

#endif  // IVW_VOLUMERAMPRECISION_H
