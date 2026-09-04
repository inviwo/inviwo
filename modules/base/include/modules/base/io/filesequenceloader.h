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

#include <modules/base/basemoduledefine.h>

#include <inviwo/core/datastructures/volume/temporalvolume.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/util/fileextension.h>

#include <filesystem>
#include <memory>
#include <mutex>
#include <span>
#include <vector>

namespace inviwo {

class DataReaderFactory;

/**
 * @ingroup dataio
 *
 * @brief A VolumeLoader that loads one Volume file per time step (frame).
 *
 * Each frame corresponds to a single file. Files are read on demand using Inviwo's
 * DataReaderFactory, so any registered Volume reader can be used. The @c prototype is obtained by
 * loading metadata from the first file once at construction time.
 *
 * @note load() may be called concurrently from a background thread. A fresh DataReader is created
 * for each load call, guarded by an internal mutex, so the loader is safe to use with
 * TemporalVolume's prefetch mechanism.
 *
 * @see TemporalVolume, VolumeLoader
 */
class IVW_MODULE_BASE_API FileSequenceLoader : public VolumeLoader {
public:
    /**
     * @param paths     one file per frame, must be non-empty
     * @param times     physical time value per frame; either empty (use 0,1,2,…) or same size as
     *                  @p paths
     * @param factory   the data reader factory used to create Volume readers, must not be null
     * @param extension optional file extension used to select the reader, otherwise inferred from
     *                  the file path
     *
     * @throws Exception if @p paths is empty, @p factory is null, or @p times has a mismatching
     * size
     * @throws DataReaderException if no reader is found for the first file or it fails to load
     */
    FileSequenceLoader(std::vector<std::filesystem::path> paths, std::vector<Seconds> times,
                       DataReaderFactory* factory, FileExtension extension = {});

    virtual std::shared_ptr<Volume> load(size_t index, std::shared_ptr<Volume> reuse) override;
    virtual size_t size() const override;
    virtual std::span<const Seconds> times() const override;
    virtual VolumeConfig prototype() const override;

private:
    std::shared_ptr<Volume> readFile(size_t index) const;

    std::vector<std::filesystem::path> paths_;
    std::vector<Seconds> times_;
    DataReaderFactory* factory_;
    FileExtension extension_;
    VolumeConfig prototype_;
    mutable std::mutex readerMutex_;
};

}  // namespace inviwo
