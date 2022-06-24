/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2021 Inviwo Foundation
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

#include <modules/discretedata/channels/datachannel.h>
#include <modules/discretedata/channels/bufferchannel.h>

namespace inviwo {
namespace discretedata {

template <typename T, ind N>
template <typename VecNT>
void DataChannel<T, N>::fill(VecNT& dest, ind index, ind numElements) const {
    static_assert(sizeof(VecNT) == sizeof(T) * N,
                  "Size and type do not agree with the vector type.");
    T* rawPtr = reinterpret_cast<T*>(&dest);
    this->fillRaw(rawPtr, index, numElements);
}

template <typename T, ind N>
template <typename VecNT>
void DataChannel<T, N>::operator()(VecNT& dest, ind index) const {
    fill(dest, index);
}

template <typename T, ind N>
template <typename VecNT>
DataChannel<T, N>::iterator<VecNT> DataChannel<T, N>::begin() {
    return iterator<VecNT>(this->newIterator(), 0);
}
template <typename T, ind N>
template <typename VecNT>
DataChannel<T, N>::iterator<VecNT> DataChannel<T, N>::end() {
    return iterator<VecNT>(this->newIterator(), this->size());
}

template <typename T, ind N>
template <typename VecNT>
DataChannel<T, N>::const_iterator<VecNT> DataChannel<T, N>::begin() const {
    return const_iterator<VecNT>(this, 0);
}

template <typename T, ind N>
template <typename VecNT>
DataChannel<T, N>::const_iterator<VecNT> DataChannel<T, N>::end() const {
    return const_iterator<VecNT>(this, this->size());
}

// /**
//  * \brief Get iterator range
//  * Templated iterator return type, only specified once.
//  * @tparam VecNT Return type of resulting iterators
//  */
// template <typename T, ind N>
// CDataChannel<T, N>::hannelRange<VecNT> DataChannel<T, N>::all() {
//     return ChannelRange<VecNT>(this);
// }

// /**
//  * \brief Get const iterator range
//  * Templated iterator return type, only specified once.
//  * @tparam VecNT Return type of resulting iterators
//  */
// template <typename VecNT>
// DataChannel<T, N>::ConstChannelRange<VecNT> all() const {
//     return ConstChannelRange<VecNT>(this);
// }

template <typename T, ind N>
DataChannel<T, N>::DataChannel(const std::string& name, GridPrimitive definedOn)
    : BaseChannel<T, N>(name, DataFormat<T>::id(), definedOn) {}

template <typename T, ind N>
template <typename VecNT>
void DataChannel<T, N>::getMin(VecNT& dest) const {
    static_assert(sizeof(VecNT) == sizeof(T) * N,
                  "Size and type do not agree with the vector type.");

    if (!validMinMax_) {
        computeMinMax();
    }

    T* rawVec = reinterpret_cast<T*>(&dest);
    for (ind i = 0; i < N; ++i) {
        rawVec[i] = min_[i];
    }
}

template <typename T, ind N>
template <typename VecNT>
void DataChannel<T, N>::getMax(VecNT& dest) const {
    static_assert(sizeof(VecNT) == sizeof(T) * N,
                  "Size and type do not agree with the vector type.");

    if (!validMinMax_) {
        computeMinMax();
    }

    T* rawVec = reinterpret_cast<T*>(&dest);
    for (ind i = 0; i < N; ++i) {
        rawVec[i] = max_[i];
    }
}

template <typename T, ind N>
template <typename VecNT>
void DataChannel<T, N>::getMinMax(VecNT& minDest, VecNT& maxDest) const {
    static_assert(sizeof(VecNT) == sizeof(T) * N,
                  "Size and type do not agree with the vector type.");
    getMin(minDest);
    getMax(maxDest);
}

template <typename T, ind N>
void DataChannel<T, N>::computeMinMax() const {
    using Vec = std::array<T, N>;

    Vec minT{0};
    Vec maxT{0};

    bool initialized = false;

    for (const Vec& val : this->all<Vec>()) {
        if (!this->isValid(val[0])) continue;
        if (!initialized) {
            minT = val;
            maxT = val;
            initialized = true;
            continue;
        }

        for (ind dim = 0; dim < N; ++dim) {
            minT[dim] = std::min(minT[dim], val[dim]);
            maxT[dim] = std::max(maxT[dim], val[dim]);
        }
    }

    for (ind dim = 0; dim < N; ++dim) {
        min_[dim] = static_cast<double>(minT[dim]);
        max_[dim] = static_cast<double>(maxT[dim]);
    }

    validMinMax_ = true;
}

template <typename T>
bool BaseTypedChannel<T>::isValid(T val) const {
    return !std::isnan(val) && val != invalidValue_;
}

template <typename T>
T BaseTypedChannel<T>::getInvalidValue() const {
    return invalidValue_;
}

template <typename T>
void BaseTypedChannel<T>::setInvalidValue(T val) {
    invalidValue_ = val;
}

template <typename T>
double BaseTypedChannel<T>::getInvalidValueDouble() const {
    return static_cast<double>(invalidValue_);
}

template <typename T>
void BaseTypedChannel<T>::setInvalidValueDouble(double val) {
    invalidValue_ = static_cast<T>(val);
}

template <typename T, ind N>
bool DataChannel<T, N>::isDataValid(ind index) const {
    std::array<T, N> data;
    this->fillRaw(data.data(), index);
    for (ind n =0; n < N; ++n)
        if(this->isValid(data[n])) return true;
    return false;
    // return this->isValid(data[0]);
}

template <typename T, ind N>
std::shared_ptr<Channel> DataChannel<T, N>::toBufferChannel() const {
    return std::dynamic_pointer_cast<Channel>(std::make_shared<BufferChannel<T, N>>(*this));
}

}  // namespace discretedata
}  // namespace inviwo
