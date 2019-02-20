/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_VOLUMEUTILS_H
#define IVW_VOLUMEUTILS_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

#include <tuple>

namespace inviwo {

class Volume;

namespace util {

bool IVW_CORE_API hasMargins(const std::shared_ptr<const Volume> &volume);

bool IVW_CORE_API isBricked(const std::shared_ptr<const Volume> &volume);

size3_t IVW_CORE_API getBrickDimensions(const std::shared_ptr<const Volume> &volume);

/**
 * \brief return the margins of a volume, in normalized texture coordinates [0,1]
 *
 * @return pair of margins from the bottom left corner and the top right corner
 */
std::pair<vec3, vec3> IVW_CORE_API getVolumeMargins(const std::shared_ptr<const Volume> &volume);

/**
 * \brief returns the true volume dimensions considering volume margins and bricking
 *
 * @return true volume dimensions
 */
size3_t IVW_CORE_API getVolumeDimensions(const std::shared_ptr<const Volume> &volume);

/**
 * \brief calculates the volume of a single voxel, taking the basis and the dimensions into account
 * The units of the result is in the unit as the basis vectors ^3
 *
 * @return volume of one voxel
 */
double IVW_CORE_API voxelVolume(const Volume &volume);

}  // namespace util

}  // namespace inviwo

#endif  // IVW_VOLUMEUTILS_H
