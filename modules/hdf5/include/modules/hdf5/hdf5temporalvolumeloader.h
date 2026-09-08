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

#include <inviwo/core/datastructures/volume/temporalvolume.h>
#include <inviwo/core/datastructures/volume/volumeconfig.h>
#include <inviwo/core/util/glmmat.h>

#include <cstddef>
#include <memory>
#include <mutex>
#include <vector>

namespace inviwo {

class DataFormatBase;

namespace hdf5 {

/**
 * @brief A VolumeLoader that reads one time-varying HDF5 dataset with rank >= 4, treating one
 * dimension as the time axis and the remaining three as the volume axes.
 *
 * @p selection must have one entry per dataset dimension in column major (Inviwo) order,
 * i.e. `[extraDims..., X, Y, Z]`, as returned by DimSelectionsProperty::getSelection(). The entry
 * at @p timeDimension is re-targeted per frame; every other "extra" entry (i.e. every entry before
 * the trailing X/Y/Z ones) must clamp to a single index, or construction throws.
 *
 * @note load() serializes all HDF5 access via an internal mutex since the underlying HDF5 C
 * library is not guaranteed to be safe for concurrent use from the background thread pool that
 * TemporalVolume may invoke load() from.
 */
class IVW_MODULE_HDF5_API HDF5TemporalVolumeLoader : public VolumeLoader {
public:
    /**
     * @param handle        HDF5 handle already pointing at the dataset to read
     * @param selection     one Selection per dataset dimension, column major order
     * @param timeDimension index into @p selection designating the time axis
     * @param format        output data format, or nullptr to deduce it from the dataset
     * @param basis         model matrix applied to every produced frame
     * @param dt            time between consecutive frames, in seconds
     */
    HDF5TemporalVolumeLoader(Handle handle, std::vector<Selection> selection, size_t timeDimension,
                             const DataFormatBase* format, const dmat4& basis, double dt);

    virtual std::shared_ptr<Volume> load(size_t index, std::shared_ptr<Volume> reuse) override;
    virtual size_t size() const override;
    virtual std::span<const Seconds> times() const override;
    virtual VolumeConfig prototype() const override;

private:
    std::shared_ptr<Volume> readFrame(size_t rawIndex, std::shared_ptr<Volume> reuse) const;

    mutable std::mutex mutex_;
    Handle handle_;
    std::vector<Selection> selection_;
    size_t timeDimension_;
    const DataFormatBase* format_;
    dmat4 basis_;

    size_t rawStart_;
    size_t rawStride_;
    std::vector<Seconds> times_;
    VolumeConfig prototype_;
};

}  // namespace hdf5

}  // namespace inviwo
