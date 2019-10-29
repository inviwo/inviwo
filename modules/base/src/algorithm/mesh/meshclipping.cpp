/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/base/algorithm/mesh/meshclipping.h>
#include <inviwo/core/datastructures/geometry/simplemesh.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/datastructures/geometry/edge.h>
#include <inviwo/core/datastructures/geometry/polygon.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <modules/base/algorithm/dataminmax.h>

#include <optional>
#include <algorithm>

namespace inviwo {

namespace meshutil {

namespace detail {

// Check point equality with threshold
bool equal(vec3 v1, vec3 v2, float eps) {
    return glm::all(glm::lessThan(glm::abs(v1 - v2), vec3{eps}));
}

// Compute barycentric coordinates/weights for
// point p with respect to triangle (a, b, c)
vec3 barycentricTriangle(vec3 p, vec3 a, vec3 b, vec3 c) {
    vec3 v0 = b - a, v1 = c - a, v2 = p - a;
    float d00 = glm::dot(v0, v0);
    float d01 = glm::dot(v0, v1);
    float d11 = glm::dot(v1, v1);
    float d20 = glm::dot(v2, v0);
    float d21 = glm::dot(v2, v1);
    float invDenom = 1.f / (d00 * d11 - d01 * d01);
    vec3 bary;
    bary.y = (d11 * d20 - d01 * d21) * invDenom;
    bary.z = (d00 * d21 - d01 * d20) * invDenom;
    bary.x = 1.f - bary.y - bary.z;
    return bary;
}

std::vector<Edge3D> findUniqueEdges(const std::vector<Edge3D>& edges, const float eps) {

    std::vector<Edge3D> result;

    for (const auto& e : edges) {
        if (!equal(e.v1, e.v2, eps)) result.push_back(e);
    }

    std::sort(result.begin(), result.end(), [](Edge3D a, Edge3D b) {
        return glm::distance(a.v1, a.v2) > glm::distance(b.v1, b.v2);
    });

    const auto newEnd = std::unique(result.begin(), result.end(), [eps](Edge3D a, Edge3D b) {
        return equal(a.v1, b.v1, eps) && equal(a.v2, b.v2, eps);
    });

    result.resize(std::distance(result.begin(), newEnd));

    return result;
}

std::vector<Polygon<Edge3D>> findLoops(const std::vector<Edge3D>& edges, const float eps) {
    std::vector<Polygon<Edge3D>> polygons;
    std::vector<Edge3D> connectedEdges;
    std::vector<Edge3D> unconnectEdges(edges);

    // Start with one edge, check which other edge is connected to it
    while (!unconnectEdges.empty()) {
        Edge3D currentEdge = unconnectEdges.front();
        connectedEdges.push_back(currentEdge);
        unconnectEdges.erase(unconnectEdges.begin());
        bool createdPolygon = false;

        // Search all edges for a connection
        for (size_t i = 0; i < edges.size(); ++i) {
            if (equal(edges[i].v1, currentEdge.v2, eps) && edges[i].v2 != currentEdge.v1) {
                connectedEdges.push_back(Edge3D(currentEdge.v2, edges[i].v2));
                const auto it =
                    std::find(unconnectEdges.begin(), unconnectEdges.end(), currentEdge);

                if (it != unconnectEdges.end()) unconnectEdges.erase(it);

                currentEdge = edges[i];
                i = 0;
            } else if (equal(edges[i].v2, currentEdge.v2, eps) && edges[i].v1 != currentEdge.v1) {
                connectedEdges.push_back(Edge3D(currentEdge.v2, edges[i].v1));
                const auto it =
                    std::find(unconnectEdges.begin(), unconnectEdges.end(), currentEdge);

                if (it != unconnectEdges.end()) unconnectEdges.erase(it);

                currentEdge = Edge3D(edges[i].v2, edges[i].v1);
                i = 0;
            }

            // Last edge connect to first edge, close the loop and make a polygon
            if (equal(connectedEdges[0].v1, currentEdge.v2, eps)) {
                if (currentEdge.v1 != currentEdge.v2) {
                    connectedEdges.push_back(Edge3D(currentEdge.v2, connectedEdges[0].v1));
                    const auto it =
                        std::find(unconnectEdges.begin(), unconnectEdges.end(), currentEdge);

                    if (it != unconnectEdges.end()) unconnectEdges.erase(it);
                }

                Polygon<Edge3D> newPoly(connectedEdges.size());

                for (size_t j = 0; j < connectedEdges.size(); ++j) {
                    newPoly.at(j) = connectedEdges.at(j);
                }

                polygons.push_back(newPoly);
                connectedEdges.clear();
                createdPolygon = true;
                break;
            }

            if (i == edges.size() - 1) {
                throw Exception(
                    "Found edge, that is not connected to any other edge. This could mean, the "
                    "clipped mesh was not manifold.");
            }
        }

        if (!createdPolygon) {
            throw Exception("Could not connect edges to manifold polygon");
        }
    }

    return polygons;
}

std::vector<Polygon<Edge3D>> simplifyPolygons(const std::vector<Polygon<Edge3D>>& polygons,
                                              const float eps) {
    std::vector<Polygon<Edge3D>> simplifiedPolygons;

    for (const auto& poly : polygons) {
        if (poly.size() == 0) continue;

        std::vector<Edge3D> simplifiedEdges{poly.get(0)};
        for (size_t i = 1; i < poly.size(); ++i) {
            auto& pivotEdge = simplifiedEdges.back();
            const auto testEdge = poly.get(i);
            const auto pivotDir = glm::normalize(pivotEdge.v2 - pivotEdge.v1);
            const auto testDir = glm::normalize(testEdge.v2 - testEdge.v1);
            if (glm::abs(glm::dot(pivotDir, testDir) - 1.0f) < eps) {
                pivotEdge.v2 = testEdge.v2;
            } else {
                simplifiedEdges.push_back(testEdge);
            }
        }

        Polygon<Edge3D> newPoly(simplifiedEdges.size());
        for (size_t i = 0; i < simplifiedEdges.size(); ++i) {
            newPoly.at(i) = simplifiedEdges[i];
        }
        simplifiedPolygons.push_back(newPoly);
    }

    std::vector<Polygon<Edge3D>> notDegenerated;
    for (const auto& poly : simplifiedPolygons) {
        if (poly.size() >= 3) notDegenerated.push_back(poly);
    }

    return notDegenerated;
}

std::vector<vec3> findUniquePoints(const Polygon<Edge3D> polygon, const float eps) {
    std::vector<vec3> points;
    for (size_t i = 0; i < polygon.size(); ++i) {
        Edge3D e = polygon.get(i);
        bool found = false;
        for (const auto& pt : points) {
            if (equal(pt, e.v1, eps)) {
                found = true;
                break;
            }
        }
        if (!found) points.push_back(e.v1);
        found = false;
        for (const auto& pt : points) {
            if (equal(pt, e.v2, eps)) {
                found = true;
                break;
            }
        }
        if (!found) points.push_back(e.v2);
    }
    return points;
}

void removeDuplicateEdges(std::vector<glm::u32vec2>& cuts, const std::vector<vec3>& positions,
                          float eps) {

    cuts.erase(
        std::remove_if(cuts.begin(), cuts.end(),
                       [&](glm::u32vec2 edge) {
                           return glm::all(glm::equal(positions[edge[0]], positions[edge[1]], eps));
                       }),
        cuts.end());

    std::transform(cuts.begin(), cuts.end(), cuts.begin(), [&](glm::u32vec2 edge) {
        const auto pa = glm::value_ptr(positions[edge[0]]);
        const auto pb = glm::value_ptr(positions[edge[1]]);
        return std::lexicographical_compare(pa, pa + 3, pb, pb + 3)
                   ? edge
                   : glm::u32vec2{edge[1], edge[0]};
    });

    std::sort(cuts.begin(), cuts.end(), [&](glm::u32vec2 a, glm::u32vec2 b) {
        const auto pa0 = glm::value_ptr(positions[a[0]]);
        const auto pb0 = glm::value_ptr(positions[b[0]]);
        if (std::lexicographical_compare(pa0, pa0 + 3, pb0, pb0 + 3)) {
            return true;
        } else if (std::lexicographical_compare(pb0, pb0 + 3, pa0, pa0 + 3)) {
            return false;
        } else {
            const auto pa1 = glm::value_ptr(positions[a[1]]);
            const auto pb1 = glm::value_ptr(positions[b[1]]);
            return std::lexicographical_compare(pa1, pa1 + 3, pb1, pb1 + 3);
        }
    });
    cuts.erase(std::unique(cuts.begin(), cuts.end(),
                           [&](glm::u32vec2 a, glm::u32vec2 b) {
                               return glm::all(glm::equal(positions[a[0]], positions[b[0]], eps)) &&
                                      glm::all(glm::equal(positions[a[1]], positions[b[1]], eps));
                           }),
               cuts.end());
}

std::vector<std::vector<std::uint32_t>> gatherLoops(std::vector<glm::u32vec2>& edges,
                                                    const std::vector<vec3>& positions, float eps) {
    std::vector<std::vector<std::uint32_t>> loops;

    auto findMatch = [eps, &positions](std::vector<glm::u32vec2>& edges, std::uint32_t index) {
        const auto it1 = std::find_if(edges.begin(), edges.end(), [&](glm::u32vec2 edge) {
            return glm::all(glm::equal(positions[edge[0]], positions[index], eps));
        });

        if (it1 != edges.end()) {
            return std::make_tuple(it1, (*it1)[1]);
        }
        const auto it2 = std::find_if(edges.begin(), edges.end(), [&](glm::u32vec2 edge) {
            return glm::all(glm::equal(positions[edge[1]], positions[index], eps));
        });

        if (it2 != edges.end()) {
            return std::make_tuple(it2, (*it2)[0]);
        }

        return std::make_tuple(edges.end(), uint32_t{0});
    };

    while (!edges.empty()) {
        auto& loop = loops.emplace_back();
        loop.push_back(edges.back()[0]);
        loop.push_back(edges.back()[1]);
        edges.pop_back();

        while (!edges.empty()) {
            const auto [it, index] = findMatch(edges, loop.back());
            if (it != edges.end()) {
                if (glm::all(glm::equal(positions[index], positions[loop.front()], eps))) {
                    edges.erase(it);
                    break;
                } else {
                    loop.push_back(index);
                    edges.erase(it);
                }
            } else {
                // throw Exception(
                LogWarnCustom(
                    "MeshClipping",
                    "Found edge, that is not connected to any other edge. This could mean, the "
                    "clipped mesh was not manifold.");
                // IVW_CONTEXT_CUSTOM("MeshClipping"));
                break;
            }
        }
    }
    return loops;
}

std::vector<float> barycentricInsidePolygon(vec2 v, const std::vector<vec2>& vis) {

    std::vector<vec2> sis;
    std::transform(vis.begin(), vis.end(), std::back_inserter(sis),
                   [v](const vec2& vi) { return vi - v; });

    std::vector<float> weights(vis.size(), 0.0f);

    std::vector<float> tana(vis.size(), 0.0f);

    for (size_t i = 0; i < vis.size(); ++i) {
        const size_t j = (i + 1) % vis.size();
        const auto ri = glm::length(sis[i]);
        const auto Aij = glm::determinant(mat2{sis[i], sis[j]});
        const auto Dij = glm::dot(sis[i], sis[j]);
        if (util::almostEqual(ri, 0.0f)) {
            weights[i] = 1.0f;
            return weights;
        }
        const auto rj = glm::length(sis[j]);

        if (util::almostEqual(Aij, 0.0f)) {
            if (Dij < 0.0f) {
                weights[i] = ri / (ri + rj);
                weights[j] = rj / (ri + rj);
                return weights;
            }
        } else {
            tana[i] = (rj - Dij / ri) / Aij;
        }
    }

    float total = 0.0f;
    for (size_t i = 0; i < vis.size(); ++i) {
        const auto prev = (i - 1 + vis.size()) % vis.size();
        weights[i] = tana[prev] + tana[i];
        total += weights[i];
    }
    std::transform(weights.begin(), weights.end(), weights.begin(),
                   [total](float w) { return w / total; });

    return weights;
}

template <typename F>
void capHoles(std::vector<glm::u32vec2>& edges, vec3 normal, const std::vector<vec3>& positions,
              std::vector<std::uint32_t>& indices, const F& addInterpolatedVertex) {

    constexpr float relError = 0.00001f;
    const auto eps =
        relError * std::accumulate(edges.begin(), edges.end(), 0.0f, [&](float m, glm::u32vec2 e) {
            return glm::max(m, glm::max(glm::compMax(glm::abs(positions[e[0]])),
                                        glm::compMax(glm::abs(positions[e[1]]))));
        });

    removeDuplicateEdges(edges, positions, eps);
    const auto loops = gatherLoops(edges, positions, eps);

    for (const auto& loop : loops) {
        const auto center =
            std::accumulate(loop.begin(), loop.end(), vec3{0},
                            [&](vec3 acc, uint32_t index) { return acc + positions[index]; }) /
            static_cast<float>(loop.size());

        const auto u = glm::normalize(positions[loop.front()] - center);
        const auto v = glm::normalize(glm::cross(u, normal));
        const auto trans = mat3{u, v, normal};

        std::vector<vec2> uv;
        std::transform(loop.begin(), loop.end(), std::back_inserter(uv),
                       [&](uint32_t p) { return vec2{trans * positions[p]}; });

        const auto weights = barycentricInsidePolygon(vec2{trans * center}, uv);
        const auto centerIndex = addInterpolatedVertex(loop, weights);

        for (size_t i = 0; i < loop.size(); ++i) {
            indices.push_back(loop[i]);
            indices.push_back(loop[(i + 1) % loop.size()]);
            indices.push_back(centerIndex);
        }
    }
}

template <typename F>
void clipIndices(const inviwo::Mesh::MeshInfo& meshInfo, std::shared_ptr<inviwo::Mesh>& clippedMesh,
                 const std::vector<uint32_t>& indices, const inviwo::Plane& plane,
                 const std::vector<inviwo::vec3>& positions, const F& addInterpolatedVertex,
                 bool capClippedHoles) {

    if (meshInfo.dt == DrawType::Points) {
        auto outIndices = clippedMesh->addIndexBuffer(DrawType::Points, meshInfo.ct);
        for (auto i : indices) {
            if (plane.isInside(positions[i])) {
                outIndices->add(i);
            }
        }

    } else if (meshInfo.dt == DrawType::Lines) {
        if (meshInfo.ct == ConnectivityType::None) {
            if (indices.size() < 2) return;
            auto outIndices = clippedMesh->addIndexBuffer(DrawType::Lines, ConnectivityType::None);
            for (unsigned int l = 0; l < indices.size() - 1; l += 2) {
                const auto i1 = indices[l];
                const auto i2 = indices[l + 1];

                const auto in1 = plane.isInside(positions[i1]);
                const auto in2 = plane.isInside(positions[i2]);

                if (in1 && in2) {
                    outIndices->add(i1);
                    outIndices->add(i2);
                } else if (in1) {
                    const auto weight = *plane.getIntersectionWeight(positions[i1], positions[i2]);
                    outIndices->add(i1);
                    outIndices->add(addInterpolatedVertex({i1, i2}, {1.0f - weight, weight}));
                } else if (in2) {
                    const auto weight = *plane.getIntersectionWeight(positions[i1], positions[i2]);
                    outIndices->add(addInterpolatedVertex({i1, i2}, {1.0f - weight, weight}));
                    outIndices->add(i2);
                }
            }
        } else if (meshInfo.ct == ConnectivityType::Adjacency) {
            if (indices.size() < 4) return;
            auto outIndices =
                clippedMesh->addIndexBuffer(DrawType::Lines, ConnectivityType::Adjacency);
            for (unsigned int l = 0; l < indices.size() - 3; l += 4) {

                const auto i1 = indices[l];
                const auto i2 = indices[l + 1];
                const auto i3 = indices[l + 2];
                const auto i4 = indices[l + 3];

                const auto in2 = plane.isInside(positions[i2]);
                const auto in3 = plane.isInside(positions[i3]);

                if (in2 && in3) {
                    outIndices->add(i1);
                    outIndices->add(i2);
                    outIndices->add(i3);
                    outIndices->add(i4);
                } else if (in2) {
                    const auto weight = *plane.getIntersectionWeight(positions[i2], positions[i3]);
                    outIndices->add(i1);
                    outIndices->add(i2);
                    outIndices->add(addInterpolatedVertex({i2, i3}, {1.0f - weight, weight}));
                    outIndices->add(i3);
                } else if (in2) {
                    const auto weight = *plane.getIntersectionWeight(positions[i2], positions[i3]);
                    outIndices->add(i2);
                    outIndices->add(addInterpolatedVertex({i2, i3}, {1.0f - weight, weight}));
                    outIndices->add(i3);
                    outIndices->add(i4);
                }
            }
        } else if (meshInfo.ct == ConnectivityType::Strip) {
            if (indices.size() < 2) return;

            auto start = indices.begin();
            const auto end = indices.end();

            while (start != end) {
                start = std::find_if(start, end,
                                     [&](uint32_t i) { return plane.isInside(positions[i]); });
                const auto lineEnd = std::find_if(
                    start, end, [&](uint32_t i) { return !plane.isInside(positions[i]); });
                if (start != end) {
                    auto& outIndices =
                        clippedMesh->addIndexBuffer(DrawType::Lines, ConnectivityType::Strip)
                            ->getDataContainer();
                    std::copy(start, lineEnd, std::back_inserter(outIndices));
                }
                start = lineEnd;
            }

        } else if (meshInfo.ct == ConnectivityType::StripAdjacency) {
            if (indices.size() < 4) return;
            auto start = indices.begin() + 1;
            const auto end = indices.end() - 1;

            while (start != end) {
                start = std::find_if(start, end,
                                     [&](uint32_t i) { return plane.isInside(positions[i]); });
                const auto lineEnd = std::find_if(
                    start, end, [&](uint32_t i) { return !plane.isInside(positions[i]); });
                if (start != end) {
                    auto& outIndices =
                        clippedMesh
                            ->addIndexBuffer(DrawType::Lines, ConnectivityType::StripAdjacency)
                            ->getDataContainer();

                    outIndices.push_back(*std::prev(start));
                    std::copy(start, lineEnd, std::back_inserter(outIndices));
                    outIndices.push_back(*lineEnd);
                }
                start = lineEnd;
            }

        } else {
            throw Exception("Cannot clip, need line connectivity Strip or None",
                            IVW_CONTEXT_CUSTOM("MeshClipping"));
        }
    } else if (meshInfo.dt == DrawType::Triangles) {
        auto outIndices = clippedMesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
        std::vector<glm::u32vec2> newEdges;
        if (meshInfo.ct == ConnectivityType::Strip) {
            for (unsigned int t = 0; t < indices.size() - 2; ++t) {
                const glm::u32vec3 triangle{indices[t], indices[t & 1 ? t + 2 : t + 1],
                                            indices[t & 1 ? t + 1 : t + 2]};
                auto newEdge = detail::sutherlandHodgman(triangle, plane, positions,
                                                         outIndices->getDataContainer(),
                                                         addInterpolatedVertex);
                if (newEdge) newEdges.push_back(*newEdge);
            }
        } else if (meshInfo.ct == ConnectivityType::None) {
            for (unsigned int t = 0; t < indices.size() - 2; t += 3) {
                const glm::u32vec3 triangle{indices[t], indices[t + 1], indices[t + 2]};
                auto newEdge = detail::sutherlandHodgman(triangle, plane, positions,
                                                         outIndices->getDataContainer(),
                                                         addInterpolatedVertex);
                if (newEdge) newEdges.push_back(*newEdge);
            }
        } else {
            throw Exception("Cannot clip, need triangle connectivity Strip or None",
                            IVW_CONTEXT_CUSTOM("MeshClipping"));
        }
        if (capClippedHoles) {
            detail::capHoles(newEdges, plane.getNormal(), positions, outIndices->getDataContainer(),
                             addInterpolatedVertex);
        }
    }
}  // namespace detail

}  // namespace detail

std::shared_ptr<Mesh> clipMeshAgainstPlaneNew(const Mesh& mesh, const Plane& worldSpacePlane,
                                              bool capClippedHoles) {

    const auto plane =
        worldSpacePlane.transform(mesh.getCoordinateTransformer().getWorldToDataMatrix());

    auto clippedMesh = std::make_shared<Mesh>();
    clippedMesh->setModelMatrix(mesh.getModelMatrix());
    clippedMesh->setWorldMatrix(mesh.getWorldMatrix());

    std::vector<
        std::function<std::uint32_t(const std::vector<uint32_t>&, const std::vector<float>&)>>
        addInterpolatedVertexFuncs;

    for (const auto& buffer : mesh.getBuffers()) {
        buffer.second->getRepresentation<BufferRAM>()->dispatch<void>([&](auto inBuffer) {
            using PB = util::PrecisionType<decltype(inBuffer)>;
            using ValueType = util::PrecisionValueType<decltype(inBuffer)>;

            using T = typename util::same_extent<ValueType, float>::type;

            auto outBuffer = std::make_shared<BufferRAMPrecision<ValueType, PB::target>>(*inBuffer);
            auto buff = std::make_shared<Buffer<ValueType, PB::target>>(outBuffer);
            clippedMesh->addBuffer(buffer.first, buff);

            const auto inter = [outBuffer](const std::vector<uint32_t>& indices,
                                           const std::vector<float>& weights) {
                const T val =
                    std::inner_product(indices.begin(), indices.end(), weights.begin(), T{0},
                                       std::plus<>{}, [&](uint32_t index, float weight) {
                                           return static_cast<T>((*outBuffer)[index]) * weight;
                                       });

                outBuffer->add(static_cast<ValueType>(val));
                return static_cast<uint32_t>(outBuffer->getSize() - 1);
            };

            addInterpolatedVertexFuncs.emplace_back(std::move(inter));
        });
    }

    const auto addInterpolatedVertex = [&addInterpolatedVertexFuncs](
                                           const std::vector<uint32_t>& indices,
                                           const std::vector<float>& weights) -> uint32_t {
        uint32_t res = 0;
        for (auto& fun : addInterpolatedVertexFuncs) res = fun(indices, weights);
        return res;
    };

    const auto posBuffer = clippedMesh->findBuffer(BufferType::PositionAttrib);
    if (!posBuffer.first) {
        throw Exception("Unsupported mesh type, position buffer not found",
                        IVW_CONTEXT_CUSTOM("MeshClipping"));
    }
    if (posBuffer.first->getDataFormat()->getId() != DataVec3Float32::id()) {
        throw Exception("Unsupported mesh type, only vec3 position buffers supported",
                        IVW_CONTEXT_CUSTOM("MeshClipping"));
    }

    const auto& positions =
        static_cast<const Vec3BufferRAM*>(posBuffer.first->getRepresentation<BufferRAM>())
            ->getDataContainer();

    for (const auto& item : mesh.getIndexBuffers()) {
        const auto meshInfo = item.first;
        const auto indexBuffer = item.second;
        const auto& indices = indexBuffer->getRAMRepresentation()->getDataContainer();

        detail::clipIndices(meshInfo, clippedMesh, indices, plane, positions, addInterpolatedVertex,
                            capClippedHoles);
    }
    if (mesh.getIndexBuffers().empty()) {
        const auto meshInfo = mesh.getDefaultMeshInfo();
        std::vector<uint32_t> indices(mesh.getBuffer(0)->getSize());
        std::iota(indices.begin(), indices.end(), 0);
        detail::clipIndices(meshInfo, clippedMesh, indices, plane, positions, addInterpolatedVertex,
                            capClippedHoles);
    }

    return clippedMesh;
}

std::shared_ptr<Mesh> clipMeshAgainstPlane(const Mesh& mesh, const Plane& worldSpacePlane,
                                           bool capClippedHoles) {

    using namespace detail;

    // Perform clipping in data space

    // Transform plane:

    auto worldToData = mesh.getCoordinateTransformer().getWorldToDataMatrix();
    auto worldToDataNormal = glm::transpose(glm::inverse(worldToData));
    auto dataSpacePos = vec3(worldToData * vec4(worldSpacePlane.getPoint(), 1.0));
    auto dataSpaceNormal =
        glm::normalize(vec3(worldToDataNormal * vec4(worldSpacePlane.getNormal(), 0.0)));

    Plane plane(dataSpacePos, dataSpaceNormal);

    // Extract vertex data from mesh:

    DrawType drawType = DrawType::Triangles;
    ConnectivityType connectivityType = ConnectivityType::Strip;
    const std::vector<vec3>* vertexList = nullptr;
    const std::vector<vec3>* texcoordlist = nullptr;
    const std::vector<vec4>* colorList = nullptr;
    const std::vector<unsigned int>* triangleList = nullptr;

    if (auto simple = dynamic_cast<const SimpleMesh*>(&mesh)) {
        vertexList = &simple->getVertexList()->getRAMRepresentation()->getDataContainer();
        texcoordlist = &simple->getTexCoordList()->getRAMRepresentation()->getDataContainer();
        colorList = &simple->getColorList()->getRAMRepresentation()->getDataContainer();
        if (simple->getNumberOfIndicies() > 0) {
            triangleList = &simple->getIndexList()->getRAMRepresentation()->getDataContainer();
            drawType = simple->getIndexMeshInfo(0).dt;
            connectivityType = simple->getIndexMeshInfo(0).ct;
        }
    } else if (auto basic = dynamic_cast<const BasicMesh*>(&mesh)) {
        // TODO do clipping in all the index list now we only consider the first one
        vertexList = &basic->getVertices()->getRAMRepresentation()->getDataContainer();
        texcoordlist = &basic->getTexCoords()->getRAMRepresentation()->getDataContainer();
        colorList = &basic->getColors()->getRAMRepresentation()->getDataContainer();
        if (basic->getNumberOfIndicies() > 0) {
            triangleList =
                &basic->getIndexBuffers()[0].second->getRAMRepresentation()->getDataContainer();
            drawType = basic->getIndexBuffers()[0].first.dt;
            connectivityType = basic->getIndexBuffers()[0].first.ct;
        }
    } else {
        throw Exception("Unsupported mesh type, only simple and basic meshes are supported");
    }

    if (drawType != DrawType::Triangles) {
        throw Exception("Cannot clip, need triangle mesh");
    }

    auto outputMesh = std::make_shared<SimpleMesh>(DrawType::Triangles, ConnectivityType::None);
    outputMesh->setModelMatrix(mesh.getModelMatrix());
    outputMesh->setWorldMatrix(mesh.getWorldMatrix());

    if (vertexList->empty()) {
        return outputMesh;  // nothing to do
    }

    // Check if we are using indices
    if (triangleList == nullptr) {
        throw Exception("Cannot clip, need mesh with indices");
    }

    if (triangleList->size() == 0) {
        return outputMesh;  // nothing to do
    }

    /* Sutherland-Hodgman Clipping
            1) Traverse each edge of each triangle
            2) For each edge with vertices [v1, v2]
                    Case 1: If v1 and v2 is inside, add v2
                    Case 2: If v1 inside and v2 outside, add intersection
                    Case 3: If v1 outside and v2 inside, add intersection and then add v2
                    Case 4: If v1 and v2 is outside, add nothing
            Observation: A clipped triangle can either contain
                    3 points (if only case 1 and 4 occurred) or
                    4 points (if case 2 and 3 occurred) or
                    0 points (if only case 4 occurred, thus no points)
            3) If 4 points, make two triangles, 0 1 2 and 0 3 2, total 6 points.
    */

    std::vector<Edge3D> intersectionsEdges;
    std::vector<std::pair<vec3, vec3>> intersectionTex;
    std::vector<std::pair<vec3, vec4>> intersectionCol;

    // Sutherland-Hodgman on one triangle
    const auto sutherlandHodgman = [&](unsigned int t, bool clockwise) {
        const auto indices = *triangleList;

        unsigned int tri[3];

        tri[0] = indices[t];

        if (clockwise) {
            tri[1] = indices[t + 2];
            tri[2] = indices[t + 1];
        } else {
            tri[1] = indices[t + 1];
            tri[2] = indices[t + 2];
        }

        std::vector<vec3> newVertices;
        std::vector<vec3> newTexCoords;
        std::vector<vec4> newColors;
        bool intersectionAdded = false;

        // Handle the 4 cases for each of the 3 edges
        for (size_t i = 0; i < 3; ++i) {
            size_t j = (i + 1) % 3;

            if (plane.isInside(vertexList->at(tri[i]))) {
                if (plane.isInside(vertexList->at(tri[j]))) {
                    // Case 1
                    // Add v2
                    newVertices.push_back(vertexList->at(tri[j]));
                    newTexCoords.push_back(texcoordlist->at(tri[j]));
                    newColors.push_back(colorList->at(tri[j]));
                } else {
                    // Case 2
                    // Add Intersection
                    const auto inter =
                        plane.getIntersection(vertexList->at(tri[i]), vertexList->at(tri[j]));
                    if (!inter) throw Exception("Edge must intersect plane");
                    vec3 intersection = *inter;
                    newVertices.push_back(intersection);
                    vec3 interBC =
                        barycentricTriangle(intersection, vertexList->at(tri[0]),
                                            vertexList->at(tri[1]), vertexList->at(tri[2]));
                    vec3 interTex = (texcoordlist->at(tri[0]) * interBC.x) +
                                    (texcoordlist->at(tri[1]) * interBC.y) +
                                    (texcoordlist->at(tri[2]) * interBC.z);
                    newTexCoords.push_back(interTex);
                    vec4 interCol = (colorList->at(tri[0]) * interBC.x) +
                                    (colorList->at(tri[1]) * interBC.y) +
                                    (colorList->at(tri[2]) * interBC.z);
                    newColors.push_back(interCol);
                    intersectionTex.push_back(std::make_pair(intersection, interTex));
                    intersectionCol.push_back(std::make_pair(intersection, interCol));

                    // We save the intersection as part of edge on the clipping plane
                    // Note that a bad triangle order could cause edges to cross each other
                    if (intersectionAdded)
                        intersectionsEdges.back().v1 = intersection;
                    else {
                        intersectionsEdges.push_back(Edge3D(intersection));
                        intersectionAdded = true;
                    }
                }
            } else {
                if (plane.isInside(vertexList->at(tri[j]))) {
                    // Case 3
                    // Add Intersection
                    const auto inter =
                        plane.getIntersection(vertexList->at(tri[i]), vertexList->at(tri[j]));
                    if (!inter) throw Exception("Edge must intersect plane");
                    vec3 intersection = *inter;
                    newVertices.push_back(intersection);
                    vec3 interBC =
                        barycentricTriangle(intersection, vertexList->at(tri[0]),
                                            vertexList->at(tri[1]), vertexList->at(tri[2]));
                    vec3 interTex = (texcoordlist->at(tri[0]) * interBC.x) +
                                    (texcoordlist->at(tri[1]) * interBC.y) +
                                    (texcoordlist->at(tri[2]) * interBC.z);
                    newTexCoords.push_back(interTex);
                    vec4 interCol = (colorList->at(tri[0]) * interBC.x) +
                                    (colorList->at(tri[1]) * interBC.y) +
                                    (colorList->at(tri[2]) * interBC.z);
                    newColors.push_back(interCol);
                    intersectionTex.push_back(std::make_pair(intersection, interTex));
                    intersectionCol.push_back(std::make_pair(intersection, interCol));

                    // We save the intersection as part of edge on the clipping plane
                    if (intersectionAdded)
                        intersectionsEdges.back().v2 = intersection;
                    else {
                        intersectionsEdges.push_back(Edge3D(intersection));
                        intersectionAdded = true;
                    }

                    // Add v2
                    newVertices.push_back(vertexList->at(tri[j]));
                    newTexCoords.push_back(texcoordlist->at(tri[j]));
                    newColors.push_back(colorList->at(tri[j]));
                }

                // Case 4
            }
        }

        // Handle more then 3 vertices
        if (newVertices.size() > 3) {
            if (newVertices.size() >= 5)
                throw Exception("Can't handle " + std::to_string(newVertices.size()) +
                                " vertices after clipping");
            vec3 lastVert = newVertices.at(3);
            vec3 lastTexc = newTexCoords.at(3);
            vec4 lastColor = newColors.at(3);
            newVertices.pop_back();
            newTexCoords.pop_back();
            newColors.pop_back();
            newVertices.push_back(newVertices.at(0));
            newTexCoords.push_back(newTexCoords.at(0));
            newColors.push_back(newColors.at(0));
            newVertices.push_back(newVertices.at(2));
            newTexCoords.push_back(newTexCoords.at(2));
            newColors.push_back(newColors.at(2));
            newVertices.push_back(lastVert);
            newTexCoords.push_back(lastTexc);
            newColors.push_back(lastColor);
        }

        // Add vertices to mesh
        for (size_t i = 0; i < newVertices.size(); ++i) {
            outputMesh->addIndex(
                outputMesh->addVertex(newVertices.at(i), newTexCoords.at(i), newColors.at(i)));
        }
    };

    // Check if it is a Triangle Strip
    if (connectivityType == ConnectivityType::Strip) {
        for (unsigned int t = 0; t < triangleList->size() - 2; ++t) {
            sutherlandHodgman(t, t & 1);
        }
    } else if (connectivityType == ConnectivityType::None) {
        for (unsigned int t = 0; t < triangleList->size() - 2; t += 3) {
            sutherlandHodgman(t, false);
        }
    } else {
        throw Exception("Cannot clip, need triangle connectivity Strip or None");
    }

    if (!capClippedHoles) {
        return outputMesh;  // mesh with hole, i.e. outside vertices replaced by intersections
    }

    // =======================================================================================

    // To close holes, we need to search for connections in intersection edges.
    // This requires an EPSILON as lower threshold for which points are viewed as the same.
    // We try to choose a good EPSILON based on mesh size to ensure we find connections.
    // Effects of bad EPSILON can be:
    // Too large => Uniqueness test wrongly discards edges => Missing triangles
    // Too small => Loops may not be found => Exception or missing triangles

    const auto& buffers = mesh.getBuffers();
    const auto posBufferIt = std::find_if(buffers.begin(), buffers.end(), [](const auto& buff) {
        return buff.first.type == BufferType::PositionAttrib;
    });
    if (posBufferIt == buffers.end() || posBufferIt->second->getSize() == 0) {
        throw Exception("Mesh has no position buffer or position buffer is empty.");
    }
    const auto minmax = util::bufferMinMax(posBufferIt->second.get(), IgnoreSpecialValues::Yes);
    const float EPSILON = 0.00001f * glm::length(vec3(minmax.second) - vec3(minmax.first));

    // Find unique edges that surround the hole(s):

    const auto uniqueintersectionsEdges = findUniqueEdges(intersectionsEdges, EPSILON);

    if (uniqueintersectionsEdges.empty()) {
        return outputMesh;
    }

    // Triangulate hole:

    // With convex input we could assume only one hole, that is a convex, non-self-intersecting,
    // closed polygon, but generally we have to find edge groups that form loops.
    const auto loops = findLoops(uniqueintersectionsEdges, EPSILON);

    // Simplify polygons, e.g. merging successive straight edges,
    // to prevent triangulation becoming too bad, espescially when clipping a mesh multiple times.
    const auto polygons = simplifyPolygons(loops, EPSILON);

    if (polygons.empty()) {
        return outputMesh;
    }

    // Calculate uv basis.
    const auto u = vec3(polygons[0].get(0).v2 - polygons[0].get(0).v1);
    const auto v = glm::cross(plane.getNormal(), u);

    // Add new polygons as triangles to the mesh.
    // Interpolating tex coords and color using barycentric coordinates.
    for (size_t p = 0; p < polygons.size(); ++p) {
        const size_t pSize = polygons[p].size();

        std::vector<vec2> uv;
        std::vector<vec3> tex;
        std::vector<vec4> col;

        for (size_t i = 0; i < pSize; ++i) {
            // Calculate u-v plane coordinates of the vertex on the polygon
            uv.push_back(
                vec2(glm::dot(u, polygons[p].get(i).v1), glm::dot(v, polygons[p].get(i).v1)));

            // Lookup texcoord and colors for the vertex of the polygon
            for (size_t t = 0; t < intersectionTex.size(); ++t) {
                if (intersectionTex.at(t).first == polygons[p].get(i).v1) {
                    tex.push_back(intersectionTex.at(t).second);
                    col.push_back(intersectionCol.at(t).second);
                    break;
                }
            }
        }

        uv.pop_back();

        std::vector<vec3> newVertices;
        std::vector<vec3> newTexCoords;
        std::vector<vec4> newColors;

        // For polygons with less than 5 unique points, we could do better triangulation without the
        // mean point, but the following works in any case.

        // Calculate mean in polygon as connection point for triangles.
        // Note that the mean might not lie inside polygon in non-convex case.
        const auto points = findUniquePoints(polygons[p], EPSILON);
        vec3 sum(0.0f);
        for (const auto& pt : points) sum += pt;
        const auto mean = sum / points.size();

        // Calculate barycentric coordinates (weights) for all the vertices based on
        // centroid.
        vec2 uvC(glm::dot(u, mean), glm::dot(v, mean));
        std::vector<float> baryW = barycentricInsidePolygon2D(uvC, uv);

        // Use weights for interpolation.
        vec3 texC(0.f);
        vec4 colC(0.f);
        for (size_t i = 0; i < pSize - 1; ++i) {
            texC += tex[i] * baryW[i];
            colC += col[i] * baryW[i];
        }

        for (size_t i = 0; i < pSize; ++i) {
            size_t ip = (i + 1) % pSize;
            newVertices.push_back(mean);
            newVertices.push_back(polygons[p].get(i).v2);
            newVertices.push_back(polygons[p].get(i).v1);

            newTexCoords.push_back(texC);
            newTexCoords.push_back(tex[ip]);
            newTexCoords.push_back(tex[i]);

            newColors.push_back(colC);
            newColors.push_back(col[ip]);
            newColors.push_back(col[i]);
        }

        // Add triangles to the mesh
        for (size_t i = 0; i < newVertices.size(); ++i) {
            outputMesh->addIndex(
                outputMesh->addVertex(newVertices[i], newTexCoords[i], newColors[i]));
        }
    }

    return outputMesh;
}

std::vector<float> barycentricInsidePolygon2D(vec2 p, const std::vector<vec2>& v) {

    const size_t numV = v.size();

    // Use float precision for result
    std::vector<float> baryW(numV, 0.0f);

    // Use double precision for intermediate values
    std::vector<dvec2> s(numV);
    std::vector<double> ri(numV);

    for (size_t i = 0; i < numV; ++i) {
        s[i] = v[i] - p;
        ri[i] = std::sqrt(glm::dot(s[i], s[i]));
    }

    std::vector<double> A(numV);
    std::vector<double> tanA(numV);

    for (size_t i = 0; i < numV; ++i) {
        size_t ip = (i + 1) % numV;
        A[i] = s[i].x * s[ip].y - s[ip].x * s[i].y;
        if (A[i] == 0.0) {
            if (util::almostEqual(p, v[i])) {
                baryW[i] = 1.f;
                return baryW;
            } else if (util::almostEqual(p, v[ip])) {
                baryW[ip] = 1.f;
                return baryW;
            } else {
                double l = ri[i] + ri[ip];
                baryW[i] = static_cast<float>(ri[ip] / l);
                baryW[ip] = static_cast<float>(ri[i] / l);
                return baryW;
            }
        }
        tanA[i] = (ri[i] * ri[ip] - glm::dot(s[i], s[ip])) / A[i];
    }

    double wsum = 0.0;

    for (size_t i = 0; i < numV; ++i) {
        size_t ip = (numV - 1 + i) % numV;
        double wi = 2.0 * (tanA[i] + tanA[ip]) / ri[i];
        baryW[i] = static_cast<float>(wi);
        wsum += wi;
    }

    if (std::abs(wsum) > 0.f) {
        for (size_t i = 0; i < numV; ++i) {
            double wnorm = static_cast<double>(baryW[i]) / wsum;
            baryW[i] = static_cast<float>(wnorm);
        }
    }

    for (size_t i = 0; i < numV; ++i) {
        if (std::isnan(baryW[i])) throw Exception("Mean Value Coordinate computation yield NaN");
    }

    return baryW;
}

}  // namespace meshutil

}  // namespace inviwo
