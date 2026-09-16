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

#include <inviwo/core/algorithm/rangeutils.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/properties/optionproperty.h>

#include <modules/hdf5/datastructures/hdf5path.h>
#include <modules/hdf5/datastructures/hdf5handle.h>
#include <modules/hdf5/datastructures/hdf5selection.h>

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
    Path path;
    const DataFormatBase* format = nullptr;
    std::vector<size_t> dimensions;

    /// Dimensions in column major (Inviwo/OpenGL) order.
    [[nodiscard]] auto getColumnMajorDimensions() const { return dimensions | std::views::reverse; }
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
IVW_MODULE_HDF5_API const DataFormatBase* conversionFormat(size_t index);

inline constexpr auto dataSetInfoToOption = [](const DataSetInfo& info) {
    return OptionPropertyStringOption{info.path.toString(), util::dataSetDescription(info),
                                      info.path.toString()};
};

inline constexpr glm::dmat4 createBasis(glm::size3_t dim, glm::dvec3 spacing) {
    auto basis = glm::diagonal4x4(dvec4{dvec3{dim} * spacing, 1.0});
    basis[3] = dvec4{-0.5 * dvec3(basis[0] + basis[1] + basis[2]), 1.0};
    return basis;
}

inline constexpr auto validSelectionAndDims(range_of<Selection> auto selections,
                                            range_of<size_t> auto dimensions) {

    return std::views::zip(selections, dimensions) | std::views::transform([](auto&& item) {
               return std::tuple{std::apply(clamp, item), std::get<1>(item)};
           }) |
           std::views::filter([](auto&& item) { return std::get<0>(item).count > 1; });
}

inline constexpr glm::dmat4 adjustBasis(glm::dmat4 basis, range_of<Selection> auto selections,
                                        range_of<size_t> auto dimensions, bool adjustBasis,
                                        bool adjustOffset) {
    if (!adjustBasis) return basis;

    auto selAndDims = validSelectionAndDims(selections, dimensions);

    for (auto&& [i, item] : std::views::zip(std::views::iota(0uz), selAndDims)) {
        if (i > 2) throw Exception("Invalid selection, resulting rank > 3");

        auto&& [sel, dim] = item;
        if (adjustOffset) {
            basis[3] += basis[i] * static_cast<double>(sel.start) / static_cast<double>(dim);
        }
        basis[i] *= static_cast<double>(sel.count * sel.stride) / static_cast<double>(dim);
    }
    if (!adjustOffset) {
        const vec3 offset = -0.5f * vec3(basis[0] + basis[1] + basis[2]);
        basis[3] = vec4(offset, 1.0f);
    }
    return basis;
}

}  // namespace util

}  // namespace hdf5

}  // namespace inviwo
