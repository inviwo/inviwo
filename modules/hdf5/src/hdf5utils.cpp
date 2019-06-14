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

#include <modules/hdf5/hdf5utils.h>

namespace inviwo {

namespace hdf5 {

Paths findpaths(const H5::Group& grp, const Path& path, const std::string& type) {
    Paths paths;

    if (isOfType(grp, type)) {
        paths.push_back(path);
    } else {
        for (hsize_t i = 0; i < grp.getNumObjs(); i++) {
            std::string childName = grp.getObjnameByIdx(i);
            if (grp.getObjTypeByIdx(i) == H5G_GROUP) {
                H5::Group child = grp.openGroup(childName);
                Paths newpaths = findpaths(child, path + childName, type);
                paths.insert(paths.end(), newpaths.begin(), newpaths.end());
            }
        }
    }
    return paths;
}

VolumeInfos getVolumeInfo(const H5::DataSet& ds, const Path& path) {
    auto size = std::make_unique<hsize_t[]>(ds.getSpace().getSimpleExtentNdims());
    ds.getSpace().getSimpleExtentDims(size.get());
    int sub_densities = (int)size[0];

    VolumeInfos paths;

    for (int i = 0; i < sub_densities; i++) {
        VolumeInfo info;
        info.path_ = path;
        info.index_ = i;
        info.dim_.x = size[1];
        info.dim_.y = size[2];
        info.dim_.z = size[3];

        paths.push_back(info);
    }

    return paths;
}

bool isOfType(const H5::Group& grp, const std::string& type) {
    bool result = false;
    try {
        if (grp.attrExists("type")) {
            H5::Attribute attr = grp.openAttribute("type");
            H5::DataType dt = attr.getDataType();

            if (dt.getClass() == H5T_STRING && dt.isVariableStr()) {
                H5std_string val;
                attr.read(dt, val);
                if (val == type) {
                    result = true;
                }
            }
            dt.close();
            attr.close();
        }
    } catch (const H5::AttributeIException&) {
    }
    return result;
}

}  // namespace hdf5

}  // namespace inviwo
