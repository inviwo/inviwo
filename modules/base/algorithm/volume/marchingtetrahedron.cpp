/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include "marchingtetrahedron.h"
namespace inviwo {

std::shared_ptr<Mesh> MarchingTetrahedron::apply(std::shared_ptr<const Volume> volume, const double &iso,
                                 const vec4 &color, std::function<void(float)> progressCallback) {
    detail::MarchingTetrahedronDispatcher disp;
    return volume->getDataFormat()->dispatch(disp, volume, iso, color, progressCallback);
}

void detail::evaluateTetra(K3DTree<size_t, float> &vertexTree, IndexBufferRAM *indexBuffer,
                           std::vector<vec3> &positions, std::vector<vec3> &normals,
                           const glm::vec3 &p0, const double &v0, const glm::vec3 &p1,
                           const double &v1, const glm::vec3 &p2, const double &v2,
                           const glm::vec3 &p3, const double &v3) {
    int index = 0;
    if (v0 >= 0) index += 1;
    if (v1 >= 0) index += 2;
    if (v2 >= 0) index += 4;
    if (v3 >= 0) index += 8;
    glm::vec3 a, b, c, d;
    if (index == 0 || index == 15) return;
    if (index == 1 || index == 14) {
        a = interpolate(p0, v0, p2, v2);
        b = interpolate(p0, v0, p1, v1);
        c = interpolate(p0, v0, p3, v3);
        if (index == 1) {
            addTriangle(vertexTree, indexBuffer, positions, normals, a, b, c);
        } else {
            addTriangle(vertexTree, indexBuffer, positions, normals, a, c, b);
        }
    } else if (index == 2 || index == 13) {
        a = interpolate(p1, v1, p0, v0);
        b = interpolate(p1, v1, p2, v2);
        c = interpolate(p1, v1, p3, v3);
        if (index == 2) {
            addTriangle(vertexTree, indexBuffer, positions, normals, a, b, c);
        } else {
            addTriangle(vertexTree, indexBuffer, positions, normals, a, c, b);
        }

    } else if (index == 4 || index == 11) {
        a = interpolate(p2, v2, p0, v0);
        b = interpolate(p2, v2, p1, v1);
        c = interpolate(p2, v2, p3, v3);
        if (index == 4) {
            addTriangle(vertexTree, indexBuffer, positions, normals, a, c, b);
        } else {
            addTriangle(vertexTree, indexBuffer, positions, normals, a, b, c);
        }
    } else if (index == 7 || index == 8) {
        a = interpolate(p3, v3, p0, v0);
        b = interpolate(p3, v3, p2, v2);
        c = interpolate(p3, v3, p1, v1);
        if (index == 7) {
            addTriangle(vertexTree, indexBuffer, positions, normals, a, b, c);
        } else {
            addTriangle(vertexTree, indexBuffer, positions, normals, a, c, b);
        }
    } else if (index == 3 || index == 12) {
        a = interpolate(p0, v0, p2, v2);
        b = interpolate(p1, v1, p3, v3);
        c = interpolate(p0, v0, p3, v3);
        d = interpolate(p1, v1, p2, v2);

        if (index == 3) {
            addTriangle(vertexTree, indexBuffer, positions, normals, a, b, c);
            addTriangle(vertexTree, indexBuffer, positions, normals, a, d, b);
        } else {
            addTriangle(vertexTree, indexBuffer, positions, normals, a, c, b);
            addTriangle(vertexTree, indexBuffer, positions, normals, a, b, d);
        }

    } else if (index == 5 || index == 10) {
        a = interpolate(p2, v2, p3, v3);
        b = interpolate(p0, v0, p1, v1);
        c = interpolate(p0, v0, p3, v3);
        d = interpolate(p1, v1, p2, v2);

        if (index == 5) {
            addTriangle(vertexTree, indexBuffer, positions, normals, a, b, c);
            addTriangle(vertexTree, indexBuffer, positions, normals, a, d, b);
        } else {
            addTriangle(vertexTree, indexBuffer, positions, normals, a, c, b);
            addTriangle(vertexTree, indexBuffer, positions, normals, a, b, d);
        }

    } else if (index == 6 || index == 9) {
        a = interpolate(p1, v1, p3, v3);
        b = interpolate(p0, v0, p2, v2);
        c = interpolate(p0, v0, p1, v1);
        d = interpolate(p2, v2, p3, v3);

        if (index == 6) {
            addTriangle(vertexTree, indexBuffer, positions, normals, a, c, b);
            addTriangle(vertexTree, indexBuffer, positions, normals, a, b, d);
        } else {
            addTriangle(vertexTree, indexBuffer, positions, normals, a, b, c);
            addTriangle(vertexTree, indexBuffer, positions, normals, a, d, b);
        }
    }
}

size_t detail::addVertex(K3DTree<size_t, float> &vertexTree, std::vector<vec3> &positions,
                         std::vector<vec3> &normals, const vec3 pos) {
    K3DTree<size_t, float>::Node *nearest = vertexTree.findNearest(vec3(pos));
    vec3 p;
    if (nearest) {
        p.x = nearest->getPosition()[0];
        p.y = nearest->getPosition()[1];
        p.z = nearest->getPosition()[2];
    }
    if (!nearest || (glm::distance(p, pos) > glm::epsilon<double>() * 5)) {
        nearest = vertexTree.insert(pos, positions.size());
        positions.push_back(pos);
        normals.push_back(vec3(0, 0, 0));
    }
    return nearest->get();
}

void detail::addTriangle(K3DTree<size_t, float> &vertexTree, IndexBufferRAM *indexBuffer,
                         std::vector<vec3> &positions, std::vector<vec3> &normals,
                         const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c) {
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

glm::vec3 detail::interpolate(const glm::vec3 &p0, const double &v0, const glm::vec3 &p1,
                              const double &v1) {
    double t = 0;

    if (v0 != v1) t = v0 / (v0 - v1);

    float tF = static_cast<float>(t);
    return tF * p1 + (1.f - tF) * p0;
}

}  // namespace
