/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2018 Inviwo Foundation
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

#ifndef IVW_RAWVOLUMEREADER_H
#define IVW_RAWVOLUMEREADER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumedisk.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/volumedatareaderdialog.h>

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

    virtual void setParameters(const DataFormatBase* format, ivec3 dimensions, bool littleEndian,
                               DataMapper dataMapper, size_t dataOffset = 0u);

    virtual std::shared_ptr<Volume> readData(const std::string& filePath) override;
    virtual std::shared_ptr<Volume> readData(const std::string& filePath,
                                             MetaDataOwner* metadata) override;

    bool haveReadLittleEndian() const { return littleEndian_; }
    const DataFormatBase* getFormat() const { return format_; }

private:
    std::string rawFile_;
    bool littleEndian_;
    size3_t dimensions_;
    vec3 spacing_;
    const DataFormatBase* format_;
    DataMapper dataMapper_;
    size_t dataOffset_;
    bool parametersSet_;
};

}  // namespace inviwo

#endif  // IVW_RAWVOLUMEREADER_H
