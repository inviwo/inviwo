/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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

#include <modules/opengl/volume/volumeglconverter.h>

#include <inviwo/core/datastructures/volume/volumeram.h>  // for VolumeRAM (ptr only), createVol...
#include <inviwo/core/util/formats.h>                     // for DataFormatBase
#include <inviwo/core/util/logcentral.h>                  // for LogCentral, LogError
#include <modules/opengl/volume/volumegl.h>               // for VolumeGL
#include <modules/opengl/texture/texture3d.h>             // IWYU pragma: keep

#include <ostream>      // for operator<<, char_traits
#include <type_traits>  // for remove_extent_t

namespace inviwo {

std::shared_ptr<VolumeGL> VolumeRAM2GLConverter::createFrom(
    std::shared_ptr<const VolumeRAM> src) const {
    auto dst = std::make_shared<VolumeGL>(src->getDimensions(), src->getDataFormat(),
                                          src->getSwizzleMask(), src->getInterpolation(),
                                          src->getWrapping(), false);

    if (!dst) {
        throw ConverterException(IVW_CONTEXT, "Cannot convert format '{}' from RAM to GL",
                                 *src->getDataFormat());
    }

    dst->getTexture()->initialize(src->getData());
    return dst;
}

void VolumeRAM2GLConverter::update(std::shared_ptr<const VolumeRAM> src,
                                   std::shared_ptr<VolumeGL> dst) const {
    dst->setDimensions(src->getDimensions());
    dst->setSwizzleMask(src->getSwizzleMask());
    dst->setInterpolation(src->getInterpolation());
    dst->setWrapping(src->getWrapping());

    dst->getTexture()->upload(src->getData());
}

std::shared_ptr<VolumeRAM> VolumeGL2RAMConverter::createFrom(
    std::shared_ptr<const VolumeGL> src) const {
    auto dst = createVolumeRAM(src->getDimensions(), src->getDataFormat(), nullptr,
                               src->getSwizzleMask(), src->getInterpolation(), src->getWrapping());

    if (!dst) {
        throw ConverterException(IVW_CONTEXT, "Cannot convert format '{}' from GL to RAM",
                                 *src->getDataFormat());
    }

    src->getTexture()->download(dst->getData());
    return dst;
}

void VolumeGL2RAMConverter::update(std::shared_ptr<const VolumeGL> src,
                                   std::shared_ptr<VolumeRAM> dst) const {
    dst->setDimensions(src->getDimensions());
    dst->setSwizzleMask(src->getSwizzleMask());
    dst->setInterpolation(src->getInterpolation());
    dst->setWrapping(src->getWrapping());

    src->getTexture()->download(dst->getData());
}

}  // namespace inviwo
