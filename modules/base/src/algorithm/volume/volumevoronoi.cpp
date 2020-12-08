/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <modules/base/algorithm/volume/volumevoronoi.h>

namespace inviwo {
namespace util {

std::shared_ptr<Volume> voronoiSegmentation(
    std::shared_ptr<const Volume> volume,
    const std::vector<std::pair<uint32_t, vec3>>& seedPointsWithIndices,
    const std::optional<std::vector<float>>& weights, bool weightedVoronoi) {

    auto newVolumeRep =
        std::make_shared<VolumeRAMPrecision<unsigned short>>(volume->getDimensions());
    auto newVolume = std::make_shared<Volume>(newVolumeRep);
    newVolume->setModelMatrix(volume->getModelMatrix());
    newVolume->setWorldMatrix(volume->getWorldMatrix());

    newVolume->setInterpolation(InterpolationType::Nearest);
    newVolume->setWrapping(wrapping3d::clampAll);

    newVolume->dataMap_.dataRange = dvec2(0.0, static_cast<double>(seedPointsWithIndices.size()));
    newVolume->dataMap_.valueRange = newVolume->dataMap_.dataRange;

    const auto indexToModel = mat3(volume->getCoordinateTransformer().getIndexToModelMatrix());
    auto volumeIndices = newVolumeRep->getDataTyped();
    util::IndexMapper3D index(volume->getDimensions());

    volume->getRepresentation<VolumeRAM>()->dispatch<void>([&](auto vrprecision) {
        util::forEachVoxelParallel(*vrprecision, [&](const size3_t& voxelPos) {
            const auto transformedVoxelPos = indexToModel * voxelPos;
            auto minDist = std::numeric_limits<double>::max();

            for (size_t i = 0; i < seedPointsWithIndices.size(); i++) {
                // Squared distance
                auto dist = glm::distance2(seedPointsWithIndices[i].second, transformedVoxelPos);

                if (weightedVoronoi && weights.has_value()) {
                    dist = dist - weights.value()[i] * weights.value()[i];
                }

                if (dist < minDist) {
                    volumeIndices[index(voxelPos)] = seedPointsWithIndices[i].first;
                    minDist = dist;
                }
            }
        });
    });

    return newVolume;
};

}  // namespace util
}  // namespace inviwo
