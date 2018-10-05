/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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

#include <modules/discretedata/discretedatamoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/metadata/metadataowner.h>
#include <inviwo/core/metadata/metadata.h>

namespace inviwo {
namespace discretedata {

template <typename T, ind N>
DataChannel<T, N>::DataChannel(const std::string& name, GridPrimitive definedOn)
    : BaseChannel(name, DataFormatId::NotSpecialized, definedOn) {
    // Switch all types.
    if (std::is_same<T, f16>::value) this->setDataFormatId(DataFormatId::Float16);
    if (std::is_same<T, glm::f32>::value) this->setDataFormatId(DataFormatId::Float32);
    if (std::is_same<T, glm::f64>::value) this->setDataFormatId(DataFormatId::Float64);
    // Integers
    if (std::is_same<T, glm::i8>::value) this->setDataFormatId(DataFormatId::Int8);
    if (std::is_same<T, glm::i16>::value) this->setDataFormatId(DataFormatId::Int16);
    if (std::is_same<T, glm::i32>::value) this->setDataFormatId(DataFormatId::Int32);
    if (std::is_same<T, glm::i64>::value) this->setDataFormatId(DataFormatId::Int64);

    // Unsigned Integers
    if (std::is_same<T, glm::u8>::value) this->setDataFormatId(DataFormatId::UInt8);
    if (std::is_same<T, glm::u16>::value) this->setDataFormatId(DataFormatId::UInt16);
    if (std::is_same<T, glm::u32>::value) this->setDataFormatId(DataFormatId::UInt32);
    if (std::is_same<T, glm::u64>::value) this->setDataFormatId(DataFormatId::UInt64);
}

template <typename T, ind N>
template <typename VecNT>
void DataChannel<T, N>::getMin(VecNT& dest) const {
    static_assert(sizeof(VecNT) == sizeof(T) * N,
                  "Size and type do not agree with the vector type.");

    const MetaScalarType* min = this->template getMetaData<MetaScalarType>("Minimum");
    if (!min) {
        computeMinMax();
        min = this->template getMetaData<MetaScalarType>("Minimum");
    }

    T* rawVec = reinterpret_cast<T*>(&dest);
    auto mStr = min->getClassIdentifier();
    double* rawMin = reinterpret_cast<double*>(&mStr);  // For 'N == 1' case
    for (ind i = 0; i < N; ++i) rawVec[i] = static_cast<T>(rawMin[i]);
}

template <typename T, ind N>
template <typename VecNT>
void DataChannel<T, N>::getMax(VecNT& dest) const {
    static_assert(sizeof(VecNT) == sizeof(T) * N,
                  "Size and type do not agree with the vector type.");

    const MetaScalarType* max = this->template getMetaData<MetaScalarType>("Maximum");
    if (!max) {
        computeMinMax();
        max = this->template getMetaData<MetaScalarType>("Maximum");
    }
    T* rawVec = reinterpret_cast<T*>(&dest);
    auto mStr = max->getClassIdentifier();
    double* rawMax = reinterpret_cast<double*>(&mStr);  // For 'N == 1' case
    for (ind i = 0; i < N; ++i) rawVec[i] = static_cast<T>(rawMax[i]);
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

    auto* this_mut = const_cast<MetaDataOwner*>(static_cast<const MetaDataOwner*>(this));
    MetaScalarType* minMeta = this_mut->template createMetaData<MetaScalarType>("Minimum");
    MetaScalarType* maxMeta = this_mut->template createMetaData<MetaScalarType>("Maximum");

    // Working on raw T* to catch 'glmVec = T' case (for N == 1)

    T minT[N];
    T maxT[N];

    this->fill(minT, 0);
    this->fill(maxT, 0);

    for (const GlmVector& val : this->all<GlmVector>()) {
        const T* data = reinterpret_cast<const T*>(&val);
        for (ind dim = 0; dim < N; ++dim) {
            minT[dim] = std::min(minT[dim], data[dim]);
            maxT[dim] = std::max(maxT[dim], data[dim]);
        }
    }

    double minD[N], maxD[N];
    for (ind dim = 0; dim < N; ++dim) {
        minD[dim] = static_cast<double>(minT[dim]);
        maxD[dim] = static_cast<double>(maxT[dim]);
    }

    MetaVec& min = *reinterpret_cast<MetaVec*>(minD);
    MetaVec& max = *reinterpret_cast<MetaVec*>(maxD);
    std::cout << "Max: " << maxD[0] << '\n';

    minMeta->set(min);
    maxMeta->set(max);
}

}  // namespace discretedata
}  // namespace inviwo
