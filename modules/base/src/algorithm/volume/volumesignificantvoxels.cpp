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

#include <modules/base/algorithm/volume/volumesignificantvoxels.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>

#include <algorithm>

namespace inviwo {

size_t util::volumeSignificantVoxels(const VolumeRAM* volume, IgnoreSpecialValues ignore) {
    return volume->dispatch<size_t>([&ignore](auto vr) -> size_t {
        using ValueType = util::PrecisionValueType<decltype(vr)>;

        const auto data = vr->getDataTyped();
        const auto dim = vr->getDimensions();
        const auto size = dim.x * dim.y * dim.z;

        if (ignore == IgnoreSpecialValues::Yes) {
            return std::count_if(data, data + size, [](const auto& v) {
                return util::all(v != v + ValueType(1)) && util::any(v != ValueType(0));
            });
        } else {
            return std::count_if(data, data + size,
                                 [](const auto& v) { return util::any(v != ValueType(0)); });
        }
    });
}

}  // namespace inviwo
