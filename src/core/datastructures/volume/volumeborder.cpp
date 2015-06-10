/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <inviwo/core/datastructures/volume/volumeborder.h>

namespace inviwo {

bool VolumeBorders::operator!=(const VolumeBorders& vb) const {
    return (llf != vb.llf || urb != vb.urb);
}

bool VolumeBorders::operator==(const VolumeBorders& vb) const {
    return (llf == vb.llf && urb == vb.urb);
}

VolumeBorders::VolumeBorders(const size3_t& llfBorder, const size3_t& urbBorder)
    : llf(llfBorder), urb(urbBorder) {}

VolumeBorders::VolumeBorders(size_t front, size_t back, size_t left, size_t right, size_t lower,
                             size_t upper)
    : llf(size3_t(front, left, lower)), urb(size3_t(back, right, upper)) {}

VolumeBorders::VolumeBorders()
    : llf(size3_t(0, 0, 0)), urb(size3_t(0, 0, 0)), numVoxels(0), hasBorder(false) {}

}  // namespace
