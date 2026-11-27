/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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
#include <modules/hdf5/datastructures/hdf5handle.h>
#include <modules/hdf5/datastructures/hdf5selection.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/buffer/buffer.h>

#include <functional>
#include <memory>
#include <vector>
#include <span>
#include <cstddef>
#include <cstdint>

namespace inviwo {

class DataFormatBase;

namespace hdf5 {

/**
 * Controls when the data of a Volume or Layer created from an HDF5 dataset is read.
 */
enum class Loading : std::uint8_t {
    Eager,  ///< Read the data into a RAM representation right away.
    Lazy    ///< Create a VolumeHDF or LayerHDF representation which reads the data on demand when
            ///< it is converted to another representation, e.g. directly into a GL texture.
};

IVW_MODULE_HDF5_API VolumeConfig getVolumeConfig(const Handle& handle,
                                                 const std::vector<Selection>& selection,
                                                 VolumeConfig volumeConfig);
IVW_MODULE_HDF5_API LayerConfig getLayerConfig(const Handle& handle,
                                               const std::vector<Selection>& selection,
                                               LayerConfig layerConfig);

IVW_MODULE_HDF5_API std::pair<VolumeConfig, Selection> getTemporalVolumeConfig(
    const Handle& handle, std::vector<Selection> selection, size_t timeDimension,
    VolumeConfig volumeConfig);

/**
 * Read the hyperslab @p selection of the dataset at @p handle into @p dst, converting the data to
 * @p format. The @p selection is given in column major (Inviwo) order and must resolve to
 * @p dimensions, see getVolumeConfig. @p dst must hold exactly `glm::compMul(dimensions)` elements
 * of @p format, which will be written in column major order.
 * @throws Exception if the selection does not match @p dimensions, if the size of @p dst does not
 * match, or if the data could not be read.
 */
IVW_MODULE_HDF5_API void readVolumeData(const Handle& handle,
                                        const std::vector<Selection>& selection, size3_t dimensions,
                                        const DataFormatBase* format, std::span<std::byte> dst);

/**
 * Read the hyperslab @p selection of the dataset at @p handle into @p dst, converting the data to
 * @p format. @see readVolumeData
 */
IVW_MODULE_HDF5_API void readLayerData(const Handle& handle,
                                       const std::vector<Selection>& selection, size2_t dimensions,
                                       const DataFormatBase* format, std::span<std::byte> dst);

/**
 * Read the dataset at @p handle into a Volume. The @p selection defines a hyperslab per
 * dimension in column major (Inviwo) order. If @p type is null the data format is deduced from
 * the dataset.
 * @p getVolume is used to allocate the resulting Volume, allowing the caller to reuse storage.
 * With Loading::Lazy no data is read, instead the Volume gets a VolumeHDF representation which
 * reads the data on demand. Any existing representations of a reused Volume are invalidated.
 */
IVW_MODULE_HDF5_API std::shared_ptr<Volume> getVolumeAtPathAsType(
    const Handle& handle, const std::vector<Selection>& selection, VolumeConfig volumeConfig,
    const std::function<std::shared_ptr<Volume>(const VolumeConfig&)>& getVolume,
    Loading loading = Loading::Eager);

/**
 * Helper for the @p getVolume callback of getVolumeAtPathAsType. If @p reuse has the same
 * dimensions and data format as @p config its metadata is reset from @p config and it is returned,
 * otherwise a new Volume is created from @p config.
 */
IVW_MODULE_HDF5_API std::shared_ptr<Volume> reuseOrCreateVolume(const VolumeConfig& config,
                                                                std::shared_ptr<Volume> reuse);

/**
 * Read the dataset at @p handle into a Layer. @see getVolumeAtPathAsType.
 */
IVW_MODULE_HDF5_API std::shared_ptr<Layer> getLayerAtPathAsType(
    const Handle& handle, const std::vector<Selection>& selection, LayerConfig layerConfig,
    Loading loading = Loading::Eager);

/**
 * Read the dataset at @p handle into a Buffer. @see getVolumeAtPathAsType.
 */
IVW_MODULE_HDF5_API std::shared_ptr<BufferBase> getBufferAtPathAsType(
    const Handle& handle, const std::vector<Selection>& selection,
    std::optional<DataFormatId> formatId);

IVW_MODULE_HDF5_API glm::dmat4 getBasis(const Handle& handle);

IVW_MODULE_HDF5_API glm::dvec2 getRange(const Handle& handle);

}  // namespace hdf5

}  // namespace inviwo
