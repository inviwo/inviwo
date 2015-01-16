/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeoperation.h>
#include <modules/experimental/datastructures/kdtree.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>

namespace inviwo {

class MarchingTetrahedron : public VolumeOperation {
public:
    MarchingTetrahedron(const VolumeRepresentation *in, const double &iso, const vec4 &color)
        : VolumeOperation(in), iso_(iso), color_(color) {}
    virtual ~MarchingTetrahedron() {}

    template <typename T, size_t B>
    void evaluate();

    static inline Geometry* apply(const VolumeRepresentation *in, const double &iso,
                                  const vec4 &color) {
        MarchingTetrahedron marchingOP = MarchingTetrahedron(in, iso, color);
        marchingOP.evaluateFor<MarchingTetrahedron>();
        return marchingOP.getOutput<Geometry>();
    }

private:
    double iso_;
    K3DTree<unsigned> vertexTree_;
    std::vector<vec3> positions_;
    std::vector<vec3> normals_;
    vec4 color_;

    void evaluateTetra(IndexBufferRAM *indexBuffer, const glm::vec3 &p0, const double &v0,
                       const glm::vec3 &p1, const double &v1, const glm::vec3 &p2, const double &v2,
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
                addTriangle(indexBuffer, a, b, c);
            } else {
                addTriangle(indexBuffer, a, c, b);
            }
        } else if (index == 2 || index == 13) {
            a = interpolate(p1, v1, p0, v0);
            b = interpolate(p1, v1, p2, v2);
            c = interpolate(p1, v1, p3, v3);
            if (index == 2) {
                addTriangle(indexBuffer, a, b, c);
            } else {
                addTriangle(indexBuffer, a, c, b);
            }

        } else if (index == 4 || index == 11) {
            a = interpolate(p2, v2, p0, v0);
            b = interpolate(p2, v2, p1, v1);
            c = interpolate(p2, v2, p3, v3);
            if (index == 4) {
                addTriangle(indexBuffer, a, c, b);
            } else {
                addTriangle(indexBuffer, a, b, c);
            }
        } else if (index == 7 || index == 8) {
            a = interpolate(p3, v3, p0, v0);
            b = interpolate(p3, v3, p2, v2);
            c = interpolate(p3, v3, p1, v1);
            if (index == 7) {
                addTriangle(indexBuffer, a, b, c);
            } else {
                addTriangle(indexBuffer, a, c, b);
            }
        } else if (index == 3 || index == 12) {
            a = interpolate(p0, v0, p2, v2);
            b = interpolate(p1, v1, p3, v3);
            c = interpolate(p0, v0, p3, v3);
            d = interpolate(p1, v1, p2, v2);

            if (index == 3) {
                addTriangle(indexBuffer, a, b, c);
                addTriangle(indexBuffer, a, d, b);
            } else {
                addTriangle(indexBuffer, a, c, b);
                addTriangle(indexBuffer, a, b, d);
            }

        } else if (index == 5 || index == 10) {
            a = interpolate(p2, v2, p3, v3);
            b = interpolate(p0, v0, p1, v1);
            c = interpolate(p0, v0, p3, v3);
            d = interpolate(p1, v1, p2, v2);

            if (index == 5) {
                addTriangle(indexBuffer, a, b, c);
                addTriangle(indexBuffer, a, d, b);
            } else {
                addTriangle(indexBuffer, a, c, b);
                addTriangle(indexBuffer, a, b, d);
            }

        } else if (index == 6 || index == 9) {
            a = interpolate(p1, v1, p3, v3);
            b = interpolate(p0, v0, p2, v2);
            c = interpolate(p0, v0, p1, v1);
            d = interpolate(p2, v2, p3, v3);

            if (index == 6) {
                addTriangle(indexBuffer, a, c, b);
                addTriangle(indexBuffer, a, b, d);
            } else {
                addTriangle(indexBuffer, a, b, c);
                addTriangle(indexBuffer, a, d, b);
            }
        }
    }

    unsigned addVertex(const vec3 pos) {
        K3DTree<unsigned>::Node *nearest = vertexTree_.findNearest(vec3(pos));
        vec3 p;
        if (nearest) {
            p.x = nearest->getPosition()[0];
            p.y = nearest->getPosition()[1];
            p.z = nearest->getPosition()[2];
        }
        if (!nearest || (glm::distance(p, pos) > glm::epsilon<double>() * 5)) {
            nearest = vertexTree_.insert(vec3(pos), static_cast<unsigned>(positions_.size()));
            positions_.push_back(pos);
            normals_.push_back(vec3(0, 0, 0));
        }
        return nearest->get();
    }

    void addTriangle(IndexBufferRAM *indexBuffer, const glm::vec3 &a, const glm::vec3 &b,
                     const glm::vec3 &c) {
        unsigned i0 = addVertex(a);
        unsigned i1 = addVertex(b);
        unsigned i2 = addVertex(c);

        if (i0 == i1 || i0 == i2 || i1 == i2) {
            // triangle is so small so that the vertices are merged.
            return;
        }

        indexBuffer->add(i0);
        indexBuffer->add(i1);
        indexBuffer->add(i2);

        vec3 e0 = b - a;
        vec3 e1 = c - a;
        vec3 n = glm::normalize(glm::cross(e0, e1));

        normals_[i0] += n;
        normals_[i1] += n;
        normals_[i2] += n;
    }

    template <typename T>
    double getValue(const T *src, uvec3 pos, uvec3 dim);

    template <typename T>
    static double toSingle(const glm::detail::tvec2<T, glm::defaultp> &v) {
        return v.x;
    }
    template <typename T>
    static double toSingle(const glm::detail::tvec3<T, glm::defaultp> &v) {
        return v.x;
    }
    template <typename T>
    static double toSingle(const glm::detail::tvec4<T, glm::defaultp> &v) {
        return v.x;
    }

    template <typename T>
    static double toSingle(const T &v) {
        return v;
    }

    static glm::vec3 interpolate(const glm::vec3 &p0, const double &v0, const glm::vec3 &p1,
                                 const double &v1) {
        float t = 0;
        if (v0 != v1) t = v0 / (v0 - v1);
        return t * p1 + (1 - t) * p0;
    }
};

template <typename T>
double MarchingTetrahedron::getValue(const T *src, uvec3 pos, uvec3 dim) {
    double v = toSingle(src[VolumeRAM::posToIndex(pos, dim)]);
    return -(v - iso_);
}

template <typename T, size_t B>
void MarchingTetrahedron::evaluate() {
    const VolumeRAMPrecision<T> *volume =
        dynamic_cast<const VolumeRAMPrecision<T> *>(getInputVolume());

    if (!volume) {
        setOutput(NULL);
        return;
    }

    vertexTree_.clear();
    positions_.clear();
    normals_.clear();

    BasicMesh *mesh = new BasicMesh();
    IndexBufferRAM *indexBuffer =
        mesh->addIndexBuffer(GeometryEnums::TRIANGLES, GeometryEnums::NONE);

    const Volume *baseVolume = dynamic_cast<const Volume *>(volume->getOwner());
    if (baseVolume) {
        mesh->setModelMatrix(baseVolume->getModelMatrix());
        mesh->setWorldMatrix(baseVolume->getWorldMatrix());
    }
    setOutput(mesh);

    const T *src = reinterpret_cast<const T *>(volume->getData());

    uvec3 dim = volume->getDimensions();
    double x, y, z, dx, dy, dz;
    dx = 1.0f / (dim.x - 1);
    dy = 1.0f / (dim.y - 1);
    dz = 1.0f / (dim.z - 1);
    double v[8];
    glm::vec3 p[8];

    const static unsigned int tetras[6][4] = {
        {0, 1, 3, 5}, {1, 2, 3, 5}, {2, 3, 5, 6}, {0, 3, 4, 5}, {7, 4, 3, 5}, {7, 6, 5, 3}};

    for (unsigned k = 0; k < dim.z - 1; k++) {
        for (unsigned j = 0; j < dim.y - 1; j++) {
            for (unsigned i = 0; i < dim.x - 1; i++) {
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

                v[0] = getValue(src, uvec3(i, j, k), dim);
                v[1] = getValue(src, uvec3(i + 1, j, k), dim);
                v[2] = getValue(src, uvec3(i + 1, j + 1, k), dim);
                v[3] = getValue(src, uvec3(i, j + 1, k), dim);
                v[4] = getValue(src, uvec3(i, j, k + 1), dim);
                v[5] = getValue(src, uvec3(i + 1, j, k + 1), dim);
                v[6] = getValue(src, uvec3(i + 1, j + 1, k + 1), dim);
                v[7] = getValue(src, uvec3(i, j + 1, k + 1), dim);

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
                    evaluateTetra(indexBuffer, p[tetras[a][0]], v[tetras[a][0]], p[tetras[a][1]],
                                  v[tetras[a][1]], p[tetras[a][2]], v[tetras[a][2]],
                                  p[tetras[a][3]], v[tetras[a][3]]);
                }
            }
        }
    }

    ivwAssert(positions_.size() == normals_.size(), "positions_ and normals_ must be equal");
    std::vector<vec3>::iterator P = positions_.begin();
    std::vector<vec3>::iterator N = normals_.begin();
    for (; P != positions_.end(); ++P, ++N) {
        mesh->addVertex(*P, glm::normalize(*N), *P, color_);
    }
}

}  // namespace

#endif  // IVW_MARCHINGTETRAHEDRON_H
