/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2021 Inviwo Foundation
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

#include <modules/base/processors/volumeshifter.h>

#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeShifter::processorInfo_{
    "org.inviwo.VolumeShifter",  // Class identifier
    "Volume Shifter",            // Display name
    "Volume Operation",          // Category
    CodeState::Experimental,     // Code state
    Tags::CPU,                   // Tags
};
const ProcessorInfo VolumeShifter::getProcessorInfo() const { return processorInfo_; }

VolumeShifter::VolumeShifter()
    : Processor()
    , inport_("volume")
    , outport_("outport")
    , enabled_("enabled", "Enabled", true)
    , offset_("offset", "Offset", vec3(0.0f), vec3(-1.0f), vec3(1.0f)) {

    addPort(inport_);
    addPort(outport_);
    addProperties(enabled_, offset_);
}

void VolumeShifter::process() {
    if (!enabled_ || (offset_.get() == vec3(0.0f))) {
        outport_.setData(inport_.getData());
        return;
    }

    const auto offset = ivec3(offset_.get() * vec3(inport_.getData()->getDimensions()));

    auto volumeRam =
        inport_.getData()->getRepresentation<VolumeRAM>()->dispatch<std::shared_ptr<VolumeRAM>>(
            [&offset](auto vr) {
                using ValueType = util::PrecisionValueType<decltype(vr)>;

                const auto src = vr->getDataTyped();
                const auto dim = ivec3(vr->getDimensions());
                const int size = glm::compMul(dim);
                util::IndexMapper<3, int> im(dim);

                auto vol = std::make_shared<VolumeRAMPrecision<ValueType>>(
                    vr->getDimensions(), vr->getSwizzleMask(), vr->getInterpolation(),
                    vr->getWrapping());
                auto dst = vol->getDataTyped();
                for (int i = 0; i < size; ++i) {
                    const auto dstIndex = VolumeRAM::periodicPosToIndex(im(i) + offset, dim);
                    IVW_ASSERT(dstIndex < size, "invalid voxel index");
                    dst[dstIndex] = src[i];
                }
                return vol;
            });

    auto vol = std::make_shared<Volume>(volumeRam);
    vol->copyMetaDataFrom(*(inport_.getData()));
    vol->dataMap_ = inport_.getData()->dataMap_;
    vol->setModelMatrix(inport_.getData()->getModelMatrix());
    vol->setWorldMatrix(inport_.getData()->getWorldMatrix());

    outport_.setData(vol);
}

}  // namespace inviwo
