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

#include <inviwo/core/datastructures/volume/volumeramprecision.h>

namespace inviwo {

struct VolumeRamCreationDispatcher {
    using type = std::shared_ptr<VolumeRAM>;
    template <typename Result, typename T>
    std::shared_ptr<VolumeRAM> operator()(void* dataPtr, const size3_t& dimensions,
                                          const SwizzleMask& swizzleMask) {
        using F = typename T::type;
        return std::make_shared<VolumeRAMPrecision<F>>(static_cast<F*>(dataPtr), dimensions,
                                                       swizzleMask);
    }
};

std::shared_ptr<VolumeRAM> createVolumeRAM(const size3_t& dimensions, const DataFormatBase* format,
                                           void* dataPtr, const SwizzleMask& swizzleMask) {
    VolumeRamCreationDispatcher disp;
    return dispatching::dispatch<std::shared_ptr<VolumeRAM>, dispatching::filter::All>(
        format->getId(), disp, dataPtr, dimensions, swizzleMask);
}

}  // namespace inviwo
