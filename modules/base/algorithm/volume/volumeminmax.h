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

#ifndef IVW_VOLUMEMINMAX_H
#define IVW_VOLUMEMINMAX_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/volume/volumeram.h>

namespace inviwo {
namespace util {
namespace detail {

struct VolumeMinMaxDispatcher {
    using type = std::pair<dvec4, dvec4>;

    template <typename T>
    std::pair<dvec4, dvec4> dispatch(const VolumeRAM* volume) {
        using dataType = typename T::type;
        using primitive = typename T::primitive;
        auto data = static_cast<const dataType*>(volume->getData());
        auto df = volume->getDataFormat();

#include <warn/push>
#include <warn/ignore/conversion>  // Visual Studio 2015 complains about type conversion despite static_cast
        dataType minV(static_cast<primitive>(df->getMax()));
        dataType maxV(static_cast<primitive>(df->getLowest()));
#include <warn/pop>

        const auto dim = volume->getDimensions();
        const auto size = dim.x * dim.y * dim.z;
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

IVW_MODULE_BASE_API std::pair<dvec4, dvec4> volumeMinMax(const VolumeRAM* volume);

}  // namespace

}  // namespace

#endif // IVW_VOLUMEMINMAX_H

