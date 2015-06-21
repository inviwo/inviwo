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

#ifndef IVW_MARCHINGTETRAHEDRON_H
#define IVW_MARCHINGTETRAHEDRON_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>

#include <modules/base/datastructures/kdtree.h>

namespace inviwo {

class IVW_MODULE_BASE_API MarchingTetrahedron {
public:
    static Mesh *apply(const VolumeRepresentation *in, const double &iso, const vec4 &color);
};

namespace detail {
struct IVW_MODULE_BASE_API MarchingTetrahedronDispatcher {
    using type = Mesh *;
    template <class T>
    Mesh *dispatch(const VolumeRepresentation *in, const double &iso, const vec4 &color);
};

template <typename T>
double getValue(const T *src, size3_t pos, size3_t dim, double iso) {
    double v = util::glm_convert<double>(src[VolumeRAM::posToIndex(pos, dim)]);
    return -(v - iso);
}

void evaluateTetra(K3DTree<size_t, float> &vertexTree, IndexBufferRAM *indexBuffer,
                   std::vector<vec3> &positions, std::vector<vec3> &normals, const glm::vec3 &p0,
                   const double &v0, const glm::vec3 &p1, const double &v1, const glm::vec3 &p2,
                   const double &v2, const glm::vec3 &p3, const double &v3);

size_t addVertex(K3DTree<size_t, float> &vertexTree, std::vector<vec3> &positions,
                 std::vector<vec3> &normals, const vec3 pos);

void addTriangle(K3DTree<size_t, float> &vertexTree, IndexBufferRAM *indexBuffer,
                 std::vector<vec3> &positions, std::vector<vec3> &normals, const glm::vec3 &a,
                 const glm::vec3 &b, const glm::vec3 &c);

glm::vec3 interpolate(const glm::vec3 &p0, const double &v0, const glm::vec3 &p1,
                      const double &v1);

template <class DataType>
Mesh *inviwo::detail::MarchingTetrahedronDispatcher::dispatch(const VolumeRepresentation *in,
                                                              const double &iso,
                                                              const vec4 &color) {
    using T = typename DataType::type;

    const VolumeRAMPrecision<T> *volume = dynamic_cast<const VolumeRAMPrecision<T> *>(in);
    if (!volume) return nullptr;

    K3DTree<size_t, float> vertexTree;

    BasicMesh *mesh = new BasicMesh();
    auto indexBuffer = mesh->addIndexBuffer(GeometryEnums::TRIANGLES, GeometryEnums::NONE);

    std::vector<vec3> positions;
    std::vector<vec3> normals;

    const Volume *baseVolume = dynamic_cast<const Volume *>(volume->getOwner());
    if (baseVolume) {
        mesh->setModelMatrix(baseVolume->getModelMatrix());
        mesh->setWorldMatrix(baseVolume->getWorldMatrix());
    }

    const T *src = static_cast<const T *>(volume->getData());

    const size3_t dim{volume->getDimensions()};
    double x, y, z, dx, dy, dz;
    dx = 1.0f / (dim.x - 1);
    dy = 1.0f / (dim.y - 1);
    dz = 1.0f / (dim.z - 1);
    double v[8];
    glm::vec3 p[8];

    const static size_t tetras[6][4] = {
        {0, 1, 3, 5}, {1, 2, 3, 5}, {2, 3, 5, 6}, {0, 3, 4, 5}, {7, 4, 3, 5}, {7, 6, 5, 3}};

    for (size_t k = 0; k < dim.z - 1; k++) {
        for (size_t j = 0; j < dim.y - 1; j++) {
            for (size_t i = 0; i < dim.x - 1; i++) {
                x = dx * i;
                y = dy * j;
                z = dz * k;

                p[0] = glm::vec3(x, y, z);
                p[1] = glm::vec3(x + dx, y, z);
                p[2] = glm::vec3(x + dx, y + dy, z);
                p[3] = glm::vec3(x, y + dy, z);
                p[4] = glm::vec3(x, y, z + dz);
                p[5] = glm::vec3(x + dx, y, z + dz);
                p[6] = glm::vec3(x + dx, y + dy, z + dz);
                p[7] = glm::vec3(x, y + dy, z + dz);

                v[0] = getValue(src, size3_t(i, j, k), dim, iso);
                v[1] = getValue(src, size3_t(i + 1, j, k), dim, iso);
                v[2] = getValue(src, size3_t(i + 1, j + 1, k), dim, iso);
                v[3] = getValue(src, size3_t(i, j + 1, k), dim, iso);
                v[4] = getValue(src, size3_t(i, j, k + 1), dim, iso);
                v[5] = getValue(src, size3_t(i + 1, j, k + 1), dim, iso);
                v[6] = getValue(src, size3_t(i + 1, j + 1, k + 1), dim, iso);
                v[7] = getValue(src, size3_t(i, j + 1, k + 1), dim, iso);

                bool ok = true;
                for (int ii = 0; ii < 8 && ok; ii++) {
                    ok = false;
                    if (v[ii] != v[ii]) break;
                    if (v[ii] == std::numeric_limits<float>::infinity()) break;
                    if (v[ii] == -std::numeric_limits<float>::infinity()) break;
                    if (v[ii] == std::numeric_limits<float>::max()) break;
                    if (v[ii] == std::numeric_limits<float>::min()) break;
                    if (v[ii] == std::numeric_limits<double>::infinity()) break;
                    if (v[ii] == -std::numeric_limits<double>::infinity()) break;
                    if (v[ii] == std::numeric_limits<double>::max()) break;
                    if (v[ii] == std::numeric_limits<double>::min()) break;
                    ok = true;
                }
                if (!ok) continue;

                for (int a = 0; a < 6; a++) {
                    evaluateTetra(vertexTree, indexBuffer, positions, normals, p[tetras[a][0]],
                                  v[tetras[a][0]], p[tetras[a][1]], v[tetras[a][1]],
                                  p[tetras[a][2]], v[tetras[a][2]], p[tetras[a][3]],
                                  v[tetras[a][3]]);
                }
            }
        }
    }

    ivwAssert(positions.size() == normals.size(), "positions_ and normals_ must be equal");
    for (auto pit = positions.begin(), nit = normals.begin(); pit != positions.end();
         ++pit, ++nit) {
        mesh->addVertex(*pit, glm::normalize(*nit), *pit, color);
    }

    return mesh;
}

}  // namespace

}  // namespace

#endif  // IVW_MARCHINGTETRAHEDRON_H
