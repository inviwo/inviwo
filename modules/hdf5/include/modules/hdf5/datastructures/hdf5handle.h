/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#ifndef IVW_HDF5DATA_H
#define IVW_HDF5DATA_H

#include <modules/hdf5/hdf5moduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/document.h>

#include <H5Cpp.h>

#include <modules/hdf5/hdf5utils.h>
#include <modules/hdf5/hdf5types.h>
#include <modules/hdf5/hdf5exception.h>

#include <limits>
#include <functional>
#include <type_traits>

#include <half/half.hpp>

namespace inviwo {

namespace hdf5 {

class IVW_MODULE_HDF5_API Handle {
public:
    struct Selection {
        Selection(size_t startval, size_t endval, size_t strideval)
            : start(startval), end(endval), stride(strideval) {}
        size_t start;
        size_t end;
        size_t stride;
    };

    Handle(std::string filename);
    Handle(std::string filename, Path path);
    Handle(const Handle& rhs);
    Handle& operator=(const Handle& that);
    Handle(Handle&& rhs);
    Handle& operator=(Handle&& that);

    ~Handle();

    Document getInfo() const;

    const H5::Group& getGroup() const;

    Handle* getHandleForPath(const std::string& path) const;

    std::shared_ptr<Volume> getVolumeAtPathAsType(const Path& path,
                                                  std::vector<Selection> selection,
                                                  const DataFormatBase* type) const;

    template <typename T>
    std::vector<T> getVectorAtPath(const Path& path) const;

    template <typename T>
    std::vector<glm::tvec3<T, glm::defaultp>> getVectorOfVec3AtPath(const Path& path) const;

    static const uvec3 colorCode;
    static const std::string classIdentifier;
    static const std::string dataName;

private:
    double getMin(const DataFormatBase* type) const;
    double getMax(const DataFormatBase* type) const;

    H5::Group data_;
    std::string filename_;
    Path path_;
};

template <typename T>
std::vector<T> Handle::getVectorAtPath(const Path& path) const {
    H5::DataSet ds = data_.openDataSet(path);
    size_t rank = ds.getSpace().getSimpleExtentNdims();

    hsize_t* dims = new hsize_t[rank];
    ds.getSpace().getSimpleExtentDims(dims);

    size_t size = 1;
    for (size_t i = 0; i < rank; ++i) {
        size *= dims[i];
    }
    H5::DataType dt = ds.getDataType();

    if (dt == TypeMap<T>::getType()) {
        std::vector<T> res(size);
        ds.read(res.data(), ds.getDataType());
        return res;
    } else {
        throw Exception("Trying to read invalid data type");
    }
}

template <typename T>
std::vector<glm::tvec3<T, glm::defaultp>> Handle::getVectorOfVec3AtPath(const Path& path) const {
    H5::DataSet ds = data_.openDataSet(path);
    size_t rank = ds.getSpace().getSimpleExtentNdims();

    if (rank != 2) throw Exception("Trying to read data with invalid rank");

    hsize_t* dims = new hsize_t[rank];
    ds.getSpace().getSimpleExtentDims(dims);

    size_t size = 1;
    for (size_t i = 0; i < rank; ++i) {
        size *= dims[i];
    }

    if (dims[1] != 3) throw Exception("Trying to read data with invalid dimension");

    H5::DataType dt = ds.getDataType();

    if (dt == TypeMap<T>::getType()) {
        std::vector<glm::tvec3<T, glm::defaultp>> res(dims[0]);
        ds.read(reinterpret_cast<T*>(res.data()), ds.getDataType());
        return res;
    } else {
        throw Exception("Trying to read invalid data type");
    }
}

/*
H5T_NATIVE_CHAR 	char
H5T_NATIVE_SCHAR 	signed char
H5T_NATIVE_UCHAR 	unsigned char
H5T_NATIVE_SHORT 	short
H5T_NATIVE_USHORT 	unsigned short
H5T_NATIVE_INT 	int
H5T_NATIVE_UINT 	unsigned
H5T_NATIVE_LONG 	long
H5T_NATIVE_ULONG 	unsigned long
H5T_NATIVE_LLONG 	long long
H5T_NATIVE_ULLONG 	unsigned long long
H5T_NATIVE_FLOAT 	float
H5T_NATIVE_DOUBLE 	double
H5T_NATIVE_LDOUBLE 	long double
H5T_NATIVE_HSIZE 	hsize_t
H5T_NATIVE_HSSIZE 	hssize_t
H5T_NATIVE_HERR 	herr_t
H5T_NATIVE_HBOOL 	hbool_t
*/

}  // namespace hdf5

}  // namespace inviwo

#endif  // IVW_HDF5DATA_H
