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

#include <modules/base/algorithm/mesh/axisalignedboundingbox.h>
#include <modules/base/algorithm/dataminmax.h>

namespace inviwo {

namespace meshutil {

std::pair<vec3, vec3> axisAlignedBoundingBox(
    const std::vector<std::shared_ptr<const Mesh>>& meshes) {
    if (meshes.empty()) {
        return {vec3(0.f), vec3(0.f)};
    }

    vec3 worldMin(std::numeric_limits<float>::max());
    vec3 worldMax(std::numeric_limits<float>::lowest());
    for (const auto& mesh : meshes) {
        auto minmax = axisAlignedBoundingBox(*mesh);
        worldMin = glm::min(worldMin, minmax.first);
        worldMax = glm::max(worldMax, minmax.second);
    }
    return {worldMin, worldMax};
}

std::pair<vec3, vec3> axisAlignedBoundingBox(const Mesh& mesh) {
    vec3 worldMin(std::numeric_limits<float>::max());
    vec3 worldMax(std::numeric_limits<float>::lowest());

    const auto& buffers = mesh.getBuffers();
    auto it = std::find_if(buffers.begin(), buffers.end(), [](const auto& buff) {
        return buff.first.type == BufferType::PositionAttrib;
    });
    if (it != buffers.end() && it->second->getSize() > 0) {
        auto minmax = util::bufferMinMax(it->second.get());

        mat4 trans = mesh.getCoordinateTransformer().getDataToWorldMatrix();
        worldMin = glm::min(worldMin, vec3(trans * vec4(vec3(minmax.first), 1.f)));
        worldMax = glm::max(worldMax, vec3(trans * vec4(vec3(minmax.second), 1.f)));
    } else {
        // No vertices, use same values for min/max
        worldMin = worldMax = mesh.getOffset();
    }
    return {worldMin, worldMax};
}

}  // namespace meshutil

}  // namespace inviwo
