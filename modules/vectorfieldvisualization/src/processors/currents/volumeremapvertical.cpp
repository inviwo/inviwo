/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/processors/currents/volumeremapvertical.h>
#include <inviwo/core/util/volumesampler.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/volumeramutils.h>
#include <inviwo/core/util/indexmapper.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeRemapVertical::processorInfo_{
    "org.inviwo.VolumeRemapVertical",  // Class identifier
    "Volume Remap Vertical",           // Display name
    "Undefined",                       // Category
    CodeState::Experimental,           // Code state
    Tags::None,                        // Tags
};
const ProcessorInfo VolumeRemapVertical::getProcessorInfo() const { return processorInfo_; }

VolumeRemapVertical::VolumeRemapVertical()
    : Processor()
    , volumeIn_("volumeIn")
    , newSize_("newSize", "New Size", ivec3(128, 128, 128),
               {ivec3(32), ConstraintBehavior::Immutable},
               {ivec3(1028), ConstraintBehavior::Ignore})
    , negativeDepth_("negativeDepth", "Increasing Depth", true)
    , volumeOut_("volumeOut") {

    addPorts(volumeIn_, volumeOut_);
    addProperties(newSize_, negativeDepth_);
}

void VolumeRemapVertical::process() {
    std::cout << "Starting volume remap vertical" << std::endl;
    static const std::array<double, 50> VERTICAL_HEIGHTS = {
        5.0335,  15.1006, 25.2194, 35.3585, 45.5764, 55.8532, 66.2617, 76.8028, 87.577,  98.6233,
        110.096, 122.107, 134.909, 148.747, 164.054, 181.312, 201.263, 224.777, 253.068, 287.551,
        330.008, 382.365, 446.726, 524.982, 618.703, 728.692, 854.994, 996.715, 1152.38, 1320,
        1497.56, 1683.06, 1874.79, 2071.25, 2271.32, 2474.04, 2678.76, 2884.9,  3092.12, 3300.09,
        3508.63, 3717.57, 3926.81, 4136.25, 4345.86, 4555.57, 4765.37, 4975.21, 5185.11, 5395.02};
    static const double VERTICAL_MAX = 5500.0;

    // Just do dvec4 as outut, and we need no templating.
    // Would be great to only have as many components as the input, but also much more work...
    VolumeSampler samplerIn(volumeIn_.getData());

    size3_t sizeOut = newSize_.get();
    auto volumeOutRAM = std::make_shared<VolumeRAMPrecision<dvec4>>(sizeOut);
    dvec4* dataOut = volumeOutRAM->getDataTyped();
    const util::IndexMapper3D indexMapper(sizeOut);

    util::forEachVoxelParallel(*volumeOutRAM, [&](const size3_t& posIdx) {
        vec3 posOut = {(0.5f + posIdx.x) / sizeOut.x, (0.5f + posIdx.y) / sizeOut.y,
                       (0.5f + posIdx.z) / sizeOut.z};
        double verticalPos = posOut.z * VERTICAL_MAX;
        if (negativeDepth_.get()) {
            verticalPos = VERTICAL_MAX - verticalPos;
        }
        double posInZ = 0.0;
        if (verticalPos < VERTICAL_HEIGHTS[0]) {
            posInZ = 0.0;
        } else if (verticalPos > VERTICAL_HEIGHTS.back()) {
            posInZ = 1.0;
        } else {
            auto it =
                std::lower_bound(VERTICAL_HEIGHTS.begin(), VERTICAL_HEIGHTS.end(), verticalPos);
            size_t idx = std::distance(VERTICAL_HEIGHTS.begin(), it);
            double lower = VERTICAL_HEIGHTS[idx - 1];
            double upper = VERTICAL_HEIGHTS[idx];
            double cellPercentage = (verticalPos - lower) / (upper - lower);
            posInZ = (idx + cellPercentage) / (VERTICAL_HEIGHTS.size() - 1);
        }
        if (negativeDepth_.get()) {
            posInZ = 1.0 - posInZ;
        }
        dataOut[indexMapper(posIdx)] = samplerIn.sampleDataSpace({posOut.x, posOut.y, posInZ});
    });

    auto volumeOut = std::make_shared<Volume>(volumeOutRAM);
    volumeOut->setModelMatrix(volumeIn_.getData()->getModelMatrix());
    volumeOut->setWorldMatrix(volumeIn_.getData()->getWorldMatrix());
    volumeOut->copyMetaDataFrom(*volumeIn_.getData());
    volumeOut->dataMap_ = volumeIn_.getData()->dataMap_;
    volumeOut_.setData(volumeOut);
    std::cout << "Finished volume remap vertical" << std::endl;
}

}  // namespace inviwo
