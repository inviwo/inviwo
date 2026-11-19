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
#include <modules/hdf5/datastructures/hdf5path.h>
#include <modules/hdf5/datastructures/hdf5selection.h>

#include <inviwo/core/datastructures/volume/temporalvolume.h>
#include <inviwo/core/datastructures/volume/volumeconfig.h>

#include <cstddef>
#include <filesystem>
#include <memory>
#include <stop_token>
#include <utility>
#include <vector>

namespace inviwo::hdf5 {

/**
 * @brief A VolumeLoader that concatenates the time steps of the same dataset found in a sequence
 * of HDF5 files into one time series.
 *
 * Every file is expected to contain a dataset at @p path with the same structure, i.e. the same
 * rank and the same extents along all non-time dimensions. The number of time steps may differ
 * between files. Frames are numbered consecutively across files in the given file order.
 *
 * Only dataset metadata (the dataspace) is read at construction to determine the number of time
 * steps per file. Files are opened on demand in load() and closed again afterwards, so the
 * number of files is not limited by the number of simultaneously open HDF5 files. A file whose
 * dataset does not match the prototype dimensions or data format causes load() to throw.
 *
 * @p selection must have one entry per dataset dimension in column major (Inviwo) order,
 * @see HDF5TemporalVolumeLoader. The entry at @p timeDimension is clamped per file.
 */
class IVW_MODULE_HDF5_API HDF5MultiFileTemporalVolumeLoader : public TemporalVolumeLoader {
public:
    /**
     * @param files         the HDF5 files, in time order
     * @param path          path to the dataset inside each file
     * @param selection     one Selection per dataset dimension, column major order
     * @param timeDimension index into @p selection designating the time axis
     * @param dt            time between consecutive frames
     * @param config        the prototype VolumeConfig to use
     * @throws Exception if @p files is empty or any file has an incompatible dataset rank
     */
    HDF5MultiFileTemporalVolumeLoader(std::vector<std::filesystem::path> files, Path path,
                                      std::vector<Selection> selection, size_t timeDimension,
                                      Seconds dt, VolumeConfig config);

    virtual std::shared_ptr<Volume> load(size_t index, std::shared_ptr<Volume> reuse,
                                         std::stop_token stop) const override;
    virtual size_t size() const override;
    virtual Seconds time(size_t index) const override;
    virtual VolumeConfig prototype() const override;

    /**
     * Map a global frame index to a file index and the frame index within that file.
     * @throws Exception if @p index is out of range
     */
    std::pair<size_t, size_t> locate(size_t index) const;

private:
    std::vector<std::filesystem::path> files_;
    Path path_;
    std::vector<Selection> selection_;
    size_t timeDimension_;
    std::vector<Selection> timeSelections_;  // one per file
    std::vector<size_t> frameOffsets_;       // files_.size() + 1 entries, cumulative frame counts
    Seconds dt_;
    VolumeConfig prototype_;
};

}  // namespace inviwo::hdf5
