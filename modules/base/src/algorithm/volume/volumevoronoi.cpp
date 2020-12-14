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
    const size3_t volumeDimensions, const mat4& indexToModelMatrix,
    const std::vector<std::pair<uint32_t, vec3>>& seedPointsWithIndices,
    const std::optional<std::vector<float>>& weights, bool weightedVoronoi) {

    if (seedPointsWithIndices.size() == 0) {
        throw Exception("No seed points, cannot create volume voronoi segmentation",
                        IVW_CONTEXT_CUSTOM("VoronoiSegmentation"));
    }

    if (weightedVoronoi && !weights.has_value()) {
        throw Exception("Cannot use weighted voronoi when no weights are provided",
                        IVW_CONTEXT_CUSTOM("VoronoiSegmentation"));
    }

    if (weightedVoronoi && weights.has_value() &&
        weights.value().size() != seedPointsWithIndices.size()) {
        throw Exception(
            "Cannot use weighted voronoi when dimensions do not match (weights and seed positions)",
            IVW_CONTEXT_CUSTOM("VoronoiSegmentation"));
    }

    auto newVolumeRep = std::make_shared<VolumeRAMPrecision<unsigned short>>(volumeDimensions);
    auto newVolume = std::make_shared<Volume>(newVolumeRep);

    newVolume->dataMap_.dataRange = dvec2(0.0, static_cast<double>(seedPointsWithIndices.size()));
    newVolume->dataMap_.valueRange = newVolume->dataMap_.dataRange;

    auto volumeIndices = newVolumeRep->getDataTyped();
    util::IndexMapper3D index(volumeDimensions);

    if (weightedVoronoi && weights.has_value()) {
        util::forEachVoxelParallel(volumeDimensions, [&](const size3_t& voxelPos) {
            const auto transformedVoxelPos = mat3(indexToModelMatrix) * voxelPos;
            auto zipped = util::zip(seedPointsWithIndices, weights.value());

            auto&& [posWithIndex, weight] = *std::min_element(
                zipped.begin(), zipped.end(), [transformedVoxelPos](auto&& i1, auto&& i2) {
                    auto&& [p1, w1] = i1;
                    auto&& [p2, w2] = i2;

                    return glm::distance2(p1.second, transformedVoxelPos) - w1 * w1 <
                           glm::distance2(p2.second, transformedVoxelPos) - w2 * w2;
                });
            volumeIndices[index(voxelPos)] = static_cast<unsigned short>(posWithIndex.first);
        });
    } else {
        util::forEachVoxelParallel(volumeDimensions, [&](const size3_t& voxelPos) {
            const auto transformedVoxelPos = mat3(indexToModelMatrix) * voxelPos;
            auto it = std::min_element(seedPointsWithIndices.cbegin(), seedPointsWithIndices.cend(),
                                       [transformedVoxelPos](const auto& p1, const auto& p2) {
                                           return glm::distance2(p1.second, transformedVoxelPos) <
                                                  glm::distance2(p2.second, transformedVoxelPos);
                                       });
            volumeIndices[index(voxelPos)] = static_cast<unsigned short>(it->first);
        });
    }

    return newVolume;
};

}  // namespace util
}  // namespace inviwo
