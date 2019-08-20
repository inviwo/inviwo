/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/base/algorithm/mesh/meshcameraalgorithms.h>
#include <modules/base/algorithm/mesh/axisalignedboundingbox.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/algorithm/camerautils.h>

namespace inviwo {

namespace meshutil {

void centerViewOnMeshes(const std::vector<std::shared_ptr<const Mesh>>& meshes,
                        CameraProperty& camera, float minMaxRatio) {
    auto minmax = meshutil::axisAlignedBoundingBox(meshes);
    auto newLookTo = 0.5f * (minmax.first + minmax.second);
    // Locking to avoid multiple evaluations when changing properties
    NetworkLock lock(&camera);
    // Make sure the new value is not clamped
    auto& lookTo = camera.lookTo_;
    lookTo.set(newLookTo,
               glm::min(lookTo.getMinValue(), newLookTo - minMaxRatio * glm::abs(newLookTo)),
               glm::max(lookTo.getMaxValue(), newLookTo + minMaxRatio * glm::abs(newLookTo)),
               lookTo.getIncrement());
    // Adjust near/far planes if necessary
    auto nearFar = computeNearFarPlanes(minmax, camera);
    camera.setNearFarPlaneDist(nearFar.first, nearFar.second, minMaxRatio);
}

std::pair<float, float> computeNearFarPlanes(std::pair<vec3, vec3> worldSpaceBoundingBox,
                                             const CameraProperty& camera, float nearFarRatio) {
    auto m = glm::scale(worldSpaceBoundingBox.second - worldSpaceBoundingBox.first);
    m[3] = vec4(worldSpaceBoundingBox.first, 1.0f);

    auto maxViewLength =
        std::max(glm::distance(camera.lookFrom_.getMaxValue(), camera.lookTo_.get()),
                 glm::distance(camera.lookFrom_.getMinValue(), camera.lookTo_.get()));

    return camerautil::computeCameraNearFar(m, maxViewLength, nearFarRatio);
}

}  // namespace meshutil

}  // namespace inviwo
