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

#ifndef IVW_VOLUMEGRADIENT_H
#define IVW_VOLUMEGRADIENT_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/volume/volume.h>

namespace inviwo {

namespace util {
//namespace detail {
//
//struct VolumeGradientDispatcher {
//    using type = std::shared_ptr<Volume>;
//
//    template <typename T>
//    std::shared_ptr<Volume> dispatch(const Volume* volume) {
//        using dataType = typename T::type;
//        using primitive = typename T::primitive;
//
//        auto newVolume = std::make_shared<Volume>(volume->getDimensions());
//        newVolume->setModelMatrix(volume->getModelMatrix());
//        newVolume->setWorldMatrix(volume->getWorldMatrix());
//
//        util::forEachVoxel(volume->getRepresentation<VolumeRAM>(), [](const size3_t &pos) {
//            
//        });
//            
//        return newVolume;
//    }
//};
//}

IVW_MODULE_BASE_API std::shared_ptr<Volume> gradientVolume(std::shared_ptr<const Volume> volume, int channel);
}
}  // namespace

#endif  // IVW_VOLUMEGRADIENT_H
