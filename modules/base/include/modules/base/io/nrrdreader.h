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

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/io/datareader.h>

#include <memory>

namespace inviwo {

/**
 * @ingroup dataio
 * @brief Reader for NRRD (Nearly Raw Raster Data) files as 2D layers.
 *
 * Reads 2D NRRD files (.nrrd with inline data, .nhdr with detached data).
 * Supports raw, gzip, and bzip2 encodings. Multi-component data (e.g., RGB)
 * is supported via the NRRD "kinds" field.
 *
 * @see https://teem.sourceforge.net/nrrd/format.html
 */
class IVW_MODULE_BASE_API NrrdLayerReader : public DataReaderType<Layer> {
public:
    NrrdLayerReader();
    NrrdLayerReader(const NrrdLayerReader&) = default;
    NrrdLayerReader(NrrdLayerReader&&) noexcept = default;
    NrrdLayerReader& operator=(const NrrdLayerReader&) = default;
    NrrdLayerReader& operator=(NrrdLayerReader&&) noexcept = default;
    virtual NrrdLayerReader* clone() const override;
    virtual ~NrrdLayerReader() = default;

    virtual std::shared_ptr<Layer> readData(const std::filesystem::path& filePath) override;
};

/**
 * @ingroup dataio
 * @brief Reader for NRRD (Nearly Raw Raster Data) files as 3D volumes.
 *
 * Reads 3D NRRD files (.nrrd with inline data, .nhdr with detached data).
 * Supports raw, gzip, and bzip2 encodings. Multi-component data is supported
 * via the NRRD "kinds" field.
 *
 * Spatial metadata is derived from "spacings", "space directions", and
 * "space origin" NRRD header fields.
 *
 * @see https://teem.sourceforge.net/nrrd/format.html
 */
class IVW_MODULE_BASE_API NrrdVolumeReader : public DataReaderType<Volume> {
public:
    NrrdVolumeReader();
    NrrdVolumeReader(const NrrdVolumeReader&) = default;
    NrrdVolumeReader(NrrdVolumeReader&&) noexcept = default;
    NrrdVolumeReader& operator=(const NrrdVolumeReader&) = default;
    NrrdVolumeReader& operator=(NrrdVolumeReader&&) noexcept = default;
    virtual NrrdVolumeReader* clone() const override;
    virtual ~NrrdVolumeReader() = default;

    virtual std::shared_ptr<Volume> readData(const std::filesystem::path& filePath) override;
};

/**
 * @ingroup dataio
 * @brief Reader for NRRD (Nearly Raw Raster Data) files as volume sequences.
 *
 * Reads 3D and 4D NRRD files (.nrrd with inline data, .nhdr with detached data)
 * as VolumeSequence. For 4D data, the last axis is treated as time steps.
 * Supports raw, gzip, and bzip2 encodings.
 *
 * @see https://teem.sourceforge.net/nrrd/format.html
 */
class IVW_MODULE_BASE_API NrrdVolumeSequenceReader : public DataReaderType<VolumeSequence> {
public:
    NrrdVolumeSequenceReader();
    NrrdVolumeSequenceReader(const NrrdVolumeSequenceReader&) = default;
    NrrdVolumeSequenceReader(NrrdVolumeSequenceReader&&) noexcept = default;
    NrrdVolumeSequenceReader& operator=(const NrrdVolumeSequenceReader&) = default;
    NrrdVolumeSequenceReader& operator=(NrrdVolumeSequenceReader&&) noexcept = default;
    virtual NrrdVolumeSequenceReader* clone() const override;
    virtual ~NrrdVolumeSequenceReader() = default;

    virtual std::shared_ptr<VolumeSequence> readData(
        const std::filesystem::path& filePath) override;
};

}  // namespace inviwo
