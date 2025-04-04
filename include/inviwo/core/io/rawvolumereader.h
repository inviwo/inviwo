/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumedisk.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/volumedatareaderdialog.h>

#include <memory>
#include <string>

namespace inviwo {
class MetaDataOwner;

/**
 * \ingroup dataio
 */
class IVW_CORE_API RawVolumeReader : public DataReaderType<Volume> {
public:
    RawVolumeReader();
    RawVolumeReader(const RawVolumeReader& rhs);
    RawVolumeReader& operator=(const RawVolumeReader& that);
    virtual RawVolumeReader* clone() const override;
    virtual ~RawVolumeReader() = default;

    virtual std::shared_ptr<Volume> readData(const std::filesystem::path& filePath) override;
    virtual std::shared_ptr<Volume> readData(const std::filesystem::path& filePath,
                                             MetaDataOwner* metadata) override;

    const DataFormatBase* getFormat() const { return format_; }

private:
    std::filesystem::path rawFile_;
    ByteOrder byteOrder_;
    size3_t dimensions_;
    vec3 spacing_;
    const DataFormatBase* format_;
    DataMapper dataMapper_;
    size_t byteOffset_;
    bool parametersSet_;
    Compression compression_;
};

}  // namespace inviwo
