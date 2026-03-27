/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2026 Inviwo Foundation
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

#include <inviwo/core/algorithm/camerautils.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/ordinalrefproperty.h>
#include <inviwo/core/util/glmvec.h>
#include <modules/base/algorithm/mesh/axisalignedboundingbox.h>

#include <algorithm>

#include <glm/common.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace inviwo {
class Mesh;

namespace meshutil {

void centerViewOnMeshes(const std::vector<std::shared_ptr<const Mesh>>& meshes,
                        CameraProperty& camera, double minMaxRatio) {
    const auto minmax = meshutil::axisAlignedBoundingBox(meshes);
    const auto newLookTo = 0.5 * (minmax.first + minmax.second);
    // Locking to avoid multiple evaluations when changing properties
    const NetworkLock lock(&camera);
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

std::pair<double, double> computeNearFarPlanes(std::pair<dvec3, dvec3> worldSpaceBoundingBox,
                                               const CameraProperty& camera, double nearFarRatio) {
    auto m = glm::scale(worldSpaceBoundingBox.second - worldSpaceBoundingBox.first);
    m[3] = dvec4(worldSpaceBoundingBox.first, 1.0);

    auto maxViewLength =
        std::max(glm::distance(camera.lookFrom_.getMaxValue(), camera.lookTo_.get()),
                 glm::distance(camera.lookFrom_.getMinValue(), camera.lookTo_.get()));

    return camerautil::computeCameraNearFar(m, maxViewLength, nearFarRatio);
}

}  // namespace meshutil

}  // namespace inviwo
