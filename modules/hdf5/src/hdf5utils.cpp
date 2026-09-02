/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2026 Inviwo Foundation
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

#include <modules/hdf5/hdf5utils.h>

#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/stringconversion.h>

#include <warn/push>
#include <warn/ignore/all>
#include <H5Cpp.h>
#include <warn/pop>

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

namespace inviwo {

namespace hdf5 {

namespace {

std::vector<size_t> getDimensions(const H5::DataSpace& space) {
    if (space.getSimpleExtentType() == H5S_SCALAR) {
        return {1};
    } else if (space.getSimpleExtentType() == H5S_SIMPLE) {
        const int rank = space.getSimpleExtentNdims();
        std::vector<hsize_t> dims(rank);
        space.getSimpleExtentDims(dims.data());
        return {dims.begin(), dims.end()};
    } else {
        return {};
    }
}

const DataFormatBase* getDataFormat(const H5::DataType& type) {
    if (type == H5::PredType::NATIVE_FLOAT) {
        return DataFormatBase::get(DataFormatId::Float32);
    } else if (type == H5::PredType::NATIVE_DOUBLE) {
        return DataFormatBase::get(DataFormatId::Float64);
    } else if (type == H5::PredType::NATIVE_SCHAR) {
        return DataFormatBase::get(DataFormatId::Int8);
    } else if (type == H5::PredType::NATIVE_CHAR) {
        return DataFormatBase::get(DataFormatId::UInt8);
    } else if (type == H5::PredType::NATIVE_SHORT) {
        return DataFormatBase::get(DataFormatId::Int16);
    } else if (type == H5::PredType::NATIVE_USHORT) {
        return DataFormatBase::get(DataFormatId::UInt16);
    } else if (type == H5::PredType::NATIVE_INT) {
        return DataFormatBase::get(DataFormatId::Int32);
    } else if (type == H5::PredType::NATIVE_UINT) {
        return DataFormatBase::get(DataFormatId::UInt32);
    } else if (type == H5::PredType::NATIVE_LLONG) {
        return DataFormatBase::get(DataFormatId::Int64);
    } else if (type == H5::PredType::NATIVE_ULLONG) {
        return DataFormatBase::get(DataFormatId::UInt64);
    } else {
        return nullptr;
    }
}

}  // namespace

std::vector<size_t> DataSetInfo::getColumnMajorDimensions() const {
    std::vector<size_t> cmDims;
    std::ranges::reverse_copy(dimensions_, std::back_inserter(cmDims));
    return cmDims;
}

namespace util {

std::vector<DataSetInfo> getDataSets(const Handle& handle) {
    std::vector<DataSetInfo> datasets;
    const auto collect = [&](const Handle& group) {
        for (const auto& dataset : group.datasets()) {
            datasets.emplace_back({.path_ = Path{dataset.getObjName()},
                                   .format_ = getDataFormat(dataset.getDataType()),
                                   .dimensions_ = getDimensions(dataset.getSpace())});
        }
    };
    collect(handle);
    handle.visitGroups(collect);
    return datasets;
}

std::string dataSetDescription(const DataSetInfo& info) {
    const auto dims = info.getColumnMajorDimensions();
    return fmt::format("{}{}{} [{}]", info.path_.toString(), (info.format_ ? " " : ""),
                       (info.format_ ? info.format_->getString() : ""), fmt::join(dims, ", "));
}

std::vector<OptionPropertyIntOption> conversionOptions() {
    return {{"none", "No conversion", 0},
            {"float", "Float", 1},
            {"double", "Double", 2},
            {"uchar", "Unsigned Char", 3},
            {"ushort", "Unsigned Short", 4}};
}

const DataFormatBase* conversionFormat(size_t index) {
    switch (index) {
        case 1:
            return DataFloat32::get();
        case 2:
            return DataFloat64::get();
        case 3:
            return DataUInt8::get();
        case 4:
            return DataUInt16::get();
        default:
            return nullptr;
    }
}

}  // namespace util

}  // namespace hdf5

}  // namespace inviwo
