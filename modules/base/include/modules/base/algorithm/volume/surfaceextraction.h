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

#ifndef IVW_SURFACEEXTRACTIONUTIL_H
#define IVW_SURFACEEXTRACTIONUTIL_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>

#include <modules/base/datastructures/kdtree.h>

namespace inviwo {
/*
Common functions used in MarchingCubes and MarchingTetrahedron
*/
namespace marching {
template <typename T>
double getValue(const T *src, size3_t pos, size3_t dim, double iso, bool invert) {
    double v = util::glm_convert<double>(src[VolumeRAM::posToIndex(pos, dim)]);
    return invert ? v - iso : -(v - iso);
}

glm::vec3 interpolate(const glm::vec3 &p0, double v0, const glm::vec3 &p1, double v1);

void evaluateTriangle(K3DTree<size_t, float> &vertexTree, IndexBufferRAM *indexBuffer,
                      std::vector<vec3> &positions, std::vector<vec3> &normals, const glm::vec3 &p0,
                      double v0, const glm::vec3 &p1, double v1, const glm::vec3 &p2, double v2);

size_t addVertex(K3DTree<size_t, float> &vertexTree, std::vector<vec3> &positions,
                 std::vector<vec3> &normals, const vec3 pos);

void addTriangle(K3DTree<size_t, float> &vertexTree, IndexBufferRAM *indexBuffer,
                 std::vector<vec3> &positions, std::vector<vec3> &normals, const glm::vec3 &a,
                 const glm::vec3 &b, const glm::vec3 &c);

template <typename T>
void encloseSurfce(const T *src, const size3_t &dim, IndexBufferRAM *indexBuffer,
                   std::vector<vec3> &positions, std::vector<vec3> &normals, double iso,
                   bool invert, double dx, double dy, double dz) {
    auto cubeEdgeIndices = [](size_t n) -> std::vector<size_t> {
        if (n == 1) return {size_t(0)};
        return {size_t(0), n - 1};
    };

    std::array<vec3, 4> pos;
    std::array<double, 4> values;

    {
        K3DTree<size_t, float> sideVertexTree;
        // Z axis
        for (auto &k : cubeEdgeIndices(dim.z)) {
            for (size_t j = 0; j < dim.y - 1; ++j) {
                for (size_t i = 0; i < dim.x - 1; ++i) {
                    double x = dx * i;
                    double y = dy * j;
                    double z = dz * k;

                    pos[0] = glm::vec3(x, y, z);
                    pos[1] = glm::vec3(x + dx, y, z);
                    pos[2] = glm::vec3(x + dx, y + dy, z);
                    pos[3] = glm::vec3(x, y + dy, z);

                    values[0] = marching::getValue(src, size3_t(i, j, k), dim, iso, invert);
                    values[1] = marching::getValue(src, size3_t(i + 1, j, k), dim, iso, invert);
                    values[2] = marching::getValue(src, size3_t(i + 1, j + 1, k), dim, iso, invert);
                    values[3] = marching::getValue(src, size3_t(i, j + 1, k), dim, iso, invert);

                    if (k == 0) {
                        evaluateTriangle(sideVertexTree, indexBuffer, positions, normals, pos[0],
                                         values[0], pos[3], values[3], pos[1], values[1]);
                        evaluateTriangle(sideVertexTree, indexBuffer, positions, normals, pos[1],
                                         values[1], pos[3], values[3], pos[2], values[2]);
                    } else {
                        evaluateTriangle(sideVertexTree, indexBuffer, positions, normals, pos[0],
                                         values[0], pos[1], values[1], pos[3], values[3]);
                        evaluateTriangle(sideVertexTree, indexBuffer, positions, normals, pos[1],
                                         values[1], pos[2], values[2], pos[3], values[3]);
                    }
                }
            }
        }
    }
    {
        K3DTree<size_t, float> sideVertexTree;
        // Y axis
        for (size_t k = 0; k < dim.z - 1; ++k) {
            for (auto &j : cubeEdgeIndices(dim.y)) {
                for (size_t i = 0; i < dim.x - 1; ++i) {
                    double x = dx * i;
                    double y = dy * j;
                    double z = dz * k;

                    pos[0] = glm::vec3(x, y, z);
                    pos[1] = glm::vec3(x + dx, y, z);
                    pos[2] = glm::vec3(x + dx, y, z + dz);
                    pos[3] = glm::vec3(x, y, z + dz);

                    values[0] = marching::getValue(src, size3_t(i, j, k), dim, iso, invert);
                    values[1] = marching::getValue(src, size3_t(i + 1, j, k), dim, iso, invert);
                    values[2] = marching::getValue(src, size3_t(i + 1, j, k + 1), dim, iso, invert);
                    values[3] = marching::getValue(src, size3_t(i, j, k + 1), dim, iso, invert);

                    if (j == 0) {
                        evaluateTriangle(sideVertexTree, indexBuffer, positions, normals, pos[0],
                                         values[0], pos[1], values[1], pos[2], values[2]);
                        evaluateTriangle(sideVertexTree, indexBuffer, positions, normals, pos[0],
                                         values[0], pos[2], values[2], pos[3], values[3]);
                    } else {
                        evaluateTriangle(sideVertexTree, indexBuffer, positions, normals, pos[0],
                                         values[0], pos[2], values[2], pos[1], values[1]);
                        evaluateTriangle(sideVertexTree, indexBuffer, positions, normals, pos[0],
                                         values[0], pos[3], values[3], pos[2], values[2]);
                    }
                }
            }
        }
    }
    {
        K3DTree<size_t, float> sideVertexTree;
        // X axis
        for (size_t k = 0; k < dim.z - 1; ++k) {
            for (size_t j = 0; j < dim.y - 1; ++j) {
                for (auto &i : cubeEdgeIndices(dim.x)) {
                    double x = dx * i;
                    double y = dy * j;
                    double z = dz * k;

                    pos[0] = glm::vec3(x, y, z);
                    pos[1] = glm::vec3(x, y + dy, z);
                    pos[2] = glm::vec3(x, y + dy, z + dz);
                    pos[3] = glm::vec3(x, y, z + dz);

                    values[0] = marching::getValue(src, size3_t(i, j, k), dim, iso, invert);
                    values[1] = marching::getValue(src, size3_t(i, j + 1, k), dim, iso, invert);
                    values[2] = marching::getValue(src, size3_t(i, j + 1, k + 1), dim, iso, invert);
                    values[3] = marching::getValue(src, size3_t(i, j, k + 1), dim, iso, invert);

                    if (i == 0) {
                        evaluateTriangle(sideVertexTree, indexBuffer, positions, normals, pos[0],
                                         values[0], pos[3], values[3], pos[1], values[1]);
                        evaluateTriangle(sideVertexTree, indexBuffer, positions, normals, pos[1],
                                         values[1], pos[3], values[3], pos[2], values[2]);
                    } else {
                        evaluateTriangle(sideVertexTree, indexBuffer, positions, normals, pos[0],
                                         values[0], pos[1], values[1], pos[3], values[3]);
                        evaluateTriangle(sideVertexTree, indexBuffer, positions, normals, pos[1],
                                         values[1], pos[2], values[2], pos[3], values[3]);
                    }
                }
            }
        }
    }
}

}  // namespace marching

}  // namespace inviwo

#endif  // IVW_SURFACEEXTRACTIONUTIL_H
