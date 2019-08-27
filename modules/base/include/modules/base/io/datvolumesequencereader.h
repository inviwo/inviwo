/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_DATVOLUMESEQUENCEREADER_H
#define IVW_DATVOLUMESEQUENCEREADER_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/io/datareader.h>

namespace inviwo {

/**
 * \ingroup dataio
 *\brief Reader for *.dat files
 *
 *  The following tags are supported:
 *   - __Rawfile__ The name of the raw data file, should be in the same directory (Mandatory).
 *   - __ByteOrder__ the byte order in the raw data file. (Optional, LittleEndian|BigEndian,
 *     default: LittleEndian).
 *   - __DataOffset__  offset in byte to where the data starts
 *   - __Resolution | Dimension__ The size of the data grid: nx,ny,nz (Mandatory).
 *   - __Format__ The type of values in the raw file. (Mandatory)
 *   - __Spacing | SliceThickness__ The size of the voxels in the data. (Optional)
 *   - __BasisVector(1|2|3)__ Defines a coordinate system for the data. (Optional, overides spacing,
 *     default: 2*IdentityMatrix);
 *   - __Offset__ Offsets the basis vectors in space. (Optional, defaults to center the data at the
 *     origin)
 *   - __WorldVector(1|2|3|4)__ Defines a world transformation matrix that is applied last to orient
 *     the data in world space. (Optional, default: IdentityMatrix)
 *   - __DatFile__ Relative path to other file to create a VolumeSequence from
 *  The tag names are case insensitive and should always be followed by a ":"
 *  Anything after a '#' will be considered a comment.
 *
 * Supports reading VolumeSequence (for example time-varying volume data) by specifying multiple
 *.dat files.
 *
 * Example:
 *     + Datfile: sequence0.dat
 *     + Datfile: sequence1.dat
 *     + Datfile: sequence2.dat
 */
class IVW_MODULE_BASE_API DatVolumeSequenceReader
    : public DataReaderType<std::vector<std::shared_ptr<Volume>>> {
public:
    using VolumeSequence = std::vector<std::shared_ptr<Volume>>;

    DatVolumeSequenceReader();
    DatVolumeSequenceReader(const DatVolumeSequenceReader& rhs);
    DatVolumeSequenceReader& operator=(const DatVolumeSequenceReader& that);
    virtual DatVolumeSequenceReader* clone() const override;
    virtual ~DatVolumeSequenceReader() = default;

    virtual std::shared_ptr<VolumeSequence> readData(const std::string& filePath) override;

private:
    bool enableLogOutput_;
};

}  // namespace inviwo

#endif  // IVW_DATVOLUMESEQUENCEREADER_H
