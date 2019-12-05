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

#include <modules/base/algorithm/volume/marchingcubesopt.h>
#include <modules/base/algorithm/volume/surfaceextraction.h>
#include <inviwo/core/util/indexmapper.h>

#include <modules/base/datastructures/disjointsets.h>
#include <glm/gtx/normal.hpp>

#include <algorithm>
#include <limits>
#include <bitset>

namespace inviwo {

marching::Config::Config()
    : vertices{{{0, 0, 0},
                {1, 0, 0},
                {1, 1, 0},
                {0, 1, 0},
                {0, 0, 1},
                {1, 0, 1},
                {1, 1, 1},
                {0, 1, 1}}}
    , edges{{{0, 1},
             {1, 2},
             {2, 3},
             {3, 0},
             {0, 4},
             {1, 5},
             {2, 6},
             {3, 7},
             {4, 5},
             {5, 6},
             {6, 7},
             {7, 4}}}
    , faces{{{0, 1, 2, 3},
             {0, 5, 8, 4},
             {1, 6, 9, 5},
             {2, 7, 10, 6},
             {3, 4, 11, 7},
             {8, 9, 10, 11}}}
    , triangulations{[]() {
        std::unordered_map<size_t, std::vector<Triangle>> tmp;
        tmp[3] = {{{0, 1, 2}}};
        tmp[4] = {{{0, 1, 2}, {0, 2, 3}}};
        tmp[5] = {{{0, 1, 2}, {0, 2, 3}, {0, 3, 4}}};
        tmp[6] = {{{0, 1, 2}, {2, 3, 4}, {0, 4, 5}, {0, 2, 4}}};
        return tmp;
    }()}
    , nodeIdsToEdgeId{util::make_array<8, NodeId>([&](NodeId i) -> std::array<EdgeId, 8> {
        return util::make_array<8, NodeId>([&](NodeId j) -> EdgeId {
            auto it = std::find_if(edges.begin(), edges.end(), [&](auto e) {
                return (e[0] == i && e[1] == j) || (e[0] == j && e[1] == i);
            });
            if (it != edges.end()) {
                return static_cast<EdgeId>(std::distance(edges.begin(), it));
            }
            return -1;
        });
    })}
    , edgeIdToFaceIds{util::make_array<12, EdgeId>([&](EdgeId edge) -> std::array<FaceId, 2> {
        auto it1 = std::find_if(faces.begin(), faces.end(),
                                [&](auto &face) { return util::contains(face, edge); });
        auto it2 = std::find_if(it1 + 1, faces.end(),
                                [&](auto &face) { return util::contains(face, edge); });
        return {static_cast<FaceId>(std::distance(faces.begin(), it1)),
                static_cast<FaceId>(std::distance(faces.begin(), it2))};
    })}
    , nodeNeighbours{util::make_array<8>([&](size_t i) -> std::array<NodeId, 3> {
        auto distance2 = [](const size3_t &a, const size3_t &b) {
            return glm::compAdd((a - b) * (a - b));
        };
        std::array<NodeId, 3> nn{};
        int count = 0;
        for (NodeId j = 0; j < 8; ++j) {
            if (distance2(vertices[i], vertices[j]) == 1) {
                nn[count] = j;
                ++count;
            }
        }
        return nn;
    })}
    , caseTriangles{util::make_array<256>([&](size_t i) { return calcTriangles(i); })}

    , caseEdges{util::make_array<256>([&](size_t i) { return calcEdges(i); })}

    , caseIncrements{util::make_array<256>([&](size_t i) { return calcIncrenents(i); })} {}

std::vector<marching::Config::Triangle> marching::Config::calcTriangles(std::bitset<8> corners,
                                                                        bool flip) {
    if (corners.count() > 4) {
        return calcTriangles(~corners, true);
    }
    std::unordered_map<NodeId, std::vector<NodeId>> edgeMap;
    DisjointSets<int> groups(8);

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

            // 0 2, 3 1, 4 5, 7 6, 8 10, 11 9
        }
    }

    return triangles;
}

std::vector<marching::Config::EdgeId> marching::Config::calcEdges(std::bitset<8> corners, bool) {
    if (corners.count() > 4) {
        return calcEdges(~corners, true);
    }
    std::vector<EdgeId> res;

    // Check which edges that are involved add group neighboring nodes
    for (NodeId i = 0; i < 8; ++i) {
        if (corners[i]) {
            for (NodeId j : nodeNeighbours[i]) {
                if (!corners[j]) {
                    auto edge = nodeIdsToEdgeId[i][j];
                    res.push_back(edge);
                }
            }
        }
    }
    return res;
}

std::array<size_t, 8> marching::Config::calcIncrenents(std::bitset<8> corners, bool) {
    if (corners.count() > 4) {
        return calcIncrenents(~corners, true);
    }
    std::array<size_t, 8> res;
    std::fill(res.begin(), res.end(), 0);

    // Check which edges that are involved add group neighboring nodes
    const std::array<int, 12> edgeToCache = {{0, 4, 1, 4, 6, 6, 7, 7, 2, 5, 3, 5}};
    for (NodeId i = 0; i < 8; ++i) {
        if (corners[i]) {
            for (NodeId j : nodeNeighbours[i]) {
                if (!corners[j]) {
                    auto edge = nodeIdsToEdgeId[i][j];
                    res[edgeToCache[edge]] = 1;
                }
            }
        }
    }
    return res;
}

namespace {

class VCache {
public:
    enum CacheName { xCacheCurr, xCacheNext, yCacheCurr, yCacheNext, zCacheCurr, zCacheNext };
    enum CachePosName { xCurr0, xCurr1, xNext0, xNext1, yCurr, yNext, zCurr, zNext };
    VCache(const size2_t &dim) : cIm{dim} {
        cache[xCacheCurr].resize(dim.x * dim.y);
        cache[xCacheNext].resize(dim.x * dim.y);
        cache[yCacheCurr].resize(dim.x * dim.y);
        cache[yCacheNext].resize(dim.x * dim.y);
        cache[zCacheCurr].resize(dim.x);
        cache[zCacheNext].resize(dim.x);
    }

    std::pair<size_t, bool> find(const size3_t &ind, int edge, const size_t &val) {
        switch (edge) {
            case 0:
                if (ind.z == 0 && ind.y == 0) {
                    cache[xCacheCurr][cIm(pos[xCurr0], ind.y)] = val;
                    return {val, true};
                } else {
                    return {cache[xCacheCurr][cIm(pos[xCurr0], ind.y)], false};
                }
            case 1:
                if (ind.z == 0) {
                    cache[yCacheCurr][cIm(pos[yCurr] + 1, ind.y)] = val;
                    return {val, true};
                } else {
                    return {cache[yCacheCurr][cIm(pos[yCurr] + 1, ind.y)], false};
                }
            case 2:
                if (ind.z == 0) {
                    cache[xCacheCurr][cIm(pos[xCurr1], ind.y + 1)] = val;
                    return {val, true};
                } else {
                    return {cache[xCacheCurr][cIm(pos[xCurr1], ind.y + 1)], false};
                }
            case 3:
                if (ind.z == 0 && ind.x == 0) {
                    cache[yCacheCurr][cIm(pos[yCurr], ind.y)] = val;
                    return {val, true};
                } else {
                    return {cache[yCacheCurr][cIm(pos[yCurr], ind.y)], false};
                }

            case 4:
                if (ind.x == 0 && ind.y == 0) {
                    cache[zCacheCurr][pos[zCurr]] = val;
                    return {val, true};
                } else {
                    return {cache[zCacheCurr][pos[zCurr]], false};
                }
            case 5:
                if (ind.y == 0) {
                    cache[zCacheCurr][pos[zCurr] + 1] = val;
                    return {val, true};
                } else {
                    return {cache[zCacheCurr][pos[zCurr] + 1], false};
                }
            case 6:  // add
                cache[zCacheNext][pos[zNext] + 1] = val;
                return {val, true};
            case 7:
                if (ind.x == 0) {
                    cache[zCacheNext][pos[zNext]] = val;
                    return {val, true};
                } else {
                    return {cache[zCacheNext][pos[zNext]], false};
                }

            case 8:
                if (ind.y == 0) {
                    cache[xCacheNext][cIm(pos[xNext0], ind.y)] = val;
                    return {val, true};
                } else {
                    return {cache[xCacheNext][cIm(pos[xNext0], ind.y)], false};
                }
            case 9:  // add
                cache[yCacheNext][cIm(pos[yNext] + 1, ind.y)] = val;
                return {val, true};
            case 10:  // add
                cache[xCacheNext][cIm(pos[xNext1], ind.y + 1)] = val;
                return {val, true};
            case 11:
                if (ind.x == 0) {
                    cache[yCacheNext][cIm(pos[yNext], ind.y)] = val;
                    return {val, true};
                } else {
                    return {cache[yCacheNext][cIm(pos[yNext], ind.y)], false};
                }
        }
        return {0, false};
    }
    void incX(const std::array<size_t, 8> &increments) {
        for (int i = 0; i < 8; ++i) {
            pos[i] += increments[i];
        }
    }
    void incY() {
        std::swap(cache[zCacheCurr], cache[zCacheNext]);
        pos[xCurr0] = 0;
        pos[xNext0] = 0;
        pos[xCurr1] = 0;
        pos[xNext1] = 0;
        pos[yCurr] = 0;
        pos[yNext] = 0;
        pos[zCurr] = 0;
        pos[zNext] = 0;
    }
    void incZ() {
        std::swap(cache[xCacheCurr], cache[xCacheNext]);
        std::swap(cache[yCacheCurr], cache[yCacheNext]);
    }

private:
    util::IndexMapper2D cIm;
    std::array<std::vector<size_t>, 6> cache;
    std::array<size_t, 8> pos;
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
    Index(const T *src, const util::IndexMapper3D &im, const IsoTest &test)
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
        , test_{test} {}

    void init(const size_t &gridIndex) {
        int next = 0;
        for (int v = 0; v < 4; ++v) {
            const auto val = src_[gridIndex + offsets0_[v]];
            if (test_(val)) {
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
            if (test_(val)) {
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
    const IsoTest &test_;
    int next_;
    int curr_;
};

template <typename T, typename IsoTest>
const std::array<OffsetIndexMasks, 4> Index<T, IsoTest>::oim_ = {
    {{0, 1, {0, 0, 0}}, {3, 2, {0, 1, 0}}, {4, 5, {0, 0, 1}}, {7, 6, {0, 1, 1}}}};

}  // namespace

namespace util {
std::shared_ptr<Mesh> marchingCubesOpt(std::shared_ptr<const Volume> volume, double iso,
                                       const vec4 &color, bool invert, bool enclose,
                                       std::function<void(float)> progressCallback,
                                       std::function<bool(const size3_t &)> maskingCallback) {

    auto indexBuffer = std::make_shared<IndexBuffer>();
    auto vertexBuffer = std::make_shared<Buffer<vec3>>();
    auto textureBuffer = std::make_shared<Buffer<vec3>>();
    auto colorBuffer = std::make_shared<Buffer<vec4>>();
    auto normalBuffer = std::make_shared<Buffer<vec3>>();

    auto indexRAM = indexBuffer->getEditableRAMRepresentation();
    auto &indices = indexRAM->getDataContainer();
    auto &positions = vertexBuffer->getEditableRAMRepresentation()->getDataContainer();
    auto &textures = textureBuffer->getEditableRAMRepresentation()->getDataContainer();
    auto &colors = colorBuffer->getEditableRAMRepresentation()->getDataContainer();
    auto &normals = normalBuffer->getEditableRAMRepresentation()->getDataContainer();

    if (progressCallback) progressCallback(0.0f);

    const auto mc = [&](auto ram, auto isoTest, auto mapValue) {
        using T = util::PrecisionValueType<decltype(ram)>;
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
            const auto tv0 = src[im(ind + cube.vertices[a])];
            const auto v0 = mapValue(tv0);
            const auto tv1 = src[im(ind + cube.vertices[b])];
            const auto v1 = mapValue(tv1);

            const auto t = v0 / (v0 - v1);
            const auto r0 = pos + doffs[a];
            const auto r1 = pos + doffs[b];
            return r0 + t * (r1 - r0);
        };

        VCache vcache(size2_t{dim.x, dim.y});
        Index<T, decltype(isoTest)> index(src, im, isoTest);
        size3_t ind;
        dvec3 pos;

        const float err =
            static_cast<float>(4.0 * glm::epsilon<double>() * glm::epsilon<double>() * dr.x * dr.y);

        for (ind.z = 0, pos.z = 0.0; ind.z < dim1.z; ++ind.z, pos.z += dr.z) {
            vcache.incZ();
            for (ind.y = 0, pos.y = 0.0; ind.y < dim1.y; ++ind.y, pos.y += dr.y) {
                ind.x = 0;
                const auto cInd = im(ind);
                vcache.incY();
                index.init(cInd);
                for (pos.x = 0.0; ind.x < dim1.x; ++ind.x, pos.x += dr.x) {
                    index.update(cInd + ind.x);
                    if (index == 0 || index == 255) continue;
                    if (maskingCallback && !maskingCallback(ind)) continue;

                    std::array<size_t, 12> inds;
                    for (const auto edge : cube.caseEdges[index]) {
                        const auto c = vcache.find(ind, edge, positions.size());
                        inds[edge] = c.first;
                        if (c.second) {
                            const auto vertex = interpolate(ind, pos, edge);
                            positions.emplace_back(vertex);
                            normals.emplace_back(0.0f, 0.0f, 0.0f);
                        }
                    }
                    for (const auto &tri : cube.caseTriangles[index]) {
                        const auto side0 = positions[inds[tri[1]]] - positions[inds[tri[0]]];
                        const auto side1 = positions[inds[tri[2]]] - positions[inds[tri[0]]];
                        auto n = glm::cross(side0, side1);
                        if (glm::length2(n) < err) {
                            continue;  // triangle is so small area is 0.
                        }
                        n = glm::normalize(n);
                        for (int v = 0; v < 3; ++v) {
                            indices.push_back(static_cast<uint32_t>(inds[tri[v]]));
                            normals[inds[tri[v]]] += n;
                        }
                    }
                    vcache.incX(cube.caseIncrements[index]);
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
                using ValueType = util::PrecisionValueType<decltype(ram)>;
                mc(ram,
                   [tiso = util::glm_convert<ValueType>(iso)](auto &&val) { return val > tiso; },
                   [iso](auto &&val) { return util::glm_convert<double>(val) - iso; });
            });
    } else {
        volume->getRepresentation<VolumeRAM>()->dispatch<void, dispatching::filter::Scalars>(
            [&](auto ram) {
                using ValueType = util::PrecisionValueType<decltype(ram)>;
                mc(ram,
                   [tiso = util::glm_convert<ValueType>(iso)](auto &&val) { return val < tiso; },
                   [iso](auto &&val) { return -(util::glm_convert<double>(val) - iso); });
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
    mesh->addIndices({DrawType::Triangles, ConnectivityType::None}, indexBuffer);
    mesh->addBuffer(BufferType::PositionAttrib, vertexBuffer);
    mesh->addBuffer(BufferType::TexcoordAttrib, textureBuffer);
    mesh->addBuffer(BufferType::ColorAttrib, colorBuffer);
    mesh->addBuffer(BufferType::NormalAttrib, normalBuffer);

    if (progressCallback) progressCallback(1.0f);

    return mesh;
}
}  // namespace util

}  // namespace inviwo
