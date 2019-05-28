/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <modules/base/algorithm/volume/marchingtetrahedron.h>
#include <modules/base/algorithm/volume/surfaceextraction.h>

namespace inviwo {

std::shared_ptr<Mesh> MarchingTetrahedron::apply(
    std::shared_ptr<const Volume> volume, double iso, const vec4 &color, bool invert, bool enclose,
    std::function<void(float)> progressCallback,
    std::function<bool(const size3_t &)> maskingCallback) {
    LogWarnCustom("MarchingTetrahedron::apply",
                  "Deprecated: Use util::marchingtetrahedron(...) instead");
    return util::marchingtetrahedron(volume, iso, color, invert, enclose, progressCallback,
                                     maskingCallback);
}

namespace marchingtetrahedron {

//  v7 ----- v6
//  /|       /|
// v3 ----- v2|
// |v4 - - -|v5
// |/       |/
// v0 ----- v1

const static std::array<size3_t, 8> offs = {size3_t{0, 0, 0}, size3_t{1, 0, 0}, size3_t{1, 1, 0},
                                            size3_t{0, 1, 0}, size3_t{0, 0, 1}, size3_t{1, 0, 1},
                                            size3_t{1, 1, 1}, size3_t{0, 1, 1}};

const static std::array<std::array<size_t, 4>, 6> tetras = {
    std::array<size_t, 4>{0, 1, 3, 5}, std::array<size_t, 4>{1, 2, 3, 5},
    std::array<size_t, 4>{2, 3, 5, 6}, std::array<size_t, 4>{0, 3, 4, 5},
    std::array<size_t, 4>{7, 4, 3, 5}, std::array<size_t, 4>{7, 6, 5, 3}};

void evaluateTetra(K3DTree<size_t, float> &vertexTree, IndexBufferRAM *indexBuffer,
                   std::vector<vec3> &positions, std::vector<vec3> &normals, const glm::vec3 &p0,
                   double v0, const glm::vec3 &p1, double v1, const glm::vec3 &p2, double v2,
                   const glm::vec3 &p3, double v3) {
    int index = 0;
    if (v0 > 0) index = index | 1;
    if (v1 > 0) index = index | 2;
    if (v2 > 0) index = index | 4;
    if (v3 > 0) index = index | 8;
    glm::vec3 a, b, c, d;
    if (index == 0 || index == 15) return;
    if (index == 1 || index == 14) {
        a = marching::interpolate(p0, v0, p2, v2);
        b = marching::interpolate(p0, v0, p1, v1);
        c = marching::interpolate(p0, v0, p3, v3);
        if (index == 1) {
            marching::addTriangle(vertexTree, indexBuffer, positions, normals, a, b, c);
        } else {
            marching::addTriangle(vertexTree, indexBuffer, positions, normals, a, c, b);
        }
    } else if (index == 2 || index == 13) {
        a = marching::interpolate(p1, v1, p0, v0);
        b = marching::interpolate(p1, v1, p2, v2);
        c = marching::interpolate(p1, v1, p3, v3);
        if (index == 2) {
            marching::addTriangle(vertexTree, indexBuffer, positions, normals, a, b, c);
        } else {
            marching::addTriangle(vertexTree, indexBuffer, positions, normals, a, c, b);
        }

    } else if (index == 4 || index == 11) {
        a = marching::interpolate(p2, v2, p0, v0);
        b = marching::interpolate(p2, v2, p1, v1);
        c = marching::interpolate(p2, v2, p3, v3);
        if (index == 4) {
            marching::addTriangle(vertexTree, indexBuffer, positions, normals, a, c, b);
        } else {
            marching::addTriangle(vertexTree, indexBuffer, positions, normals, a, b, c);
        }
    } else if (index == 7 || index == 8) {
        a = marching::interpolate(p3, v3, p0, v0);
        b = marching::interpolate(p3, v3, p2, v2);
        c = marching::interpolate(p3, v3, p1, v1);
        if (index == 7) {
            marching::addTriangle(vertexTree, indexBuffer, positions, normals, a, b, c);
        } else {
            marching::addTriangle(vertexTree, indexBuffer, positions, normals, a, c, b);
        }
    } else if (index == 3 || index == 12) {
        a = marching::interpolate(p0, v0, p2, v2);
        b = marching::interpolate(p1, v1, p3, v3);
        c = marching::interpolate(p0, v0, p3, v3);
        d = marching::interpolate(p1, v1, p2, v2);

        if (index == 3) {
            marching::addTriangle(vertexTree, indexBuffer, positions, normals, a, b, c);
            marching::addTriangle(vertexTree, indexBuffer, positions, normals, a, d, b);
        } else {
            marching::addTriangle(vertexTree, indexBuffer, positions, normals, a, c, b);
            marching::addTriangle(vertexTree, indexBuffer, positions, normals, a, b, d);
        }

    } else if (index == 5 || index == 10) {
        a = marching::interpolate(p2, v2, p3, v3);
        b = marching::interpolate(p0, v0, p1, v1);
        c = marching::interpolate(p0, v0, p3, v3);
        d = marching::interpolate(p1, v1, p2, v2);

        if (index == 5) {
            marching::addTriangle(vertexTree, indexBuffer, positions, normals, a, b, c);
            marching::addTriangle(vertexTree, indexBuffer, positions, normals, a, d, b);
        } else {
            marching::addTriangle(vertexTree, indexBuffer, positions, normals, a, c, b);
            marching::addTriangle(vertexTree, indexBuffer, positions, normals, a, b, d);
        }

    } else if (index == 6 || index == 9) {
        a = marching::interpolate(p1, v1, p3, v3);
        b = marching::interpolate(p0, v0, p2, v2);
        c = marching::interpolate(p0, v0, p1, v1);
        d = marching::interpolate(p2, v2, p3, v3);

        if (index == 6) {
            marching::addTriangle(vertexTree, indexBuffer, positions, normals, a, c, b);
            marching::addTriangle(vertexTree, indexBuffer, positions, normals, a, b, d);
        } else {
            marching::addTriangle(vertexTree, indexBuffer, positions, normals, a, b, c);
            marching::addTriangle(vertexTree, indexBuffer, positions, normals, a, d, b);
        }
    }
}
}  // namespace marchingtetrahedron

namespace util {
std::shared_ptr<Mesh> marchingtetrahedron(std::shared_ptr<const Volume> volume, double iso,
                                          const vec4 &color, bool invert, bool enclose,
                                          std::function<void(float)> progressCallback,
                                          std::function<bool(const size3_t &)> maskingCallback) {

    return volume->getRepresentation<VolumeRAM>()->dispatch<std::shared_ptr<Mesh>>([&](auto ram) {
        using T = util::PrecisionValueType<decltype(ram)>;
        if (progressCallback) progressCallback(0.0f);

        if (!maskingCallback) {
            throw Exception("Masking callback not set",
                            IVW_CONTEXT_CUSTOM("util::marchingtetrahedron"));
        }

        K3DTree<size_t, float> vertexTree;

        auto mesh = std::make_shared<BasicMesh>();
        auto indexBuffer = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);

        std::vector<vec3> positions;
        std::vector<vec3> normals;

        mesh->setModelMatrix(volume->getModelMatrix());
        mesh->setWorldMatrix(volume->getWorldMatrix());

        const T *src = ram->getDataTyped();

        const size3_t dim{volume->getDimensions()};
        double dx, dy, dz;
        dx = 1.0 / static_cast<double>(std::max(size_t(1), (dim.x - 1)));
        dy = 1.0 / static_cast<double>(std::max(size_t(1), (dim.y - 1)));
        dz = 1.0 / static_cast<double>(std::max(size_t(1), (dim.z - 1)));

        const auto volSize = dim.x * dim.y * dim.z;
        indexBuffer->getDataContainer().reserve(volSize * 6);
        positions.reserve(volSize * 6);
        normals.reserve(volSize * 6);

        for (size_t k = 0; k < dim.z - 1; k++) {
            for (size_t j = 0; j < dim.y - 1; j++) {
                for (size_t i = 0; i < dim.x - 1; i++) {
                    if (!maskingCallback({i, j, k})) continue;
                    double x = dx * i;
                    double y = dy * j;
                    double z = dz * k;

                    std::array<vec3, 8> pos;
                    std::array<double, 8> values;

                    for (int l = 0; l < 8; l++) {
                        const auto &o = marchingtetrahedron::offs[l];
                        pos[l] = glm::vec3(x + dx * o.x, y + dy * o.y, z + dz * o.z);
                        values[l] = marching::getValue(src, size3_t(i, j, k) + o, dim, iso, invert);
                    }

                    for (auto &t : marchingtetrahedron::tetras) {
                        marchingtetrahedron::evaluateTetra(vertexTree, indexBuffer.get(), positions,
                                                           normals, pos[t[0]], values[t[0]],
                                                           pos[t[1]], values[t[1]], pos[t[2]],
                                                           values[t[2]], pos[t[3]], values[t[3]]);
                    }
                }
            }
            if (progressCallback) {
                progressCallback(static_cast<float>(k + 1) / static_cast<float>(dim.z - 1));
            }
        }

        if (enclose) {
            marching::encloseSurfce(src, dim, indexBuffer.get(), positions, normals, iso, invert,
                                    dx, dy, dz);
        }

        ivwAssert(positions.size() == normals.size(), "positions_ and normals_ must be equal");
        std::vector<BasicMesh::Vertex> vertices;
        vertices.reserve(positions.size());

        for (auto pit = positions.begin(), nit = normals.begin(); pit != positions.end();
             ++pit, ++nit) {
            vertices.push_back({*pit, glm::normalize(*nit), *pit, color});
        }

        mesh->addVertices(vertices);

        if (progressCallback) progressCallback(1.0f);

        return mesh;
    });
}
}  // namespace util

}  // namespace inviwo
