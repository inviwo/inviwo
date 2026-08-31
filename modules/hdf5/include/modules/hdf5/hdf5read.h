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

namespace inviwo {

class DataFormatBase;

namespace hdf5 {

/**
 * Read the dataset at @p handle into a Volume. The @p selection defines a hyperslab per dimension
 * in column major (Inviwo) order. If @p type is null the data format is deduced from the dataset.
 * @p getVolume is used to allocate the resulting Volume, allowing the caller to reuse storage.
 */
IVW_MODULE_HDF5_API std::shared_ptr<Volume> getVolumeAtPathAsType(
    const Handle& handle, std::vector<Selection> selection, const DataFormatBase* type,
    const std::function<std::shared_ptr<Volume>(const VolumeConfig&)>& getVolume);

/**
 * Read the dataset at @p handle into a Layer. @see getVolumeAtPathAsType.
 */
IVW_MODULE_HDF5_API std::shared_ptr<Layer> getLayerAtPathAsType(const Handle& handle,
                                                                std::vector<Selection> selection,
                                                                const DataFormatBase* type);

/**
 * Read the dataset at @p handle into a Buffer. @see getVolumeAtPathAsType.
 */
IVW_MODULE_HDF5_API std::shared_ptr<BufferBase> getBufferAtPathAsType(
    const Handle& handle, std::vector<Selection> selection, const DataFormatBase* type);

}  // namespace hdf5

}  // namespace inviwo
