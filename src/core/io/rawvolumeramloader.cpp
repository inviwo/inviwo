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

#include <inviwo/core/io/rawvolumeramloader.h>

#include <inviwo/core/datastructures/volume/volumeramprecision.h>

#include <glm/gtx/component_wise.hpp>

namespace inviwo {

RawVolumeRAMLoader::RawVolumeRAMLoader(const std::filesystem::path& rawFile, size_t offset,
                                       ByteOrder byteOrder, Compression compression)
    : rawFile_{rawFile}, offset_{offset}, byteOrder_{byteOrder}, compression_{compression} {}

RawVolumeRAMLoader* RawVolumeRAMLoader::clone() const { return new RawVolumeRAMLoader(*this); }

std::shared_ptr<VolumeRepresentation> RawVolumeRAMLoader::createRepresentation(
    const VolumeRepresentation& src) const {

    const auto size = glm::compMul(src.getDimensions()) * src.getDataFormat()->getSizeInBytes();
    auto data = std::make_unique<char[]>(size);
    if (compression_ == Compression::Enabled) {
        util::readCompressedBytesIntoBuffer(rawFile_, offset_, size, byteOrder_,
                                            src.getDataFormat()->getSizeInBytes(), data.get());
    } else {
        util::readBytesIntoBuffer(rawFile_, offset_, size, byteOrder_,
                                  src.getDataFormat()->getSizeInBytes(), data.get());
    }

    auto volumeRAM =
        createVolumeRAM(src.getDimensions(), src.getDataFormat(), data.get(), src.getSwizzleMask(),
                        src.getInterpolation(), src.getWrapping());
    data.release();

    return volumeRAM;
}

void RawVolumeRAMLoader::updateRepresentation(std::shared_ptr<VolumeRepresentation> dest,
                                              const VolumeRepresentation& src) const {
    auto volumeDst = std::static_pointer_cast<VolumeRAM>(dest);

    if (src.getDimensions() != volumeDst->getDimensions()) {
        volumeDst->setDimensions(src.getDimensions());
    }

    const auto size = glm::compMul(src.getDimensions());
    if (compression_ == Compression::Enabled) {
        util::readCompressedBytesIntoBuffer(
            rawFile_, offset_, size * src.getDataFormat()->getSizeInBytes(), byteOrder_,
            src.getDataFormat()->getSizeInBytes(), volumeDst->getData());
    } else {
        util::readBytesIntoBuffer(rawFile_, offset_, size * src.getDataFormat()->getSizeInBytes(),
                                  byteOrder_, src.getDataFormat()->getSizeInBytes(),
                                  volumeDst->getData());
    }

    volumeDst->setSwizzleMask(src.getSwizzleMask());
    volumeDst->setInterpolation(src.getInterpolation());
    volumeDst->setWrapping(src.getWrapping());
}
}  // namespace inviwo
