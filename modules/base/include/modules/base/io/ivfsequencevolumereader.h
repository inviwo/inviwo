/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifndef IVW_IVFSEQUENCEVOLUMEREADER_H
#define IVW_IVFSEQUENCEVOLUMEREADER_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <modules/base/io/ivfvolumereader.h>

namespace inviwo {
/**
 * \ingroup dataio
 * \brief Reader for *.ivfs sequnce files
 *
 * Supports reading a volume sequence from disk.
 *
 * The expected structure of the ivfs sequence files is:
 * \verbatim
<?xml version="1.0" ?>
<InviwoWorkspace version="2">
    <volumes>
        <volume content="./relative/path/to/volume00.ivf" />
        <volume content="./relative/path/to/volume01.ivf" />
        ...
        <volume content="./relative/path/to/volumeNN.ivf" />
    </volumes>
</InviwoWorkspace>
 * \endverbatim
 *
 * @see inviwo::IvfSequenceVolumeWriter
 */

class IVW_MODULE_BASE_API IvfSequenceVolumeReader : public DataReaderType<VolumeSequence> {
public:
    IvfSequenceVolumeReader();
    IvfSequenceVolumeReader(const IvfSequenceVolumeReader& rhs) = default;
    IvfSequenceVolumeReader& operator=(const IvfSequenceVolumeReader& that) = default;
    virtual IvfSequenceVolumeReader* clone() const override {
        return new IvfSequenceVolumeReader(*this);
    }
    virtual ~IvfSequenceVolumeReader() = default;

    /**
     * Read a ivfs volume sequence from disk
     *
     * @param filePath path the the ivfs file
     * @return The resulting sequence of volumes
     * @see inviwo::IvfSequenceVolumeWriter::writeData(const VolumeSequence*, std::string ,
     * std::string,std::string)
     */
    virtual std::shared_ptr<VolumeSequence> readData(const std::string& filePath) override;

private:
    IvfVolumeReader reader_;
};

}  // namespace inviwo

#endif  // IVW_IVFSEQUENCEVOLUMEREADER_H
