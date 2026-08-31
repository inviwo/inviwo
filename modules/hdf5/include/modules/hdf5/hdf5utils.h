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

#pragma once

#include <modules/hdf5/hdf5moduledefine.h>
#include <modules/hdf5/datastructures/hdf5path.h>
#include <modules/hdf5/datastructures/hdf5handle.h>

#include <inviwo/core/properties/optionproperty.h>

#include <string>
#include <vector>

namespace inviwo {

class DataFormatBase;

namespace hdf5 {

/**
 * Lightweight description of an HDF5 dataset: its path, data format, and dimensions (stored in
 * HDF row major order).
 */
struct IVW_MODULE_HDF5_API DataSetInfo {
    Path path_;
    const DataFormatBase* format_ = nullptr;
    std::vector<size_t> dimensions_;

    /// Dimensions in column major (Inviwo/OpenGL) order.
    [[nodiscard]] std::vector<size_t> getColumnMajorDimensions() const;
};

namespace util {

/**
 * Collect info for every dataset reachable from @p handle, recursing into all subgroups.
 */
IVW_MODULE_HDF5_API std::vector<DataSetInfo> getDataSets(const Handle& handle);

/**
 * A human readable description of a dataset: path, data format and column major dimensions,
 * e.g. `"/group/data FLOAT32 [64, 64, 64]"`.
 */
IVW_MODULE_HDF5_API std::string dataSetDescription(const DataSetInfo& info);

/**
 * The options for the "Convert to type" property shared by the data source processors. The
 * selected index maps to a data format via @see conversionFormat.
 */
IVW_MODULE_HDF5_API std::vector<OptionPropertyIntOption> conversionOptions();

/**
 * Map the selected index of the "Convert to type" property to a data format. Index 0 (no
 * conversion) returns nullptr.
 */
IVW_MODULE_HDF5_API const DataFormatBase* conversionFormat(int index);

}  // namespace util

}  // namespace hdf5

}  // namespace inviwo
