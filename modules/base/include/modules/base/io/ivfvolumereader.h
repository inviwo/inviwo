/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/datastructures/volume/volume.h>  // for DataReaderType
#include <inviwo/core/io/datareader.h>                 // for DataReaderType

#include <memory>       // for shared_ptr
#include <string_view>  // for string_view

namespace inviwo {

/**
 * \ingroup dataio
 */
class IVW_MODULE_BASE_API IvfVolumeReader : public DataReaderType<Volume> {
public:
    IvfVolumeReader();
    IvfVolumeReader(const IvfVolumeReader& rhs) = default;
    IvfVolumeReader(IvfVolumeReader&& rhs) = default;
    IvfVolumeReader& operator=(const IvfVolumeReader& that) = default;
    IvfVolumeReader& operator=(IvfVolumeReader&& that) = default;
    virtual IvfVolumeReader* clone() const override;
    virtual ~IvfVolumeReader() = default;

    virtual std::shared_ptr<Volume> readData(const std::filesystem::path& filePath) override;
};

/**
 * \ingroup dataio
 */
class IVW_MODULE_BASE_API IvfVolumeSequenceReader : public DataReaderType<VolumeSequence> {
public:
    IvfVolumeSequenceReader();
    IvfVolumeSequenceReader(const IvfVolumeSequenceReader& rhs) = default;
    IvfVolumeSequenceReader(IvfVolumeSequenceReader&& rhs) = default;
    IvfVolumeSequenceReader& operator=(const IvfVolumeSequenceReader& that) = default;
    IvfVolumeSequenceReader& operator=(IvfVolumeSequenceReader&& that) = default;
    virtual IvfVolumeSequenceReader* clone() const override;
    virtual ~IvfVolumeSequenceReader() = default;

    virtual std::shared_ptr<VolumeSequence> readData(
        const std::filesystem::path& filePath) override;
};

}  // namespace inviwo
