/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/hdf5gl/layerhdftoglconverter.h>

#include <inviwo/core/datastructures/image/layer.h>
#include <modules/opengl/storageformat.h>
#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/texture/pixelunpackbuffer.h>

#include <cstddef>
#include <span>

namespace inviwo::hdf5 {

std::shared_ptr<LayerGL> LayerHDF2GLConverter::createFrom(
    std::shared_ptr<const LayerHDF> source) const {
    auto destination = std::make_shared<LayerGL>(source->getDimensions(), source->getLayerType(),
                                                 source->getDataFormat(), source->getSwizzleMask(),
                                                 source->getInterpolation(), source->getWrapping(),
                                                 utilgl::getStorageFormat(source->getOwner()));

    auto& texture = *destination->getTexture();
    utilgl::mapAndUploadToTexture(texture, true, [&](std::span<std::byte> dst) {
        source->readInto(dst, texture.getDataFormat());
    });
    return destination;
}

void LayerHDF2GLConverter::update(std::shared_ptr<const LayerHDF> source,
                                  std::shared_ptr<LayerGL> destination) const {
    destination->setDimensions(source->getDimensions());
    destination->setSwizzleMask(source->getSwizzleMask());
    destination->setInterpolation(source->getInterpolation());
    destination->setWrapping(source->getWrapping());

    auto& texture = *destination->getTexture();
    utilgl::mapAndUploadToTexture(texture, false, [&](std::span<std::byte> dst) {
        source->readInto(dst, texture.getDataFormat());
    });
    destination->setLossy(utilgl::isLossy(source->getDataFormat(), texture));
}

}  // namespace inviwo::hdf5
