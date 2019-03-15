/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#include <inviwo/core/util/volumeutils.h>
#include <inviwo/core/datastructures/volume/volume.h>

namespace inviwo {

namespace util {

bool hasMargins(const std::shared_ptr<const Volume> &volume) {
    return (volume && volume->getMetaData<BoolMetaData>("marginsEnabled", false));
}

bool isBricked(const std::shared_ptr<const Volume> &volume) {
    return (volume && volume->getMetaData<BoolMetaData>("brickedVolume", false));
}

size3_t getBrickDimensions(const std::shared_ptr<const Volume> &volume) {
    if (!volume) {
        return size3_t(1, 1, 1);
    }
    return size3_t(volume->getMetaData<IntVec3MetaData>("brickDim", ivec3(1, 1, 1)));
}

std::pair<vec3, vec3> getVolumeMargins(const std::shared_ptr<const Volume> &volume) {
    if (hasMargins(volume)) {
        auto marginsBottomLeft =
            volume->getMetaData<FloatVec3MetaData>("marginsBottomLeft", vec3(0.0f));
        auto marginsTopRight =
            volume->getMetaData<FloatVec3MetaData>("marginsTopRight", vec3(0.0f));
        return {marginsBottomLeft, marginsTopRight};
    }
    return {vec3(0.0f), vec3(0.0f)};
}

size3_t getVolumeDimensions(const std::shared_ptr<const Volume> &volume) {
    if (!volume) {
        return {};
    }
    auto dims = volume->getDimensions();
    // adjust dimensions for bricked volumes
    if (isBricked(volume)) {
        // volume dimensions refer only to the size of the index volume,
        // multiply it by brick dimension
        dims *= getBrickDimensions(volume);
    }
    // re-adjust volume dimensions by considering margins
    // the volume dimensions should not cover the area outside the margins
    if (hasMargins(volume)) {
        // volume has margins enabled
        // adjust start and end texture coordinate accordingly
        vec3 marginsBottomLeft;
        vec3 marginsTopRight;
        std::tie(marginsBottomLeft, marginsTopRight) = getVolumeMargins(volume);

        if (glm::any(greaterThan(marginsBottomLeft, vec3(0.0f))) ||
            glm::any(greaterThan(marginsTopRight, vec3(0.0f)))) {
            // adjust margins only if they are set
            dims = size3_t(vec3(dims) * (vec3(1.0f) - (marginsBottomLeft + marginsTopRight)));
        }
    }
    return dims;
}

double voxelVolume(const Volume &volume) {
    auto basis = volume.getBasis();
    auto dims = volume.getDimensions();
    auto a = basis[0] / static_cast<float>(dims.x);
    auto b = basis[1] / static_cast<float>(dims.y);
    auto c = basis[2] / static_cast<float>(dims.z);
    return glm::dot(glm::cross(a, b), c);
}

}  // namespace util

}  // namespace inviwo
