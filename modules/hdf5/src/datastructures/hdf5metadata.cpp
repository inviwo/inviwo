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

#include <modules/hdf5/datastructures/hdf5metadata.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/stringconversion.h>

namespace inviwo {

namespace hdf5 {

#include <warn/push>
#include <warn/ignore/switch-enum>

MetaData::MetaData(Path path, HDFType type, const DataFormatBase* format,
                   std::vector<size_t> dimensions)
    : path_(path), type_(type), format_(format), dimensions_(dimensions) {}

MetaData::MetaData(const MetaData& rhs) = default;

MetaData::MetaData(MetaData&& rhs) = default;

MetaData& MetaData::operator=(MetaData&& that) = default;

MetaData& MetaData::operator=(const MetaData& that) = default;

std::string MetaData::toString() const {
    switch (type_) {
        case MetaData::HDFType::Group:
            return "Group     " + path_.toString();
        case MetaData::HDFType::DataSet:
            return "DataSet   " + path_.toString() + " " + (format_ ? format_->getString() : "") +
                   (dimensions_.empty() ? " [" + joinString(dimensions_, ", ") + "]" : "");
        case MetaData::HDFType::Attribute:
            return "Attribute " + path_.toString() + " " + (format_ ? format_->getString() : "") +
                   (dimensions_.empty() ? " [" + joinString(dimensions_, ", ") + "]" : "");
        default:
            return "";
    }
}

std::vector<size_t> MetaData::getColumnMajorDimensions() const {
    std::vector<size_t> cmDims;
    std::reverse_copy(dimensions_.begin(), dimensions_.end(), std::back_inserter(cmDims));
    return cmDims;
}

std::vector<size_t> MetaData::getRowMajorDimensions() const { return dimensions_; }

namespace util {

template <typename T>
std::vector<MetaData> getAttributeMetaData(const T& grp, Path path) {
    std::vector<MetaData> metadata{};
    for (int i = 0; i < grp.getNumAttrs(); i++) {
        H5::Attribute attr = grp.openAttribute(i);

        metadata.emplace_back(path + attr.getName(), MetaData::HDFType::Attribute,
                              getDataFormat(attr.getDataType()), getDimensions(attr.getSpace()));
    }
    return metadata;
}

IVW_MODULE_HDF5_API std::vector<MetaData> getMetaData(const H5::Group& grp, Path path) {
    std::vector<MetaData> metadata{};
    metadata.emplace_back(path, MetaData::HDFType::Group);

    std::vector<MetaData> attrmd = getAttributeMetaData(grp, path);
    metadata.insert(metadata.end(), attrmd.begin(), attrmd.end());

    for (hsize_t i = 0; i < grp.getNumObjs(); i++) {
        const std::string childName = grp.getObjnameByIdx(i);
        const Path childpath{path + childName};

        switch (grp.getObjTypeByIdx(i)) {
            case H5G_GROUP: {
                H5::Group child = grp.openGroup(childName);
                std::vector<MetaData> childmd = getMetaData(child, childpath);
                metadata.insert(metadata.end(), childmd.begin(), childmd.end());
                break;
            }
            case H5G_DATASET: {
                H5::DataSet child = grp.openDataSet(childName);
                metadata.emplace_back(childpath, MetaData::HDFType::DataSet,
                                      getDataFormat(child.getDataType()),
                                      getDimensions(child.getSpace()));

                attrmd = getAttributeMetaData(child, childpath);
                metadata.insert(metadata.end(), attrmd.begin(), attrmd.end());
                break;
            }
            default:
                break;
        }
    }
    return metadata;
}

IVW_MODULE_HDF5_API std::vector<size_t> getDimensions(const H5::DataSpace space) {
    if (space.getSimpleExtentType() == H5S_SCALAR) {
        return std::vector<size_t>{1};
    } else if (space.getSimpleExtentType() == H5S_SIMPLE) {
        int rank = space.getSimpleExtentNdims();
        std::vector<hsize_t> dims(rank);
        space.getSimpleExtentDims(dims.data());
        std::vector<size_t> dims2(dims.begin(), dims.end());
        return dims2;
    } else {
        return std::vector<size_t>{};
    }
}

IVW_MODULE_HDF5_API const DataFormatBase* getDataFormat(const H5::DataType type) {
    if (type == H5::PredType::NATIVE_FLOAT)
        return DataFormatBase::get(DataFormatId::Float32);
    else if (type == H5::PredType::NATIVE_DOUBLE)
        return DataFormatBase::get(DataFormatId::Float64);
    else if (type == H5::PredType::NATIVE_SCHAR)
        return DataFormatBase::get(DataFormatId::Int8);
    else if (type == H5::PredType::NATIVE_CHAR)
        return DataFormatBase::get(DataFormatId::UInt8);
    else if (type == H5::PredType::NATIVE_SHORT)
        return DataFormatBase::get(DataFormatId::Int16);
    else if (type == H5::PredType::NATIVE_USHORT)
        return DataFormatBase::get(DataFormatId::UInt16);
    else if (type == H5::PredType::NATIVE_INT)
        return DataFormatBase::get(DataFormatId::Int32);
    else if (type == H5::PredType::NATIVE_UINT)
        return DataFormatBase::get(DataFormatId::UInt32);
    else if (type == H5::PredType::NATIVE_LLONG)
        return DataFormatBase::get(DataFormatId::Int64);
    else if (type == H5::PredType::NATIVE_ULLONG)
        return DataFormatBase::get(DataFormatId::UInt64);
    else
        return nullptr;
}
}  // namespace util

#include <warn/pop>

}  // namespace hdf5

}  // namespace inviwo
