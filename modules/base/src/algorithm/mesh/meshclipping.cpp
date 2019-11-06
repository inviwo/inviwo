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
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>

#include <algorithm>

namespace inviwo {

namespace meshutil {

namespace detail {

/* Sutherland-Hodgman Clipping
 *  1) Traverse each edge of each triangle
 *  2) For each edge with vertices [v1, v2]
 *      Case 1: If v1 and v2 is inside, add v2
 *      Case 2: If v1 inside and v2 outside, add intersection
 *      Case 3: If v1 outside and v2 inside, add intersection and then add v2
 *      Case 4: If v1 and v2 is outside, add nothing
 *    Observation: A clipped triangle can either contain
 *      3 points (if only case 1 and 4 occurred) or
 *      4 points (if case 2 and 3 occurred) or
 *      0 points (if only case 4 occurred, thus no points)
 *  3) If 4 points, make two triangles, 0 1 2 and 0 3 2, total 6 points.
 */
std::optional<glm::u32vec2> sutherlandHodgman(glm::u32vec3 triangle, const Plane& plane,
                                              const std::vector<vec3>& positions,
                                              std::vector<std::uint32_t>& indices,
                                              const InterpolateFunctor& addInterpolatedVertex) {

    std::vector<std::uint32_t> newIndices;
    std::vector<std::uint32_t> newEdge;

    for (size_t i = 0; i < 3; ++i) {
        const auto i1 = triangle[i];
        const auto i2 = triangle[(i + 1) % 3];
        const auto v1 = positions[i1];
        const auto v2 = positions[i2];

        if (plane.isInside(v1)) {
            if (plane.isInside(v2)) {  // Case 1
                newIndices.push_back(i2);
            } else {  // Case 2
                const auto weight = *plane.getIntersectionWeight(v1, v2);
                const auto newIndex =
                    addInterpolatedVertex({i1, i2}, {1.0f - weight, weight}, std::nullopt);
                newIndices.push_back(newIndex);
                newEdge.push_back(newIndex);
            }
        } else if (plane.isInside(v2)) {  // Case 3
            const auto weight = *plane.getIntersectionWeight(v1, v2);
            const auto newIndex =
                addInterpolatedVertex({i1, i2}, {1.0f - weight, weight}, std::nullopt);
            newIndices.push_back(newIndex);
            newEdge.push_back(newIndex);
            newIndices.push_back(i2);
        }
    }
    if (newIndices.size() == 3) {
        indices.push_back(newIndices[0]);
        indices.push_back(newIndices[1]);
        indices.push_back(newIndices[2]);
    } else if (newIndices.size() == 4) {
        indices.push_back(newIndices[0]);
        indices.push_back(newIndices[1]);
        indices.push_back(newIndices[2]);

        indices.push_back(newIndices[0]);
        indices.push_back(newIndices[2]);
        indices.push_back(newIndices[3]);
    }
    if (newEdge.size() == 2) {
        return glm::u32vec2{newEdge[0], newEdge[1]};
    } else {
        return std::nullopt;
    }
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
                LogWarnCustom(
                    "MeshClipping",
                    "Found edge, that is not connected to any other edge. This could mean, the "
                    "clipped mesh was not manifold.");
                break;
            }
        }
    }
    return loops;
}

std::vector<float> barycentricInsidePolygon(vec2 v, const std::vector<vec2>& vis) {
    const auto N = vis.size();
    std::vector<vec2> s(N);
    std::transform(vis.begin(), vis.end(), s.begin(), [v](vec2 vi) { return vi - v; });
    std::vector<float> r(N);
    std::transform(s.begin(), s.end(), r.begin(), [](vec2 si) { return glm::length(si); });

    std::vector<float> weights(N, 0.0f);
    std::vector<float> tana(N, 0.0f);

    for (size_t i = 0; i < N; ++i) {
        const size_t j = (i + 1) % N;
        const auto Aij = s[i].x * s[j].y - s[i].y * s[j].x;
        const auto Dij = glm::dot(s[i], s[j]);
        if (glm::all(glm::equal(r[i], 0.0f, 0.0000001f * r[i]))) {
            weights[i] = 1.0f;
            return weights;
        }
        if (glm::all(glm::equal(Aij, 0.0f, 0.0000001f * Aij)) && Dij < 0.0f) {
            weights[i] = r[i] / (r[i] + r[j]);
            weights[j] = r[j] / (r[i] + r[j]);
            return weights;
        }
        tana[i] = (r[i] * r[j] - Dij) / Aij;
    }

    float total = 0.0f;
    for (size_t i = 0; i < N; ++i) {
        const auto prev = (N - 1 + i) % N;
        weights[i] = 2.f * (tana[i] + tana[prev]) / r[i];
        total += weights[i];
    }
    std::transform(weights.begin(), weights.end(), weights.begin(),
                   [total](float w) { return w / total; });

    return weights;
}

vec2 polygonCentroid(const std::vector<vec2>& polygon) {
    vec2 center{0.0f, 0.0f};
    float A = 0.0f;

    for (size_t i = 0; i < polygon.size(); ++i) {
        const size_t j = (i + 1) % polygon.size();
        const float a = polygon[i].x * polygon[j].y - polygon[j].x * polygon[i].y;
        center += a * (polygon[i] + polygon[j]);
        A += a;
    }

    center /= 3.0f * A;

    return center;
}

void capHoles(std::vector<glm::u32vec2>& edges, const Plane& plane,
              const std::vector<vec3>& positions, std::vector<std::uint32_t>& indices,
              const InterpolateFunctor& addInterpolatedVertex) {

    constexpr float relError = 0.000001f;
    const auto eps =
        relError * std::accumulate(edges.begin(), edges.end(), 0.0f, [&](float m, glm::u32vec2 e) {
            return glm::max(m, glm::max(glm::compMax(glm::abs(positions[e[0]])),
                                        glm::compMax(glm::abs(positions[e[1]]))));
        });

    removeDuplicateEdges(edges, positions, eps);
    const auto loops = gatherLoops(edges, positions, eps);

    for (const auto& loop : loops) {
        if (loop.size() < 2) continue;

        const auto trans = glm::inverse(plane.inPlaneBasis());

        std::vector<vec3> pos;
        std::transform(loop.begin(), loop.end(), std::back_inserter(pos),
                       [&](uint32_t p) { return positions[p]; });

        std::vector<vec4> uv4;
        std::transform(loop.begin(), loop.end(), std::back_inserter(uv4), [&](uint32_t p) {
            return trans * vec4{positions[p], 1.0f};
        });

        std::vector<vec2> uv;
        std::transform(loop.begin(), loop.end(), std::back_inserter(uv), [&](uint32_t p) {
            return vec2{trans * vec4{positions[p], 1.0f}};
        });

        const auto uvCenter = polygonCentroid(uv);
        const auto center = vec3{glm::inverse(trans) * vec4{uvCenter, 0.0f, 1.0f}};
        const auto weights = barycentricInsidePolygon(uvCenter, uv);

        const auto orientation =
            glm::cross(positions[loop[0]] - center, positions[loop[1]] - center);
        const auto dir = glm::dot(plane.getNormal(), orientation);

        const auto centerIndex = addInterpolatedVertex(loop, weights, -plane.getNormal());
        for (size_t i = 0; i < loop.size(); ++i) {
            const auto j = (i + 1) % loop.size();
            indices.push_back(centerIndex);
            indices.push_back(
                addInterpolatedVertex({loop[dir < 0 ? i : j]}, {1.0f}, -plane.getNormal()));
            indices.push_back(
                addInterpolatedVertex({loop[dir < 0 ? j : i]}, {1.0f}, -plane.getNormal()));
        }
    }
}

std::vector<glm::u32vec2> clipIndices(const Mesh::MeshInfo& meshInfo,
                                      std::shared_ptr<Mesh>& clippedMesh,
                                      const std::vector<uint32_t>& indices, const Plane& plane,
                                      const std::vector<vec3>& positions,
                                      const InterpolateFunctor& addInterpolatedVertex) {

    std::vector<glm::u32vec2> newEdges;

    if (meshInfo.dt == DrawType::Points) {
        auto outIndices = clippedMesh->addIndexBuffer(DrawType::Points, meshInfo.ct);
        for (auto i : indices) {
            if (plane.isInside(positions[i])) {
                outIndices->add(i);
            }
        }

    } else if (meshInfo.dt == DrawType::Lines) {
        if (meshInfo.ct == ConnectivityType::None) {
            if (indices.size() < 2) return {};
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
                    outIndices->add(
                        addInterpolatedVertex({i1, i2}, {1.0f - weight, weight}, std::nullopt));
                } else if (in2) {
                    const auto weight = *plane.getIntersectionWeight(positions[i1], positions[i2]);
                    outIndices->add(
                        addInterpolatedVertex({i1, i2}, {1.0f - weight, weight}, std::nullopt));
                    outIndices->add(i2);
                }
            }
        } else if (meshInfo.ct == ConnectivityType::Adjacency) {
            if (indices.size() < 4) return newEdges;
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
                    outIndices->add(
                        addInterpolatedVertex({i2, i3}, {1.0f - weight, weight}, std::nullopt));
                    outIndices->add(i3);
                } else if (in2) {
                    const auto weight = *plane.getIntersectionWeight(positions[i2], positions[i3]);
                    outIndices->add(i2);
                    outIndices->add(
                        addInterpolatedVertex({i2, i3}, {1.0f - weight, weight}, std::nullopt));
                    outIndices->add(i3);
                    outIndices->add(i4);
                }
            }
        } else if (meshInfo.ct == ConnectivityType::Strip) {
            if (indices.size() < 2) return newEdges;

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
            if (indices.size() < 4) return newEdges;
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
        if (indices.size() < 3) return newEdges;
        auto outIndices = clippedMesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);

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
    }
    return newEdges;
}

}  // namespace detail

std::shared_ptr<Mesh> clipMeshAgainstPlane(const Mesh& mesh, const Plane& worldSpacePlane,
                                           bool capClippedHoles) {

    const auto plane =
        worldSpacePlane.transform(mesh.getCoordinateTransformer().getWorldToDataMatrix());

    auto clippedMesh = std::make_shared<Mesh>();
    clippedMesh->setModelMatrix(mesh.getModelMatrix());
    clippedMesh->setWorldMatrix(mesh.getWorldMatrix());
    clippedMesh->copyMetaDataFrom(mesh);

    std::vector<detail::InterpolateFunctor> interpolateFunctors;
    std::shared_ptr<BufferRAMPrecision<vec3, BufferTarget::Data>> posBuffer;

    for (const auto& item : mesh.getBuffers()) {
        const auto& bufferType = item.first;
        const auto& inBuffer = item.second;
        auto functor =
            inBuffer->getRepresentation<BufferRAM>()->dispatch<detail::InterpolateFunctor>(
                [&clippedMesh, bufferType, &posBuffer](auto inRam) -> detail::InterpolateFunctor {
                    using PB = util::PrecisionType<decltype(inRam)>;
                    using ValueType = util::PrecisionValueType<decltype(inRam)>;
                    using T = typename util::same_extent<ValueType, float>::type;

                    static const auto mix = [](const PB& buffer,
                                               const std::vector<uint32_t>& indices,
                                               const std::vector<float>& weights) {
                        return static_cast<ValueType>(std::inner_product(
                            indices.begin(), indices.end(), weights.begin(), T{0}, std::plus<>{},
                            [&](uint32_t index, float weight) {
                                return static_cast<T>(buffer[index]) * weight;
                            }));
                    };

                    auto outRam =
                        std::make_shared<BufferRAMPrecision<ValueType, PB::target>>(*inRam);
                    auto outBuffer = std::make_shared<Buffer<ValueType, PB::target>>(outRam);
                    clippedMesh->addBuffer(bufferType, outBuffer);

                    if constexpr (std::is_same_v<ValueType, vec3> &&
                                  PB::target == BufferTarget::Data) {
                        if (bufferType == BufferType::NormalAttrib) {
                            return [outRam](const std::vector<uint32_t>& indices,
                                            const std::vector<float>& weights,
                                            std::optional<vec3> normal) {
                                outRam->add(normal ? *normal : mix(*outRam, indices, weights));
                                return static_cast<uint32_t>(outRam->getSize() - 1);
                            };
                        } else if (bufferType == BufferType::PositionAttrib) {
                            posBuffer = outRam;
                        }
                    }

                    if constexpr (DataFormat<ValueType>::numtype == NumericType::Float) {
                        return [outRam](const std::vector<uint32_t>& indices,
                                        const std::vector<float>& weights, std::optional<vec3>) {
                            outRam->add(mix(*outRam, indices, weights));
                            return static_cast<uint32_t>(outRam->getSize() - 1);
                        };
                    } else {  // Only interpolate floating point buffers;
                        return [outRam](const std::vector<uint32_t>& indices,
                                        const std::vector<float>& weights, std::optional<vec3>) {
                            const auto it = std::max_element(weights.begin(), weights.end());
                            const auto index = std::distance(weights.begin(), it);

                            outRam->add(static_cast<ValueType>((*outRam)[indices[index]]));
                            return static_cast<uint32_t>(outRam->getSize() - 1);
                        };
                    }
                });
        interpolateFunctors.push_back(functor);
    }

    const detail::InterpolateFunctor addInterpolatedVertex =
        [&interpolateFunctors](const std::vector<uint32_t>& indices,
                               const std::vector<float>& weights,
                               std::optional<vec3> normal) -> uint32_t {
        uint32_t res = 0;
        for (auto& fun : interpolateFunctors) res = fun(indices, weights, normal);
        return res;
    };

    if (!posBuffer) {
        throw Exception("Unsupported mesh type, vec3 position buffer not found",
                        IVW_CONTEXT_CUSTOM("MeshClipping"));
    }

    const auto& positions = posBuffer->getDataContainer();
    std::vector<glm::u32vec2> newEdges;

    for (const auto& item : mesh.getIndexBuffers()) {
        const auto meshInfo = item.first;
        const auto indexBuffer = item.second;
        const auto& indices = indexBuffer->getRAMRepresentation()->getDataContainer();

        auto edges = detail::clipIndices(meshInfo, clippedMesh, indices, plane, positions,
                                         addInterpolatedVertex);
        newEdges.insert(newEdges.end(), edges.begin(), edges.end());
    }
    if (mesh.getIndexBuffers().empty()) {
        const auto meshInfo = mesh.getDefaultMeshInfo();
        std::vector<uint32_t> indices(mesh.getBuffer(0)->getSize());
        std::iota(indices.begin(), indices.end(), 0);
        auto edges = detail::clipIndices(meshInfo, clippedMesh, indices, plane, positions,
                                         addInterpolatedVertex);
        newEdges.insert(newEdges.end(), edges.begin(), edges.end());
    }

    if (capClippedHoles && !newEdges.empty()) {
        auto outIndices = clippedMesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
        detail::capHoles(newEdges, plane, positions, outIndices->getDataContainer(),
                         addInterpolatedVertex);
    }

    return clippedMesh;
}

}  // namespace meshutil

}  // namespace inviwo
