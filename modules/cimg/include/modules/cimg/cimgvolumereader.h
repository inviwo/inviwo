/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2022 Inviwo Foundation
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

#include <modules/cimg/cimgmoduledefine.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/datastructures/volume/volumedisk.h>
#include <inviwo/core/io/datareader.h>

namespace inviwo {

/**
 * \ingroup dataio
 */
class IVW_MODULE_CIMG_API CImgVolumeReader : public DataReaderType<Volume> {
public:
    CImgVolumeReader();
    CImgVolumeReader(const CImgVolumeReader& rhs) = default;
    CImgVolumeReader& operator=(const CImgVolumeReader& that) = default;
    virtual CImgVolumeReader* clone() const override;
    virtual ~CImgVolumeReader() = default;

    virtual std::shared_ptr<Volume> readData(std::string_view filePath) override;

protected:
    void printMetaInfo(const MetaDataOwner&, std::string_view) const;
};

class IVW_MODULE_CIMG_API CImgVolumeRAMLoader
    : public DiskRepresentationLoader<VolumeRepresentation> {
public:
    CImgVolumeRAMLoader(std::string_view sourceFile);
    virtual CImgVolumeRAMLoader* clone() const override;
    virtual ~CImgVolumeRAMLoader() = default;
    virtual std::shared_ptr<VolumeRepresentation> createRepresentation(
        const VolumeRepresentation& src) const override;
    virtual void updateRepresentation(std::shared_ptr<VolumeRepresentation> dest,
                                      const VolumeRepresentation& src) const override;

private:
    std::string sourceFile_;
};

}  // namespace inviwo
