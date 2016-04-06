/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#ifndef IVW_LAYERMINMAX_H
#define IVW_LAYERMINMAX_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/image/layerram.h>

namespace inviwo {

namespace util {

namespace detail {

struct LayerMinMaxDispatcher {
    using type = std::pair<dvec4, dvec4>;

    template <typename T>
    std::pair<dvec4, dvec4> dispatch(const LayerRAM* layer) {
        using dataType = typename T::type;
        auto data = static_cast<const dataType*>(layer->getData());
        auto df = layer->getDataFormat();

// Visual studio warns here even with the static casts, bug?
#include <warn/push>
#include <warn/ignore/conversion>
        auto minV = static_cast<dataType>(df->getMax());
        auto maxV = static_cast<dataType>(df->getLowest());
#include <warn/pop>

        const auto dim = layer->getDimensions();
        const auto size = dim.x * dim.y;

        for (size_t i = 0; i < size; i++) {
            auto v = data[i];

            if (util::all(v != v + dataType(1))) {
                minV = glm::min(minV, v);
                maxV = glm::max(maxV, v);
            }
        }

        return {util::glm_convert<dvec4>(minV), util::glm_convert<dvec4>(maxV)};
    }
};
}  // namespace

IVW_MODULE_BASE_API std::pair<dvec4, dvec4> layerMinMax(const LayerRAM* layer);

}  // namespace

}  // namespace

#endif  // IVW_LAYERMINMAX_H
