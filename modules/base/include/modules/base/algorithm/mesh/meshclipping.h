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

#ifndef IVW_MESHCLIPPING_ALGO_H
#define IVW_MESHCLIPPING_ALGO_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/geometry/plane.h>

namespace inviwo {

namespace meshutil {

namespace detail {

IVW_MODULE_BASE_API vec3 barycentricTriangle(vec3 p, vec3 a, vec3 b, vec3 c);

// following https://www.mn.uio.no/math/english/people/aca/michaelf/papers/barycentric.pdf
IVW_MODULE_BASE_API std::vector<float> barycentricInsidePolygon(vec2 v,
                                                                const std::vector<vec2>& vis);

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
template <typename F>
std::optional<glm::u32vec2> sutherlandHodgman(glm::u32vec3 triangle, const Plane& plane,
                                              const std::vector<vec3>& positions,
                                              std::vector<std::uint32_t>& indices,
                                              const F& addInterpolatedVertex) {

    std::vector<std::uint32_t> newIndices;
    std::vector<std::uint32_t> newEdge;

    for (size_t i = 0; i < 3; ++i) {
        const auto i1 = triangle[i];
        const auto i2 = triangle[(i + 1) % 3];
        const auto i3 = triangle[(i + 2) % 3];
        const auto v1 = positions[i1];
        const auto v2 = positions[i2];
        const auto v3 = positions[i3];

        if (plane.isInside(v1)) {
            if (plane.isInside(v2)) {  // Case 1
                newIndices.push_back(i2);
            } else {  // Case 2
                const auto weight = *plane.getIntersectionWeight(v1, v2);
                const auto newIndex = addInterpolatedVertex({i1, i2}, {1.0f - weight, weight});
                newIndices.push_back(newIndex);
                newEdge.push_back(newIndex);
            }
        } else if (plane.isInside(v2)) {  // Case 3
            const auto weight = *plane.getIntersectionWeight(v1, v2);
            const auto newIndex = addInterpolatedVertex({i1, i2}, {1.0f - weight, weight});
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
        indices.push_back(newIndices[3]);
        indices.push_back(newIndices[2]);
    }
    if (newEdge.size() == 2) {
        return glm::u32vec2{newEdge[0], newEdge[1]};
    } else {
        return std::nullopt;
    }
}

IVW_MODULE_BASE_API void removeDuplicateEdges(std::vector<glm::u32vec2>& cuts,
                                              const std::vector<vec3>& positions, float eps);

IVW_MODULE_BASE_API std::vector<std::vector<std::uint32_t>> gatherLoops(
    std::vector<glm::u32vec2>& edges, const std::vector<vec3>& positions, float eps);

}  // namespace detail

/**
 * Clip mesh against plane using Sutherland-Hodgman.
 * If holes should be closed, the input mesh must be manifold.
 * Vertex attributes that are interpolated and saved: tex coords and color.
 * Supported mesh types: SimpleMesh and BasicMesh.
 * Supported draw types: Triangle.
 * Supported connectivity types: Strip and None.
 * @param mesh to clip
 * @param plane in world space coordinate system
 * @param capClippedHoles: replaces removed parts with triangles aligned with the plane
 * @throws Exception if mesh is not supported, or if capClippedHoles is set, but
 * mesh is not manifold.
 * @returns SimpleMesh with connectivity None
 */
IVW_MODULE_BASE_API std::shared_ptr<Mesh> clipMeshAgainstPlane(const Mesh& mesh,
                                                               const Plane& worldSpacePlane,
                                                               bool capClippedHoles = true);

IVW_MODULE_BASE_API std::shared_ptr<Mesh> clipMeshAgainstPlaneNew(const Mesh& mesh,
                                                                  const Plane& worldSpacePlane,
                                                                  bool capClippedHoles = true);

/**
 * Compute barycentric coordinates/weights for
 * point p (which is inside the polygon) with respect to polygons of vertices (v)
 * Based on Mean Value Coordinates by Hormann/Floater
 */
IVW_MODULE_BASE_API std::vector<float> barycentricInsidePolygon2D(vec2, const std::vector<vec2>&);

}  // namespace meshutil

}  // namespace inviwo

#endif  // IVW_MESHCLIPPING_ALGO_H
