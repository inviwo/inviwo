/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/volume/volumemoduledefine.h>

#include <memory>
#include <vector>

namespace inviwo {

class Volume;

/**
 * Remap all voxels of @p volume by mapping values from @p src to @p dst.
 * @param[in,out] volume  voxels of this scalar volume will be remapped
 * @param src   list of source indices
 * @param dst   list of destination indices matching @p src
 * @param missingValue    assigned if a value cannot be found in @p src and @p useMissingValue is
 *                        set
 * @param useMissingValue if true, all values not included in @p src are mapped to @p missingValue
 * @throw Exception if @p src and @p dst have different sizes
 * @throw Exception if @p src contains duplicated values
 * @pre @p volume must be a scalar volume
 * @pre @p src and @p dst must have the same length
 * @pre @p src must not contain duplicates
 * @pre @p dst may contain duplicates
 * @post @p volume only contains voxels with values found in @p dst
 */
void IVW_MODULE_VOLUME_API remap(std::shared_ptr<Volume>& volume, const std::vector<int>& src,
                                 const std::vector<int>& dst, int missingValue,
                                 bool useMissingValue);

}  // namespace inviwo
