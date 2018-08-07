/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#include <modules/base/algorithm/image/layerramsubset.h>

#include <inviwo/core/util/formatdispatching.h>

namespace inviwo {

std::shared_ptr<LayerRAM> LayerRAMSubSet::apply(const LayerRepresentation *in, ivec2 offset,
                                                size2_t extent, bool clampBorderOutsideImage,
                                                const DataFormatBase *dstFormat) {
    if (dstFormat) {
#include <warn/push>
#include <warn/ignore/switch-enum>
        switch (dstFormat->getId()) {
            case DataFormatId::Vec4UInt8: {
                detail::LayerRAMSubSetConvertDispatcher<DataVec4UInt8::type> disp;
                return dispatching::dispatch<std::shared_ptr<LayerRAM>, dispatching::filter::All>(
                    dstFormat->getId(), disp, in, offset, extent, clampBorderOutsideImage);
            }
            default: {
                detail::LayerRAMSubSetDispatcher disp;
                return dispatching::dispatch<std::shared_ptr<LayerRAM>, dispatching::filter::All>(
                    dstFormat->getId(), disp, in, offset, extent, clampBorderOutsideImage);
            }
        }
#include <warn/pop>
    } else {
        detail::LayerRAMSubSetDispatcher disp;
        return dispatching::dispatch<std::shared_ptr<LayerRAM>, dispatching::filter::All>(
            dstFormat->getId(), disp, in, offset, extent, clampBorderOutsideImage);
    }
}

}  // namespace inviwo
