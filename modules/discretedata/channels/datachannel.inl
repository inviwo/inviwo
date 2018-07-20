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

#include <discretedata/discretedatamoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/metadata/metadataowner.h>
#include <inviwo/core/metadata/metadata.h>

namespace inviwo {
namespace dd {

template <typename T, ind N>
DataChannel<T, N>::DataChannel(const std::string& name, GridPrimitive definedOn)
    : Channel(N, name, DataFormatId::NotSpecialized, definedOn) {
    // Switch all types.
    if (std::is_same<T, f16>::value)      setDataFormatId(DataFormatId::Float16);
    if (std::is_same<T, glm::f32>::value) setDataFormatId(DataFormatId::Float32);
    if (std::is_same<T, glm::f64>::value) setDataFormatId(DataFormatId::Float64);

    // Integers
    if (std::is_same<T, glm::i8>::value)  setDataFormatId(DataFormatId::Int8);
    if (std::is_same<T, glm::i16>::value) setDataFormatId(DataFormatId::Int16);
    if (std::is_same<T, glm::i32>::value) setDataFormatId(DataFormatId::Int32);
    if (std::is_same<T, glm::i64>::value) setDataFormatId(DataFormatId::Int64);

    // Unsigned Integers
    if (std::is_same<T, glm::u8>::value)  setDataFormatId(DataFormatId::UInt8);
    if (std::is_same<T, glm::u16>::value) setDataFormatId(DataFormatId::UInt16);
    if (std::is_same<T, glm::u32>::value) setDataFormatId(DataFormatId::UInt32);
    if (std::is_same<T, glm::u64>::value) setDataFormatId(DataFormatId::UInt64);
}

}  // namespace
}
