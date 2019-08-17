/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#ifndef IVW_HDF5TYPES_H
#define IVW_HDF5TYPES_H

#include <modules/hdf5/hdf5moduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/hdf5/hdf5exception.h>
#include <H5Cpp.h>

namespace inviwo {

namespace hdf5 {

template <typename T>
struct TypeMap {
    static H5::PredType getType() {
        throw Exception("Trying to read unknown type", IVW_CONTEXT_CUSTOM("HDFTypeMap"));
    }
};

template <>
struct TypeMap<float> {
    static H5::PredType getType() { return H5::PredType::NATIVE_FLOAT; }
};
template <>
struct TypeMap<double> {
    static H5::PredType getType() { return H5::PredType::NATIVE_DOUBLE; }
};

template <>
struct TypeMap<char> {
    static H5::PredType getType() { return H5::PredType::NATIVE_INT8; }
};
template <>
struct TypeMap<short> {
    static H5::PredType getType() { return H5::PredType::NATIVE_INT16; }
};
template <>
struct TypeMap<int> {
    static H5::PredType getType() { return H5::PredType::NATIVE_INT32; }
};
template <>
struct TypeMap<long long> {
    static H5::PredType getType() { return H5::PredType::NATIVE_INT64; }
};

template <>
struct TypeMap<unsigned char> {
    static H5::PredType getType() { return H5::PredType::NATIVE_UINT8; }
};
template <>
struct TypeMap<unsigned short> {
    static H5::PredType getType() { return H5::PredType::NATIVE_UINT16; }
};
template <>
struct TypeMap<unsigned int> {
    static H5::PredType getType() { return H5::PredType::NATIVE_UINT32; }
};
template <>
struct TypeMap<unsigned long long> {
    static H5::PredType getType() { return H5::PredType::NATIVE_UINT64; }
};

namespace util {
IVW_MODULE_HDF5_API const DataFormatBase* getDataFormatFromDataSet(const H5::DataSet& dataset);
}

}  // namespace hdf5

}  // namespace inviwo

#endif  // IVW_HDF5TYPES_H
