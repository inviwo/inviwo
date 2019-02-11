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

#include <modules/base/algorithm/volume/surfaceextraction.h>

namespace inviwo {
namespace marching {

glm::vec3 interpolate(const glm::vec3 &p0, double v0, const glm::vec3 &p1, double v1) {
    if (v0 == v1) {
        return p0;
    }
    float t = static_cast<float>(v0 / (v0 - v1));
    return p0 + t * (p1 - p0);
}

void evaluateTriangle(K3DTree<size_t, float> &vertexTree, IndexBufferRAM *indexBuffer,
                      std::vector<vec3> &positions, std::vector<vec3> &normals, const glm::vec3 &p0,
                      double v0, const glm::vec3 &p1, double v1, const glm::vec3 &p2, double v2) {
    int index = 0;
    if (v0 <= 0.0) index += 1;
    if (v1 <= 0.0) index += 2;
    if (v2 <= 0.0) index += 4;

    if (index == 0) {  // FULLY OUTSIDE
        return;
    } else if (index == 1) {  // ONLY P0 INSIDE
        auto p01 = interpolate(p0, v0, p1, v1);
        auto p02 = interpolate(p0, v0, p2, v2);
        addTriangle(vertexTree, indexBuffer, positions, normals, p0, p01, p02);
    } else if (index == 2) {  // ONLY P1 INSIDE
        auto p10 = interpolate(p1, v1, p0, v0);
        auto p12 = interpolate(p1, v1, p2, v2);
        addTriangle(vertexTree, indexBuffer, positions, normals, p1, p12, p10);
    } else if (index == 3) {  // P0 AND P1 INSIDE
        auto p02 = interpolate(p0, v0, p2, v2);
        auto p12 = interpolate(p1, v1, p2, v2);
        addTriangle(vertexTree, indexBuffer, positions, normals, p0, p1, p12);
        addTriangle(vertexTree, indexBuffer, positions, normals, p0, p12, p02);
    } else if (index == 4) {  // ONLY P2 INSIDE
        auto p20 = interpolate(p2, v2, p0, v0);
        auto p21 = interpolate(p2, v2, p1, v1);
        addTriangle(vertexTree, indexBuffer, positions, normals, p2, p20, p21);
    } else if (index == 5) {  // P0 AND P2 INSIDE
        auto p01 = interpolate(p0, v0, p1, v1);
        auto p21 = interpolate(p2, v2, p1, v1);
        addTriangle(vertexTree, indexBuffer, positions, normals, p0, p01, p21);
        addTriangle(vertexTree, indexBuffer, positions, normals, p0, p21, p2);
    } else if (index == 6) {  // P1 AND P2 INSIDE
        auto p10 = interpolate(p1, v1, p0, v0);
        auto p20 = interpolate(p2, v2, p0, v0);
        addTriangle(vertexTree, indexBuffer, positions, normals, p1, p20, p10);
        addTriangle(vertexTree, indexBuffer, positions, normals, p1, p2, p20);
    } else if (index == 7) {  // FULLY INSIDE
        addTriangle(vertexTree, indexBuffer, positions, normals, p0, p1, p2);
    }
}

size_t addVertex(K3DTree<size_t, float> &vertexTree, std::vector<vec3> &positions,
                 std::vector<vec3> &normals, const vec3 pos) {
    auto nearest = vertexTree.findNearest(vec3(pos));
    const auto nearestPos = [&]() {
        return vec3{nearest->getPosition()[0], nearest->getPosition()[1],
                    nearest->getPosition()[2]};
    };
    if (!nearest || (glm::distance2(nearestPos(), pos) > glm::epsilon<float>())) {
        nearest = vertexTree.insert(pos, positions.size());
        positions.push_back(pos);
        normals.push_back(vec3(0, 0, 0));
    }
    return nearest->get();
}

void addTriangle(K3DTree<size_t, float> &vertexTree, IndexBufferRAM *indexBuffer,
                 std::vector<vec3> &positions, std::vector<vec3> &normals, const glm::vec3 &a,
                 const glm::vec3 &b, const glm::vec3 &c) {
    size_t i0 = addVertex(vertexTree, positions, normals, a);
    size_t i1 = addVertex(vertexTree, positions, normals, b);
    size_t i2 = addVertex(vertexTree, positions, normals, c);

    if (i0 == i1 || i0 == i2 || i1 == i2) {
        // triangle is so small so that the vertices are merged.
        return;
    }

    indexBuffer->add(static_cast<uint32_t>(i0));
    indexBuffer->add(static_cast<uint32_t>(i1));
    indexBuffer->add(static_cast<uint32_t>(i2));

    vec3 e0 = b - a;
    vec3 e1 = c - a;
    vec3 n = glm::normalize(glm::cross(e0, e1));

    normals[i0] += n;
    normals[i1] += n;
    normals[i2] += n;
}

}  // namespace marching
}  // namespace inviwo
