/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

namespace inviwo {

RawVolumeRAMLoader::RawVolumeRAMLoader(const std::string& rawFile, size_t offset,
                                       size3_t dimensions, bool littleEndian,
                                       const DataFormatBase* format)
    : rawFile_(rawFile)
    , offset_(offset)
    , dimensions_(dimensions)
    , littleEndian_(littleEndian)
    , format_(format) {}

RawVolumeRAMLoader* RawVolumeRAMLoader::clone() const { return new RawVolumeRAMLoader(*this); }

std::shared_ptr<DataRepresentation> RawVolumeRAMLoader::createRepresentation() const {
    return format_->dispatch(*this);
}

void RawVolumeRAMLoader::updateRepresentation(std::shared_ptr<DataRepresentation> dest) const {
    auto volumeDst = std::static_pointer_cast<VolumeRAM>(dest);

    if (dimensions_ != volumeDst->getDimensions()) {
        throw Exception("Mismatching volume dimensions, can't update", IvwContext);
    }

    std::size_t size = dimensions_.x * dimensions_.y * dimensions_.z;
    util::readBytesIntoBuffer(rawFile_, offset_, size * format_->getSize(), littleEndian_,
                              format_->getSize(), volumeDst->getData());
}
}  // namespace
