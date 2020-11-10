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
    std::shared_ptr<const Volume> volume, const mat3& voxelTransformation,
    const std::unordered_map<dvec3, uint32_t>& seedPointsWithIndices) {

    // TODO: check that the seed points are inside the volume (after transforming voxel pos)??

    // Need to have this as float (otherwise I could not get it to be visible in the canvas)
    // Why??
    auto newVolumeRep = std::make_shared<VolumeRAMPrecision<float>>(volume->getDimensions());
    auto newVolume = std::make_shared<Volume>(newVolumeRep);
    newVolume->setModelMatrix(volume->getModelMatrix());
    newVolume->setWorldMatrix(volume->getWorldMatrix());

    // Should this really be the same as for the input volume?
    newVolume->setInterpolation(volume->getInterpolation());
    newVolume->setWrapping(volume->getWrapping());

    newVolume->dataMap_.dataRange = dvec2(0.0, (double)seedPointsWithIndices.size());
    newVolume->dataMap_.valueRange = newVolume->dataMap_.dataRange;

    auto newData = newVolumeRep->getDataTyped();
    util::IndexMapper3D index(volume->getDimensions());

    volume->getRepresentation<VolumeRAM>()->dispatch<void>([&](auto vrprecision) {
        util::forEachVoxelParallel(*vrprecision, [&](const size3_t& voxelPos) {
            const auto transformedVoxelPos = voxelTransformation * voxelPos;
            auto minDist = std::numeric_limits<double>::max();

            std::for_each(seedPointsWithIndices.cbegin(), seedPointsWithIndices.cend(),
                          [&](const std::pair<dvec3, uint32_t>& point) {
                              // Squared distance
                              auto dist = (point.first.x - transformedVoxelPos.x) *
                                              (point.first.x - transformedVoxelPos.x) +
                                          (point.first.y - transformedVoxelPos.y) *
                                              (point.first.y - transformedVoxelPos.y) +
                                          (point.first.z - transformedVoxelPos.z) *
                                              (point.first.z - transformedVoxelPos.z);
                              if (dist < minDist) {
                                  newData[index(voxelPos)] = point.second;
                                  minDist = dist;
                              }
                          });
        });
    });

    return newVolume;
};

}  // namespace util
}  // namespace inviwo
