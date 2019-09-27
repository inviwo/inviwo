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

#include <modules/base/algorithm/cubeproxygeometry.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/geometry/simplemeshcreator.h>
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/util/volumeutils.h>

namespace inviwo {

namespace algorithm {

std::shared_ptr<SimpleMesh> createCubeProxyGeometry(const std::shared_ptr<const Volume> &volume) {
    vec3 startDataTexCoord = vec3(0.0f);
    vec3 extent = vec3(1.0f);
    // the extent will be added to the origin in the parallelepiped() yielding the end position

    if (util::hasMargins(volume)) {
        // volume has margins enabled
        // adjust start and end texture coordinate accordingly
        vec3 marginsBottomLeft;
        vec3 marginsTopRight;
        std::tie(marginsBottomLeft, marginsTopRight) = util::getVolumeMargins(volume);

        startDataTexCoord += marginsBottomLeft;
        // extent needs to be adjusted for both margins
        extent -= marginsBottomLeft + marginsTopRight;
    }

    glm::vec3 origin(0.0f);
    glm::vec3 e1(1.0f, 0.0f, 0.0f);
    glm::vec3 e2(0.0f, 1.0f, 0.0f);
    glm::vec3 e3(0.0f, 0.0f, 1.0f);
    glm::vec3 texOrigin(startDataTexCoord);
    glm::vec3 t1(extent.x, 0.0f, 0.0f);
    glm::vec3 t2(0.0f, extent.y, 0.0f);
    glm::vec3 t3(0.0f, 0.0f, extent.z);
    glm::vec4 colOrigin(startDataTexCoord, 1.0f);
    glm::vec4 c1(t1, 0.0f);
    glm::vec4 c2(t2, 0.0f);
    glm::vec4 c3(t3, 0.0f);

    // Create parallelepiped and set it to the outport. The outport will own the data.
    auto geom = SimpleMeshCreator::parallelepiped(origin, e1, e2, e3, texOrigin, t1, t2, t3,
                                                  colOrigin, c1, c2, c3);
    if (volume) {
        geom->setModelMatrix(volume->getModelMatrix());
        geom->setWorldMatrix(volume->getWorldMatrix());
    }

    return geom;
}

std::shared_ptr<SimpleMesh> createCubeProxyGeometry(const std::shared_ptr<const Volume> &volume,
                                                    const vec3 &clipOrigin,
                                                    const vec3 &clipExtent) {
    vec3 startDataTexCoord = vec3(0.0f);
    vec3 extent = vec3(1.0f);
    // the extent will be added to the origin in the parallelepiped() yielding the end position

    if (util::hasMargins(volume)) {
        // volume has margins enabled
        // adjust start and end texture coordinate accordingly
        vec3 marginsBottomLeft;
        vec3 marginsTopRight;
        std::tie(marginsBottomLeft, marginsTopRight) = util::getVolumeMargins(volume);

        startDataTexCoord += marginsBottomLeft;
        // extent needs to be adjusted for both margins
        extent -= marginsBottomLeft + marginsTopRight;
    }

    glm::vec3 origin(0.0f);
    glm::vec3 e1(1.0f, 0.0f, 0.0f);
    glm::vec3 e2(0.0f, 1.0f, 0.0f);
    glm::vec3 e3(0.0f, 0.0f, 1.0f);
    glm::vec3 texOrigin(startDataTexCoord);
    glm::vec3 t1(extent.x, 0.0f, 0.0f);
    glm::vec3 t2(0.0f, extent.y, 0.0f);
    glm::vec3 t3(0.0f, 0.0f, extent.z);
    glm::vec4 colOrigin(startDataTexCoord, 1.0f);
    glm::vec4 c1(t1, 0.0f);
    glm::vec4 c2(t2, 0.0f);
    glm::vec4 c3(t3, 0.0f);

    // clip initial bounding cube
    origin = origin + e1 * clipOrigin.x + e2 * clipOrigin.y + e3 * clipOrigin.z;
    e1 *= clipExtent.x;
    e2 *= clipExtent.y;
    e3 *= clipExtent.z;

    texOrigin = texOrigin + t1 * clipOrigin.x + t2 * clipOrigin.y + t3 * clipOrigin.z;
    t1 *= clipExtent.x;
    t2 *= clipExtent.y;
    t3 *= clipExtent.z;

    colOrigin += c1 * clipOrigin.x + c2 * clipOrigin.y + c3 * clipOrigin.z;
    c1 *= clipExtent.x;
    c2 *= clipExtent.y;
    c3 *= clipExtent.z;

    // Create parallelepiped and set it to the outport. The outport will own the data.
    auto geom = SimpleMeshCreator::parallelepiped(origin, e1, e2, e3, texOrigin, t1, t2, t3,
                                                  colOrigin, c1, c2, c3);
    if (volume) {
        geom->setModelMatrix(volume->getModelMatrix());
        geom->setWorldMatrix(volume->getWorldMatrix());
    }
    return geom;
}

std::shared_ptr<SimpleMesh> createCubeProxyGeometry(const std::shared_ptr<const Volume> &volume,
                                                    const size3_t &clipMin,
                                                    const size3_t &clipMax) {
    if (!volume) {
        // the volume is required to figure out the clipped texture coordinates
        return createCubeProxyGeometry(volume);
    }

    const size3_t volDims = util::getVolumeDimensions(volume);
    const vec3 extent(glm::max(vec3(volDims) - 1.0f, 1.0f));

    // sanitize clip min/max args with respect to volume dimensions
    const size3_t min = glm::min(clipMin, volDims);
    const size3_t max = glm::min(clipMax, volDims);

    vec3 clipOrigin(vec3(min) / extent);
    vec3 clipExtent(vec3(max - min) / extent);

    return createCubeProxyGeometry(volume, clipOrigin, clipExtent);
}

}  // namespace algorithm

}  // namespace inviwo
