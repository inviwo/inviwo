/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2022 Inviwo Foundation
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

#include <modules/base/algorithm/dataminmax.h>

#include <inviwo/core/datastructures/buffer/buffer.h>                   // for BufferBase
#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAM
#include <inviwo/core/datastructures/image/layer.h>                     // for Layer
#include <inviwo/core/datastructures/image/layerram.h>                  // for LayerRAM
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume
#include <inviwo/core/datastructures/volume/volumeram.h>                // for VolumeRAM
#include <inviwo/core/util/glmvec.h>                                    // for dvec4
#include <modules/base/algorithm/algorithmoptions.h>                    // for IgnoreSpecialValues

#include <memory>                                                       // for unique_ptr
#include <unordered_set>                                                // for unordered_set

#include <half/half.hpp>                                                // for operator<

namespace inviwo {

std::pair<dvec4, dvec4> util::volumeMinMax(const VolumeRAM* volume, IgnoreSpecialValues ignore) {
    return volume->dispatch<std::pair<dvec4, dvec4>>([&ignore](auto vr) -> std::pair<dvec4, dvec4> {
        const auto dim = vr->getDimensions();
        return dataMinMax(vr->getDataTyped(), dim.x * dim.y * dim.z, ignore);
    });
}

std::pair<dvec4, dvec4> util::layerMinMax(const LayerRAM* layer, IgnoreSpecialValues ignore) {
    return layer->dispatch<std::pair<dvec4, dvec4>>([&ignore](auto lr) -> std::pair<dvec4, dvec4> {
        const auto dim = lr->getDimensions();
        return dataMinMax(lr->getDataTyped(), dim.x * dim.y, ignore);
    });
}

std::pair<dvec4, dvec4> util::bufferMinMax(const BufferRAM* buffer, IgnoreSpecialValues ignore) {
    return buffer->dispatch<std::pair<dvec4, dvec4>>([&ignore](auto br) -> std::pair<dvec4, dvec4> {
        return dataMinMax(br->getDataContainer().data(), br->getSize(), ignore);
    });
}

std::pair<dvec4, dvec4> util::volumeMinMax(const Volume* volume, IgnoreSpecialValues ignore) {
    return util::volumeMinMax(volume->getRepresentation<VolumeRAM>(), ignore);
}

std::pair<dvec4, dvec4> util::layerMinMax(const Layer* layer, IgnoreSpecialValues ignore) {
    return util::layerMinMax(layer->getRepresentation<LayerRAM>(), ignore);
}

std::pair<dvec4, dvec4> util::bufferMinMax(const BufferBase* buffer, IgnoreSpecialValues ignore) {
    return util::bufferMinMax(buffer->getRepresentation<BufferRAM>(), ignore);
}

}  // namespace inviwo
