/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2026 Inviwo Foundation
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

#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/datastructures/representationconverter.h>
#include <inviwo/core/datastructures/representationconverterfactory.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/util/glmutils.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/indexmapper.h>

#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_set>

#include <glm/gtx/component_wise.hpp>
#include <glm/vec3.hpp>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeShifter::processorInfo_{
    "org.inviwo.VolumeShifter",  // Class identifier
    "Volume Shifter",            // Display name
    "Volume Operation",          // Category
    CodeState::Experimental,     // Code state
    Tags::CPU,                   // Tags
    R"(Shifts the voxel data within the volume by a pre-defined offset. Voxel data is wrapped. This
    processor does not change any other properties, e.g. dimensions, of the volume.
    )"_unindentHelp,
};
const ProcessorInfo& VolumeShifter::getProcessorInfo() const { return processorInfo_; }

VolumeShifter::VolumeShifter()
    : Processor()
    , inport_("volume", "input volume"_help)
    , outport_("outport", "resulting volume with shifted voxel data"_help)
    , enabled_("enabled", "Enabled", "if not enabled, the input volume is forwarded"_help, true)
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
    vol->dataMap = inport_.getData()->dataMap;
    vol->setModelMatrix(inport_.getData()->getModelMatrix());
    vol->setWorldMatrix(inport_.getData()->getWorldMatrix());

    outport_.setData(vol);
}

}  // namespace inviwo
