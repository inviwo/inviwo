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
#include <functional>
#include <optional>
#include <vector>

namespace inviwo {

namespace meshutil {

namespace detail {

using InterpolateFunctor = std::function<std::uint32_t(
    const std::vector<uint32_t>&, const std::vector<float>&, std::optional<vec3>)>;

/**
 * Compute barycentric coordinates/weights for
 * point p (which is inside the polygon) with respect to polygons of vertices (v)
 * Based on Mean Value Coordinates by Hormann/Floater
 * following https://www.mn.uio.no/math/english/people/aca/michaelf/papers/barycentric.pdf
 */
IVW_MODULE_BASE_API std::vector<float> barycentricInsidePolygon(vec2 v,
                                                                const std::vector<vec2>& vis);

/**
 * Compute the Centroid of a planar polygon
 * Following https://en.wikipedia.org/wiki/Centroid
 */
IVW_MODULE_BASE_API vec2 polygonCentroid(const std::vector<vec2>& polygon);

IVW_MODULE_BASE_API std::optional<glm::u32vec2> sutherlandHodgman(
    glm::u32vec3 triangle, const Plane& plane, const std::vector<vec3>& positions,
    std::vector<std::uint32_t>& indices, const InterpolateFunctor& addInterpolatedVertex);

IVW_MODULE_BASE_API void removeDuplicateEdges(std::vector<glm::u32vec2>& cuts,
                                              const std::vector<vec3>& positions, float eps);

IVW_MODULE_BASE_API std::vector<std::vector<std::uint32_t>> gatherLoops(
    std::vector<glm::u32vec2>& edges, const std::vector<vec3>& positions, float eps);

}  // namespace detail

/**
 * Clip mesh against plane using Sutherland-Hodgman.
 * If holes should be closed, the input mesh must be manifold.
 * Vertex attributes are interpolated. Floating types use linear interpolation, integer types use
 * nearest. Connectivity types loop and fan are not handled.
 * @param mesh to clip
 * @param plane in world space coordinate system
 * @param capClippedHoles: replaces removed parts with triangles aligned with the plane
 * @throws Exception if mesh is not supported.
 * @returns Clipped Mesh
 */
IVW_MODULE_BASE_API std::shared_ptr<Mesh> clipMeshAgainstPlane(const Mesh& mesh,
                                                               const Plane& worldSpacePlane,
                                                               bool capClippedHoles = true);

}  // namespace meshutil

}  // namespace inviwo

#endif  // IVW_MESHCLIPPING_ALGO_H
