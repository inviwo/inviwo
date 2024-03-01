/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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
#include <vector>       // for vector

namespace inviwo {

/**
 * \ingroup dataio
 *\brief Reader for *.dat files
 *
 *  The following tags are supported:
 *   - __RawFile__      The name of the raw data file, should be in the same directory (Mandatory).
 *   - __Sequences__    Number of Volume in the raw file (Optional: default 1)
 *   - __ByteOrder__    The byte order in the raw data file. (Optional, LittleEndian|BigEndian,
 *                      default: LittleEndian).
 *   - __ByteOffset__   Offset in to the raw file (Optional, default: 0)
 *   - __DataOffset__   Offset in byte to where the data starts
 *   - __Resolution | Dimension__ the size of the data grid: nx,ny,nz (Mandatory).
 *   - __Format__       The type of values in the raw file. (Mandatory). The valid formats are:
                        FLOAT16, FLOAT32, FLOAT64, INT8, INT16, INT32, INT64, UINT8, UINT16, UINT32,
                        UINT64, Vec2FLOAT16, Vec2FLOAT32, Vec2FLOAT64, Vec2INT8, Vec2INT16,
                        Vec2INT32, Vec2INT64, Vec2UINT8, Vec2UINT16, Vec2UINT32, Vec2UINT64,
                        Vec3FLOAT16, Vec3FLOAT32, Vec3FLOAT64, Vec3INT8, Vec3INT16, Vec3INT32,
                        Vec3INT64, Vec3UINT8, Vec3UINT16, Vec3UINT32, Vec3UINT64, Vec4FLOAT16,
                        Vec4FLOAT32, Vec4FLOAT64, Vec4INT8, Vec4INT16, Vec4INT32, Vec4INT64,
                        Vec4UINT8, Vec4UINT16, Vec4UINT32, Vec4UINT64
 *   - __Spacing | SliceThickness__ The size of the voxels in the data. (Optional)
 *   - __BasisVector(1|2|3)__ Defines a coordinate system for the data. (Optional, overrides
 *                            spacing, default: 2*IdentityMatrix);
 *   - __Offset__       Offsets the basis vectors in space. (Optional, defaults to center the data
 *                      at the origin)
 *   - __WorldVector(1|2|3|4)__ Defines a world transformation matrix that is applied last to orient
 *                              the data in world space. (Optional, default: IdentityMatrix)
 *   - __DatFile__      Relative path to other file to create a VolumeSequence from.
 *   - __DataRange__    DataRange of volume (Optional, defaults to the min/max of the first volume)
 *   - __ValueRange__   ValueRange of volume (Optional, defaults to the DataRange)
 *   - __ValueName__    Name of Value domain (Optional, defaults to "")
 *   - __ValueUnit__    Value Unit (Optional, defaults to Unit{})
 *   - __SwizzleMask__  Data Channel swizzle mask @see SwizzleMask (Optional, defaults to rgba)
 *   - __Interpolation__ Interpolation mode @see InterpolationType (Optional, defaults to Linear)
 *   - __Wrapping__     Wrapping mode @see Wrapping3D (Optional defaults to clampAll
 *   - __AxisNames__    Space separated axes names (Optional, defaults to "")
 *   - __Axis(1|2|3)Name__ Axis name (Optional, defaults to "")
 *   - __AxisUnits__    Space separated axes units (Optional, defaults to Unit{})
 *   - __Axis(1|2|3)Unit__ Axis unit (Optional, defaults to Unit{})
 *
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
class IVW_MODULE_BASE_API DatVolumeSequenceReader : public DataReaderType<VolumeSequence> {
public:
    DatVolumeSequenceReader();
    DatVolumeSequenceReader(const DatVolumeSequenceReader& rhs);
    DatVolumeSequenceReader& operator=(const DatVolumeSequenceReader& that);
    virtual DatVolumeSequenceReader* clone() const override;
    virtual ~DatVolumeSequenceReader() = default;

    virtual std::shared_ptr<VolumeSequence> readData(
        const std::filesystem::path& filePath) override;

private:
    bool enableLogOutput_;
};

}  // namespace inviwo
