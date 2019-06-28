/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <inviwo/core/datastructures/volume/volumedisk.h>

namespace inviwo {

VolumeDisk::VolumeDisk(size3_t dimensions, const DataFormatBase* format,
                       const SwizzleMask& swizzleMask)
    : VolumeRepresentation(format)
    , DiskRepresentation<VolumeRepresentation>()
    , dimensions_(dimensions)
    , swizzleMask_(swizzleMask) {}

VolumeDisk::VolumeDisk(std::string srcFile, size3_t dimensions, const DataFormatBase* format,
                       const SwizzleMask& swizzleMask)
    : VolumeRepresentation(format)
    , DiskRepresentation<VolumeRepresentation>(srcFile)
    , dimensions_(dimensions)
    , swizzleMask_(swizzleMask) {}

VolumeDisk* VolumeDisk::clone() const { return new VolumeDisk(*this); }

std::type_index VolumeDisk::getTypeIndex() const { return std::type_index(typeid(VolumeDisk)); }

void VolumeDisk::setDimensions(size3_t) {
    throw Exception("Can not set dimension of a Volume Disk", IVW_CONTEXT);
}

const size3_t& VolumeDisk::getDimensions() const { return dimensions_; }

void VolumeDisk::setSwizzleMask(const SwizzleMask& mask) { swizzleMask_ = mask; }

SwizzleMask VolumeDisk::getSwizzleMask() const { return swizzleMask_; }

}  // namespace inviwo
