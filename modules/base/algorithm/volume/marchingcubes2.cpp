/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2017 Inviwo Foundation
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

#include <modules/base/algorithm/volume/marchingcubes2.h>
#include <modules/base/algorithm/volume/surfaceextraction.h>
#include <inviwo/core/util/indexmapper.h>

#include <modules/base/datastructures/disjointsets.h>
#include <glm/gtx/normal.hpp>

#include <algorithm>
#include <limits>
#include <bitset>

namespace inviwo {

std::vector<marching::Config::Triangle> marching::Config::calcTriangles(std::bitset<8> corners,
                                                                        bool flip) {
    if (corners.count() > 4) {
        return calcTriangles(~corners, true);
    }
    std::unordered_map<NodeId, std::vector<NodeId>> edgeMap;
    DisjointSets groups(8);

    // Check which edges that are involved add group neighboring nodes
    for (NodeId i = 0; i < 8; ++i) {
        if (corners[i]) {
            for (NodeId j : nodeNeighbours[i]) {
                if (!corners[j]) {
                    edgeMap[i].push_back(j);
                } else {
                    groups.join(i, j);
                }
            }
        }
    }

    // Create groups of neighboring edges and nodes
    std::unordered_map<NodeId, std::vector<EdgeId>> edgeGroups;
    std::unordered_map<NodeId, std::vector<NodeId>> nodeGroups;
    for (auto item : edgeMap) {
        NodeId i = item.first;
        NodeId groupId = groups.find(i);
        nodeGroups[groupId].push_back(i);
        for (NodeId j : item.second) {
            edgeGroups[groupId].push_back(nodeIdsToEdgeId[i][j]);
        }
    }

    std::vector<Triangle> triangles;
    for (auto item : edgeGroups) {
        NodeId groupId = item.first;
        std::vector<EdgeId> &edgeGroup = item.second;

        // Sort edges in a circle by following the cube faces, needed for the triangulation
        auto it = edgeGroup.begin();
        while (it + 1 != edgeGroup.end()) {
            // find an edge that share a face with *it
            auto nit = std::find_if(it + 1, edgeGroup.end(), [&](auto edge) {
                auto &faces1 = edgeIdToFaceIds[*it];
                auto &faces2 = edgeIdToFaceIds[edge];
                return faces1[0] == faces2[0] || faces1[0] == faces2[1] || faces1[1] == faces2[0] ||
                       faces1[1] == faces2[1];
            });
            std::iter_swap(it + 1, nit);
            ++it;
        }
        dvec3 groupNormal{0.0};
        for (NodeId i : nodeGroups[groupId]) {
            groupNormal += dvec3{0.5, 0.5, 0.5} - dvec3(vertices[i]);
        }

        // apply the triangulations
        for (auto &triangle : triangulations.at(edgeGroup.size())) {
            auto e0 = edgeGroup[triangle[0]];
            auto e1 = edgeGroup[triangle[1]];
            auto e2 = edgeGroup[triangle[2]];

            auto p0 = 0.5 * (dvec3(vertices[edges[e0][0]]) + dvec3(vertices[edges[e0][1]]));
            auto p1 = 0.5 * (dvec3(vertices[edges[e1][0]]) + dvec3(vertices[edges[e1][1]]));
            auto p2 = 0.5 * (dvec3(vertices[edges[e2][0]]) + dvec3(vertices[edges[e2][1]]));

            // check that the orientation is right
            auto triNormal = glm::triangleNormal(p0, p1, p2);
            auto proj = glm::dot(groupNormal, triNormal);
            if (flip ? proj > 0.0 : proj < 0.0) {
                triangles.push_back({e0, e1, e2});
            } else {
                triangles.push_back({e2, e1, e0});
            }
        }
    }

    return triangles;
}

namespace {

class Cache {
public:
    Cache(const size2_t &dim) : cIm{dim} {}

    inline std::pair<size_t, bool> find(const size3_t &ind, int edge, const size_t &val) {

        const auto get = [](auto &map, const size_t &key,
                            const size_t &val) -> std::pair<size_t, bool> {
            auto res = map.insert({key, val});
            return {res.first->second, res.second};
        };

        switch (edge) {
            case 0:
                return get(xCacheCurr, cIm(ind.x, ind.y), val);
            case 1:
                return get(yCacheCurr, cIm(ind.x + 1, ind.y), val);
            case 2:
                return get(xCacheCurr, cIm(ind.x, ind.y + 1), val);
            case 3:
                return get(yCacheCurr, cIm(ind.x, ind.y), val);
            case 4:
                return get(zCacheCurr, ind.x, val);
            case 5:
                return get(zCacheCurr, ind.x + 1, val);
            case 6:
                return get(zCacheNext, ind.x + 1, val);
            case 7:
                return get(zCacheNext, ind.x, val);
            case 8:
                return get(xCacheNext, cIm(ind.x, ind.y), val);
            case 9:
                return get(yCacheNext, cIm(ind.x + 1, ind.y), val);
            case 10:
                return get(xCacheNext, cIm(ind.x, ind.y + 1), val);
            case 11:
                return get(yCacheNext, cIm(ind.x, ind.y), val);
            default:
                throw Exception("Invalid edge");
        }
    }

    void incZ() {
        xCacheCurr.clear();
        std::swap(xCacheCurr, xCacheNext);
        yCacheCurr.clear();
        std::swap(yCacheCurr, yCacheNext);
        zCacheNext.clear();
    }
    void incY() {
        zCacheCurr.clear();
        std::swap(zCacheCurr, zCacheNext);
    }

private:
    struct CheapHash {
        std::size_t operator()(size_t const &s) const noexcept { return s; }
    };

    util::IndexMapper2D cIm;

    using Map = std::unordered_map<size_t, size_t, CheapHash>;

    Map xCacheCurr;
    Map xCacheNext;
    Map yCacheCurr;
    Map yCacheNext;
    Map zCacheCurr;
    Map zCacheNext;
};

struct OffsetIndexMasks {
    OffsetIndexMasks(int next, int curr, size3_t off)
        : nextMask{next}, currMask{curr}, offset{off} {}
    const int nextMask;
    const int currMask;
    const size3_t offset;
};

template <typename T, typename IsoTest>
class Index {
public:
    Index(const T *src, const util::IndexMapper3D &im, const T &iso, const IsoTest &test)
        : offsets0_{[&]() {
            std::array<size_t, 4> tmp;
            std::transform(oim_.begin(), oim_.end(), tmp.begin(),
                           [&](const auto &item) { return im(item.offset); });
            return tmp;
        }()}
        , offsets1_{[&]() {
            std::array<size_t, 4> tmp;
            std::transform(oim_.begin(), oim_.end(), tmp.begin(), [&](const auto &item) {
                return im(item.offset + size3_t{1, 0, 0});
            });
            return tmp;
        }()}
        , src_{src}
        , iso_{iso}
        , test_{test} {}

    void init(const size_t &gridIndex) {
        int next = 0;
        for (int v = 0; v < 4; ++v) {
            const auto val = src_[gridIndex + offsets0_[v]];
            if (test_(val, iso_)) {
                next |= 1 << oim_[v].nextMask;
            }
        }
        next_ = next;
    }

    void update(const size_t &gridIndex) {
        int curr = next_;
        int next = 0;
        for (int v = 0; v < 4; ++v) {
            const auto val = src_[gridIndex + offsets1_[v]];
            if (test_(val, iso_)) {
                next |= 1 << oim_[v].nextMask;
                curr |= 1 << oim_[v].currMask;
            }
        }
        next_ = next;
        curr_ = curr;
    }

    operator size_t() { return curr_; }

private:
    static const std::array<OffsetIndexMasks, 4> oim_;
    const std::array<size_t, 4> offsets0_;
    const std::array<size_t, 4> offsets1_;

    const T *src_;
    const T iso_;
    const IsoTest &test_;
    int next_;
    int curr_;
};

template <typename T, typename IsoTest>
const std::array<OffsetIndexMasks, 4> Index<T, IsoTest>::oim_ = {
    {{0, 1, {0, 0, 0}}, {3, 2, {0, 1, 0}}, {4, 5, {0, 0, 1}}, {7, 6, {0, 1, 1}}}};

}  // namespace

namespace util {
std::shared_ptr<Mesh> marchingcubes2(std::shared_ptr<const Volume> volume, double iso,
                                     const vec4 &color, bool invert, bool enclose,
                                     std::function<void(float)> progressCallback,
                                     std::function<bool(const size3_t &)> maskingCallback) {

    auto indexBuffer = std::make_shared<IndexBuffer>();
    auto vertexBuffer = std::make_shared<Buffer<vec3>>();
    auto textureBuffer = std::make_shared<Buffer<vec3>>();
    auto colorBuffer = std::make_shared<Buffer<vec4>>();
    auto normalBuffer = std::make_shared<Buffer<vec3>>();

    auto indexRAM = indexBuffer->getEditableRAMRepresentation();
    auto &positions = vertexBuffer->getEditableRAMRepresentation()->getDataContainer();
    auto &textures = textureBuffer->getEditableRAMRepresentation()->getDataContainer();
    auto &colors = colorBuffer->getEditableRAMRepresentation()->getDataContainer();
    auto &normals = normalBuffer->getEditableRAMRepresentation()->getDataContainer();

    if (progressCallback) progressCallback(0.0f);

    const auto mc = [&](auto ram, auto isoTest, auto mapValue) {
        using T = util::PrecsionValueType<decltype(ram)>;
        static const marching::Config cube{};

        const T *src = ram->getDataTyped();
        const size3_t dim{volume->getDimensions()};
        const size3_t dim1 = dim - size3_t{1, 1, 1};
        const util::IndexMapper3D im(dim);

        const auto dr = dvec3(1.0) / dvec3{glm::max(size3_t{1}, (dim - size3_t{1}))};
        const auto doffs = [&]() {
            std::array<dvec3, 8> tmp;
            std::transform(cube.vertices.begin(), cube.vertices.end(), tmp.begin(),
                           [dr](auto &v) { return dr * dvec3{v}; });
            return tmp;
        }();

        const auto interpolate = [src, im, &mapValue, &doffs](const size3_t &ind, const dvec3 &pos,
                                                              marching::Config::EdgeId e) {
            const auto a = cube.edges[e][0];
            const auto b = cube.edges[e][1];
            auto v0 = util::glm_convert<double>(src[im(ind + cube.vertices[a])]);
            v0 = mapValue(v0);
            auto v1 = util::glm_convert<double>(src[im(ind + cube.vertices[b])]);
            v1 = mapValue(v1);

            const auto t = v0 / (v0 - v1);
            const auto r0 = pos + doffs[a];
            const auto r1 = pos + doffs[b];
            return r0 + t * (r1 - r0);
        };

        Cache cache(size2_t{dim.x, dim.y});
        Index<T, decltype(isoTest)> index(src, im, util::glm_convert<T>(iso), isoTest);
        size3_t ind;
        dvec3 pos;
        std::array<dvec3, 3> verts;
        for (ind.z = 0, pos.z = 0.0; ind.z < dim1.z; ++ind.z, pos.z += dr.z) {
            cache.incZ();
            for (ind.y = 0, pos.y = 0.0; ind.y < dim1.y; ++ind.y, pos.y += dr.y) {
                ind.x = 0;
                const auto cInd = im(ind);
                cache.incY();
                index.init(cInd);
                for (pos.x = 0.0; ind.x < dim1.x; ++ind.x, pos.x += dr.x) {
                    index.update(cInd + ind.x);

                    if (maskingCallback && !maskingCallback(ind)) continue;

                    for (const auto &tri : cube.cases[index]) {
                        std::transform(tri.begin(), tri.end(), verts.begin(),
                                     [&](auto e) { return interpolate(ind, pos, e); });

                        auto n = glm::cross(verts[1] - verts[0], verts[2] - verts[0]);
                        if (glm::length2(n) < glm::epsilon<double>()) {
                            // triangle is so small area is 0.
                            continue;
                        }
                        n = glm::normalize(n);

                        for (int v = 0; v < 3; ++v) {
                            auto c = cache.find(ind, t[v], positions.size());
                            if (c.second) {
                                positions.emplace_back(verts[v]);
                                normals.emplace_back(0.0f, 0.0f, 0.0f);
                            }
                            const auto i = c.first;

                            indexRAM->add(static_cast<uint32_t>(i));
                            normals[i] += n;
                        }
                    }
                }
            }
            if (progressCallback) {
                progressCallback(static_cast<float>(ind.z + 1) / static_cast<float>(dim.z - 1));
            }
        }

        if (enclose) {
            marching::encloseSurfce(src, dim, indexRAM, positions, normals, iso, invert, dr.x, dr.y,
                                    dr.z);
        }
    };
    if (invert) {
        volume->getRepresentation<VolumeRAM>()->dispatch<void, dispatching::filter::Scalars>(
            [&](auto ram) {
                mc(ram, [](auto &&val, auto &&iso) { return val > iso; },
                   [iso](const double &val) { return val - iso; });
            });
    } else {
        volume->getRepresentation<VolumeRAM>()->dispatch<void, dispatching::filter::Scalars>(
            [&](auto ram) {
                mc(ram, [](auto &&val, auto &&iso) { return val < iso; },
                   [iso](const double &val) { return -(val - iso); });
            });
    }

    ivwAssert(positions.size() == normals.size(), "positions and normals must be equal size");

    std::transform(normals.begin(), normals.end(), normals.begin(),
                   [](const vec3 &n) { return glm::normalize(n); });
    textures.insert(textures.begin(), positions.begin(), positions.end());
    colors.reserve(positions.size());
    std::fill_n(std::back_inserter(colors), positions.size(), color);

    auto mesh = std::make_shared<Mesh>();
    mesh->setModelMatrix(volume->getModelMatrix());
    mesh->setWorldMatrix(volume->getWorldMatrix());
    mesh->addIndicies({DrawType::Triangles, ConnectivityType::None}, indexBuffer);
    mesh->addBuffer(BufferType::PositionAttrib, vertexBuffer);
    mesh->addBuffer(BufferType::TexcoordAttrib, textureBuffer);
    mesh->addBuffer(BufferType::ColorAttrib, colorBuffer);
    mesh->addBuffer(BufferType::NormalAttrib, normalBuffer);

    if (progressCallback) progressCallback(1.0f);

    return mesh;
}
}  // namespace util

}  // namespace inviwo
