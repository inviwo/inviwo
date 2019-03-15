/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifndef IVW_MARCHINGCUBESOPT_H
#define IVW_MARCHINGCUBESOPT_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/exception.h>

#include <array>

namespace inviwo {

namespace util {

/**
 * Extracts an iso surface from a volume using the Marching Cubes algorithm
 *
 * Note: Shares interface with util::marchingcbes and util::marchingtetrahedron
 * This is an optimized version of util::marchingcubes
 *
 * @param volume the scalar volume
 * @param iso iso-value for the extracted surface
 * @param color the color of the resulting surface
 * @param invert flips the normals of the surface normals (useful when values greater than the
 * iso-value is 'outside' of the surface)
 * @param enclose whether to create surface where the iso surface intersects the volume boundaries
 * @param progressCallback if set, will be called will executing with the current progress in the
 * interval [0,1], useful for progress bars
 * @param maskingCallback optional callback to test whether current cell should be evaluated or not
 * (return true to include current cell)
 */

IVW_MODULE_BASE_API std::shared_ptr<Mesh> marchingCubesOpt(
    std::shared_ptr<const Volume> volume, double iso, const vec4 &color, bool invert, bool enclose,
    std::function<void(float)> progressCallback = nullptr,
    std::function<bool(const size3_t &)> maskingCallback = nullptr);
}  // namespace util

namespace marching {
/**
 * Cube definitions
 *
 *                         ●────────── e10  ─────────●
 *                    v7 (0,1,1)                v6 (1,1,1)
 *                       ╱ │                       ╱ │
 *                      ╱  │                      ╱  │
 *                         │                         │
 *                   e11   │                    e9   │
 *
 *                  ╱      e7                 ╱      e6
 *                 ●──────────  e8  ─────────●
 *            v4 (0,0,1)   │            v5 (1,0,1)   │
 *                 │       │                 │       │
 *                 │       │                 │       │
 *                 │       │                 │       │
 *                 │       ●─────────  e2  ──┼───────●
 *                    v3 (0,1,0)                v2 (1,1,0)
 *                e4     ╱                  e5     ╱
 *
 *                 │   e3                    │   e1
 *    ▲            │                         │
 *    │     ▲      │ ╱                       │ ╱
 *  Z │  Y ╱       │╱                        │╱
 *    │   ╱        ●──────────  e0  ─────────●
 *    │  ╱    v0 (0,0,0)                v1 (1,0,0)
 *    │ ╱
 *    │╱
 *    ●──────────▶
 *            X
 *
 * Caching
 * Edge
 *   0: xCacheCurr0 get 0 ind.x, ind.y
 *   1: yCacheCurr  get 4 ind.x + 1, ind.y
 *   2: xCacheCurr1 get 1 ind.x, ind.y + 1
 *   3: yCacheCurr  get 4 ind.x, ind.y
 *   4: zCacheCurr  get 6 ind.x
 *   5: zCacheCurr  get 6 ind.x + 1
 *   6: zCacheNext  add 7 ind.x + 1
 *   7: zCacheNext  get 7 ind.x
 *   8: xCacheNext0 get 2 ind.x, ind.y
 *   9: yCacheNext  add 5 ind.x + 1, ind.y
 *  10: xCacheNext1 add 3 ind.x, ind.y + 1
 *  11: yCacheNext  get 5 ind.x, ind.y
 *
 * {xCacheCurr0, xCacheCurr1, xCacheNext0, xCacheNext1,
 *  yCacheCurr, yCacheNext, zCacheCurr, zCacheNext}
 * 0, 4, 1, 4, 6, 6, 7, 7, 2, 5, 3, 5
 * Triangulations
 *                                                                      3
 *                                                                      ●
 *        2        3         2             3         2                 ╱ ╲
 *        ●         ●───────●               ●───────●                 ╱   ╲
 *       ╱ ╲         ╲     ╱ ╲             ╱ ╲     ╱ ╲               ╱     ╲
 *      ╱   ╲         ╲   ╱   ╲           ╱   ╲   ╱   ╲           4 ●───────● 2
 *     ╱     ╲         ╲ ╱     ╲         ╱     ╲ ╱     ╲           ╱ ╲     ╱ ╲
 *    ●───────●         ●───────●       ●───────●───────●         ╱   ╲   ╱   ╲
 *   0         1       0         1     4        0        1       ╱     ╲ ╱     ╲
 *                                                              ●───────●───────●
 *                                                             5        0        1
 */
struct IVW_MODULE_BASE_API Config {
    using NodeId = int;
    using EdgeId = int;
    using FaceId = int;
    using Edge = std::array<NodeId, 2>;
    using Face = std::array<EdgeId, 4>;
    using Triangle = std::array<EdgeId, 3>;

    Config();

    // Cube definitions
    const std::array<size3_t, 8> vertices;
    const std::array<Edge, 12> edges;
    const std::array<Face, 6> faces;
    const std::unordered_map<size_t, std::vector<Triangle>> triangulations;

    // Derived properties;
    const std::array<std::array<EdgeId, 8>, 8> nodeIdsToEdgeId;
    const std::array<std::array<FaceId, 2>, 12> edgeIdToFaceIds;
    const std::array<std::array<NodeId, 3>, 8> nodeNeighbours;

    std::vector<Triangle> calcTriangles(std::bitset<8> corners, bool flip = false);
    std::vector<EdgeId> calcEdges(std::bitset<8> corners, bool flip = false);
    std::array<size_t, 8> calcIncrenents(std::bitset<8> corners, bool flip = false);

    const std::array<std::vector<Triangle>, 256> caseTriangles;
    const std::array<std::vector<EdgeId>, 256> caseEdges;
    const std::array<std::array<size_t, 8>, 256> caseIncrements;
};

}  // namespace marching

}  // namespace inviwo

#endif  // IVW_MARCHINGCUBESOPT_H
