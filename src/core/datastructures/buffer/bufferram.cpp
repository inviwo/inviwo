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

#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/data.h>

#include <algorithm>

namespace inviwo {

BufferRAM::BufferRAM(const DataFormatBase *format, BufferUsage usage, BufferTarget target)
    : BufferRepresentation(format, usage, target) {}

std::type_index BufferRAM::getTypeIndex() const { return std::type_index(typeid(BufferRAM)); }

//! [Format Dispatching Example]
struct BufferRamCreationDispatcher {

    template <typename Result, typename T>
    Result operator()(size_t size, BufferUsage usage, BufferTarget target) {
        using F = typename T::type;
        switch (target) {
            case BufferTarget::Index:
                return std::make_shared<BufferRAMPrecision<F, BufferTarget::Index>>(size, usage);
            case BufferTarget::Data:
            default:
                return std::make_shared<BufferRAMPrecision<F, BufferTarget::Data>>(size, usage);
        }
    }
};

std::shared_ptr<BufferRAM> createBufferRAM(size_t size, const DataFormatBase *format,
                                           BufferUsage usage, BufferTarget target) {

    BufferRamCreationDispatcher disp;
    return dispatching::dispatch<std::shared_ptr<BufferRAM>, dispatching::filter::All>(
        format->getId(), disp, size, usage, target);
}
//! [Format Dispatching Example]

bool operator==(const BufferBase &bufA, const BufferBase &bufB) {
    if (&bufA == &bufB) {
        return true;
    }
    if (bufA.getDataFormat()->getId() != bufB.getDataFormat()->getId()) {
        return false;
    }
    if (bufA.getSize() != bufB.getSize()) {
        return false;
    }

    return bufA.getRepresentation<BufferRAM>()->dispatch<bool>([&](const auto buffer) {
        using ValueType = util::PrecisionValueType<decltype(buffer)>;

        auto containerA = buffer->getDataContainer();
        auto bufBRAM = bufB.getRepresentation<BufferRAM>();
        auto containerB =
            static_cast<const BufferRAMPrecision<ValueType> *>(bufBRAM)->getDataContainer();

        return std::equal(containerA.begin(), containerA.end(), containerB.begin(),
                          containerB.end());
    });
}

bool operator!=(const BufferBase &bufA, const BufferBase &bufB) { return !(bufA == bufB); }

}  // namespace inviwo
