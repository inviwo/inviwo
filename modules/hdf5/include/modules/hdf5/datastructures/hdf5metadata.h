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

#ifndef IVW_HDF5METADATA_H
#define IVW_HDF5METADATA_H

#include <modules/hdf5/hdf5moduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/hdf5/datastructures/hdf5path.h>
#include <H5Cpp.h>

namespace inviwo {

namespace hdf5 {

struct IVW_MODULE_HDF5_API MetaData {
public:
    enum class HDFType { Group, DataSet, Attribute };

    MetaData(Path path, HDFType type, const DataFormatBase* format = nullptr,
             std::vector<size_t> dimensions = std::vector<size_t>{});

    MetaData(const MetaData& rhs);
    MetaData& operator=(const MetaData& that);
    MetaData(MetaData&& rhs);
    MetaData& operator=(MetaData&& that);

    std::string toString() const;

    Path path_;
    HDFType type_;
    const DataFormatBase* format_;

    // return dimension in column major mode as used by Inviwo /OpenGL
    std::vector<size_t> getColumnMajorDimensions() const;
    // return dimension in row major mode as used by hdf / C / C++
    std::vector<size_t> getRowMajorDimensions() const;

private:
    // HDF Row major dimensions;
    std::vector<size_t> dimensions_;
};

template <typename CTy, typename CTr>
IVW_MODULE_HDF5_API std::basic_ostream<CTy, CTr>& operator<<(std::basic_ostream<CTy, CTr>& os,
                                                             const MetaData& metadata) {
    return os << metadata.toString();
}

namespace util {

IVW_MODULE_HDF5_API std::vector<MetaData> getMetaData(const H5::Group& grp, Path path = Path{});
IVW_MODULE_HDF5_API std::vector<size_t> getDimensions(const H5::DataSpace space);
IVW_MODULE_HDF5_API const DataFormatBase* getDataFormat(const H5::DataType type);
}  // namespace util

}  // namespace hdf5

}  // namespace inviwo

#endif  // IVW_HDF5METADATA_H
