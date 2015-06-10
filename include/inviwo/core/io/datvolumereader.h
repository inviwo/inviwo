/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_DATVOLUMEREADER_H
#define IVW_DATVOLUMEREADER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/io/datareader.h>

namespace inviwo {

/** \brief Reader for *.dat files
 *
 *  The following tags are supported:
 *   - Rawfile :: The name of the raw data file, should be in the same directory (Mandatory).
 *   - ByteOrder :: the byte order in the raw data file. (Optional, LittleEndian|BigEndian,
 *     defualt: LittleEndian).
 *   - Resolution | Dimension ::The size of the data grid: nx,ny,nz (Mandatory).
 *   - Format :: The type of values in the raw file. (Mandatory)
 *   - Spacing | SliceThickness :: The size of the voxels in the data. (Optional)
 *   - BasisVector(1|2|3) :: Defines a coordinate system for the data. (Optional, overides spacing,
 *     default: 2*IdentityMatrix);
 *   - Offset :: Offsets the basisvecors in space. (Optional, defaults to center the data on origo)
 *   - WorldVector(1|2|3|4) :: Defines a world transformation matrix that is applied last to orient
 *     the data in world space. (Optional, default: IdentityMatrix)
 *
 *  The tag names are case insensitive and should always be followed by a ":"
 */
class IVW_CORE_API DatVolumeReader : public DataReaderType<Volume> {
public:
    DatVolumeReader();
    DatVolumeReader(const DatVolumeReader& rhs);
    DatVolumeReader& operator=(const DatVolumeReader& that);
    virtual DatVolumeReader* clone() const;
    virtual ~DatVolumeReader() {}

    virtual Volume* readMetaData(const std::string filePath);
    virtual void* readData() const;
    virtual void readDataInto(void* dest) const;

private:
    std::string rawFile_;
    size_t filePos_;
    bool littleEndian_;
    size3_t dimensions_;
    const DataFormatBase* format_;
};

}  // namespace

#endif  // IVW_DATVOLUMEREADER_H
