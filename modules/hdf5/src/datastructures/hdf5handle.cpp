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

#include <modules/hdf5/datastructures/hdf5handle.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>

#include <modules/base/algorithm/dataminmax.h>

#include <algorithm>

namespace inviwo {

namespace hdf5 {

Handle::Handle(std::string filename) : filename_(filename), path_("/") {
    H5::H5File hdfFile(filename_, H5F_ACC_RDONLY);
    data_ = hdfFile.openGroup(path_);
}

Handle::Handle(std::string filename, Path path) : filename_(filename), path_(path) {
    H5::H5File hdfFile(filename_, H5F_ACC_RDONLY);
    data_ = hdfFile.openGroup(path_);
}

Handle::Handle(const Handle& rhs) : filename_(rhs.filename_), path_(rhs.path_) {
    H5::H5File hdfFile(filename_, H5F_ACC_RDONLY);
    data_ = hdfFile.openGroup(path_);
}

Handle::Handle(Handle&& rhs) : filename_(rhs.filename_), path_(rhs.path_) {
    H5::H5File hdfFile(filename_, H5F_ACC_RDONLY);
    data_ = hdfFile.openGroup(path_);
}

Handle& Handle::operator=(Handle&& that) {
    if (this != &that) {
        filename_ = that.filename_;
        path_ = that.path_;
        data_.close();
        H5::H5File hdfFile(filename_, H5F_ACC_RDONLY);
        data_ = hdfFile.openGroup(path_);
    }
    return *this;
}

Handle& Handle::operator=(const Handle& that) {
    if (this != &that) {
        filename_ = that.filename_;
        path_ = that.path_;
        data_.close();
        H5::H5File hdfFile(filename_, H5F_ACC_RDONLY);
        data_ = hdfFile.openGroup(path_);
    }
    return *this;
}

Handle::~Handle() { data_.close(); }

Handle* Handle::getHandleForPath(const std::string& path) const {
    return new Handle(this->filename_, path_ + path);
}

Document Handle::getInfo() const {
    Document doc;
    doc.append("p", "File: " + filename_ + path_);
    return doc;
}

double Handle::getMin(const DataFormatBase* type) const {
    switch (type->getNumericType()) {
        case NumericType::Float:
            return 0.0;
        case NumericType::UnsignedInteger:
            return 0.0;
        case NumericType::SignedInteger:
            return type->getMin();
        case NumericType::NotSpecialized:
        default:
            return 0.0;
    }
}
double Handle::getMax(const DataFormatBase* type) const {
    switch (type->getNumericType()) {
        case NumericType::Float:
            return 1.0;
        case NumericType::UnsignedInteger:
            return type->getMax();
        case NumericType::SignedInteger:
            return type->getMax();
        case NumericType::NotSpecialized:
        default:
            return 1.0;
    }
}

std::shared_ptr<Volume> Handle::getVolumeAtPathAsType(const Path& path,
                                                      std::vector<Selection> selection,
                                                      const DataFormatBase* type) const {

    auto dataset = data_.openDataSet(path);
    ::inviwo::util::OnScopeExit closedataset{[&]() { dataset.close(); }};

    const H5::DataSpace dataSpace = dataset.getSpace();
    const size_t rank = dataSpace.getSimpleExtentNdims();
    if (selection.size() != rank) {
        throw Exception("Selection not of the same rank as the data", IVW_CONTEXT);
    }

    std::vector<hsize_t> dataDimensions(rank);
    dataSpace.getSimpleExtentDims(dataDimensions.data());
    const hsize_t dataSize = dataSpace.getSelectNpoints();

    std::vector<hsize_t> start(rank);
    std::vector<hsize_t> count(rank);
    std::vector<hsize_t> stride(rank);

    /*
     * Column major, i.e. the FIRST listed dimension is the fasted changing
     * Inviwo, OpenGL, matlab, Fortran
     *
     * Row major, i.e. the LAST listed dimension is the fasted changing
     * HDF, C/C++, Mathematica, Python
     *
     * Solution reverse all the dimension lists.
     * Row major version of the selection to match the hdf row major dataDimensions.
     */
    std::reverse(selection.begin(), selection.end());

    size3_t volumeDimensions(1);
    int resRank = 0;
    std::vector<hsize_t> memoryDimensions{1, 1, 1};

    for (size_t i = 0; i < rank; ++i) {
        start[i] = selection[i].start;
        count[i] =
            static_cast<hsize_t>((selection[i].end - selection[i].start) / selection[i].stride);
        stride[i] = selection[i].stride;

        if (count[i] > 1) {
            if (resRank > 2) throw Exception("Invalid selection, resulting rank > 3", IVW_CONTEXT);
            memoryDimensions[resRank] = count[i];
            volumeDimensions[resRank] = count[i];
            resRank++;
        }
    }

    dataSpace.selectHyperslab(H5S_SELECT_SET, count.data(), start.data(), stride.data(), nullptr);

    H5::DataSpace memorySpace(3, memoryDimensions.data());
    memorySpace.selectAll();

    hsize_t selectionSize = memorySpace.getSelectNpoints();

    LogInfo("Data rank: " << rank << " dims " << joinString(dataDimensions, " x ") << " size "
                          << dataSize << " selection " << dataSpace.getSelectNpoints()
                          << " memory size " << memorySpace.getSelectNpoints() << " memory dim "
                          << volumeDimensions);

    const DataFormatBase* format = type ? type : util::getDataFormatFromDataSet(dataset);

    // Reverse back the Column major
    std::reverse(&volumeDimensions[0], &volumeDimensions[0] + volumeDimensions.length());
    auto volumeram = createVolumeRAM(volumeDimensions, format);

    auto minmax = volumeram->dispatch<std::pair<dvec4, dvec4>, dispatching::filter::Scalars>(
        [&](auto vrprecision) {
            using ValueType = ::inviwo::util::PrecisionValueType<decltype(vrprecision)>;

            ValueType* data = vrprecision->getDataTyped();

            try {
                dataset.read(data, TypeMap<ValueType>::getType(), memorySpace, dataSpace);
            } catch (H5::DataSetIException& e) {
                throw Exception("HDF: unable to read data: " + e.getDetailMsg(), IVW_CONTEXT);
            }

            auto res = ::inviwo::util::dataMinMax(data, selectionSize);

            LogInfo("Read HDF volume type: " << DataFormat<ValueType>::str()
                                             << " data range: " << res.first << ", " << res.second
                                             << " file: " << dataset.getFileName());

            return res;
        });

    auto volume = std::make_shared<Volume>(volumeDimensions, format);
    volume->dataMap_.dataRange.x = glm::compMin(minmax.first);
    volume->dataMap_.dataRange.y = glm::compMax(minmax.second);
    volume->dataMap_.valueRange = volume->dataMap_.dataRange;

    volume->addRepresentation(volumeram);

    return volume;
}

const uvec3 Handle::colorCode = uvec3(101, 101, 188);

const std::string Handle::classIdentifier = "org.inviwo.hdf5.handle";

const std::string Handle::dataName = "HDF";

const H5::Group& Handle::getGroup() const { return data_; }

}  // namespace hdf5

}  // namespace inviwo
