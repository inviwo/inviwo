/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2022 Inviwo Foundation
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

#include <modules/cimg/cimgmoduledefine.h>  // for IVW_MODULE_CIMG_API

#include <inviwo/core/datastructures/diskrepresentation.h>           // for DiskRepresentationLo...
#include <inviwo/core/datastructures/volume/volume.h>                // for DataReaderType
#include <inviwo/core/datastructures/volume/volumerepresentation.h>  // for VolumeRepresentation
#include <inviwo/core/io/datareader.h>                               // for DataReaderType

#include <memory>       // for shared_ptr
#include <string>       // for string
#include <string_view>  // for string_view

namespace inviwo {

class IVW_MODULE_CIMG_API TIFFStackVolumeReader : public DataReaderType<Volume> {
public:
    TIFFStackVolumeReader();
    virtual TIFFStackVolumeReader* clone() const override;
    virtual ~TIFFStackVolumeReader() = default;

    virtual std::shared_ptr<Volume> readData(std::string_view filePath) override;
};

class IVW_MODULE_CIMG_API TIFFStackVolumeRAMLoader
    : public DiskRepresentationLoader<VolumeRepresentation> {
public:
    TIFFStackVolumeRAMLoader(std::string_view sourceFile);
    virtual TIFFStackVolumeRAMLoader* clone() const override;
    virtual ~TIFFStackVolumeRAMLoader() = default;

    virtual std::shared_ptr<VolumeRepresentation> createRepresentation(
        const VolumeRepresentation& src) const override;
    virtual void updateRepresentation(std::shared_ptr<VolumeRepresentation> dest,
                                      const VolumeRepresentation& src) const override;

private:
    std::string sourceFile_;
};

}  // namespace inviwo
