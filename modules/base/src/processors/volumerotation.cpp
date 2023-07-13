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

#include <modules/base/processors/volumerotation.h>

#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeRotation::processorInfo_{
    "org.inviwo.VolumeRotation",  // Class identifier
    "Volume Rotation",            // Display name
    "Volume Operation",           // Category
    CodeState::Stable,            // Code state
    Tags::CPU,                    // Tags
};
const ProcessorInfo VolumeRotation::getProcessorInfo() const { return processorInfo_; }

VolumeRotation::VolumeRotation() : Processor(), inport_("volume"), outport_("outport") {

    addPort(inport_);
    addPort(outport_);
}

void VolumeRotation::process() {

    auto volumeRam =
        inport_.getData()->getRepresentation<VolumeRAM>()->dispatch<std::shared_ptr<VolumeRAM>>(
            [](auto vr) {
                using ValueType = util::PrecisionValueType<decltype(vr)>;

                const auto src = vr->getDataTyped();
                const auto dim = ivec3(vr->getDimensions());
                const int size = glm::compMul(dim);
                util::IndexMapper<3, int> im(dim);

                size3_t oldDim = vr->getDimensions();
                size3_t newDim = size3_t(oldDim.y, oldDim.x, oldDim.z);

                auto vol = std::make_shared<VolumeRAMPrecision<ValueType>>(
                    newDim, vr->getSwizzleMask(), vr->getInterpolation(), vr->getWrapping());
                auto dst = vol->getDataTyped();
                for (size_t z = 0; z < newDim.z; ++z)
                    for (size_t y = 0; y < newDim.y; ++y)
                        for (size_t x = 0; x < newDim.x; ++x) {
                            dst[x + y * newDim.x + z * newDim.x * newDim.y] =
                                src[y + x * oldDim.x + z * oldDim.x * oldDim.y];
                        }
                return vol;
            });

    auto vol = std::make_shared<Volume>(volumeRam);
    vol->copyMetaDataFrom(*(inport_.getData()));
    vol->dataMap_ = inport_.getData()->dataMap_;

    auto matrixSwapXY = [](auto& mat) {
        std::swap(mat[0][0], mat[1][1]);
        std::swap(mat[0][1], mat[1][0]);
        std::swap(mat[0][2], mat[1][2]);
        std::swap(mat[2][0], mat[2][1]);
        mat[0] = -mat[0];
        // mat[2][2] = -mat[2][2];
    };

    auto modelMat = inport_.getData()->getModelMatrix();
    // std::swap(modelMat[0], modelMat[1]);
    matrixSwapXY(modelMat);
    vol->setModelMatrix(modelMat);

    auto worldMat = inport_.getData()->getWorldMatrix();
    matrixSwapXY(worldMat);
    vol->setWorldMatrix(worldMat);

    outport_.setData(vol);
}

}  // namespace inviwo
