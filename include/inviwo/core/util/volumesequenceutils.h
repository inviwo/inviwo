/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/volume/volume.h>

#include <vector>
#include <memory>

namespace inviwo::util {

IVW_CORE_API bool hasTimestamps(const VolumeSequence& seq, bool checkfirstonly = true);

IVW_CORE_API std::pair<double, double> getTimestampRange(const VolumeSequence& seq,
                                                         bool sorted = true);

IVW_CORE_API bool isSorted(const VolumeSequence& seq);
VolumeSequence IVW_CORE_API sortSequence(const VolumeSequence& seq);

IVW_CORE_API std::pair<std::shared_ptr<const Volume>, std::shared_ptr<const Volume>>
getVolumesForTimestep(const VolumeSequence& seq, double t, bool sorted = true);

IVW_CORE_API bool hasTimestamp(const std::shared_ptr<const Volume>& vol);
IVW_CORE_API double getTimestamp(const std::shared_ptr<const Volume>& vol);

/**
 * Convenience data structure for checking whether all volumes within a VolumeSequence share the
 * same format, basis, worldTransform, ...
 */
struct IVW_CORE_API SharedSequenceData {
    explicit SharedSequenceData(const VolumeSequence& seq);

    bool format = true;
    bool basis = true;
    bool worldTransform = true;
    bool dimensions = true;
    bool dataMap = true;
    bool axes = true;
};

}  // namespace inviwo::util
