/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <inviwo/core/algorithm/cubeplaneintersection.h>

namespace inviwo::util {

void cubePlaneIntersectionAppend(const Plane& plane, std::vector<vec3>& pos,
                                 std::vector<std::uint32_t>& inds) {

    const auto startIndex = static_cast<std::uint32_t>(pos.size());

    {
        // Construct the edges of a unit box and intersect with the plane.
        const std::array<vec3, 8> corners{
            vec3{0.0f, 0.0f, 0.0f}, vec3{1.0f, 0.0f, 0.0f}, vec3{1.0f, 1.0f, 0.0f},
            vec3{0.0f, 1.0f, 0.0f}, vec3{0.0f, 0.0f, 1.0f}, vec3{1.0f, 0.0f, 1.0f},
            vec3{1.0f, 1.0f, 1.0f}, vec3{0.0f, 1.0f, 1.0f},
        };
        const std::array<size2_t, 12> edges{size2_t{0, 1}, size2_t{1, 2}, size2_t{2, 3},
                                            size2_t{3, 0}, size2_t{4, 5}, size2_t{5, 6},
                                            size2_t{6, 7}, size2_t{7, 4}, size2_t{0, 4},
                                            size2_t{1, 5}, size2_t{2, 6}, size2_t{3, 7}};

        for (auto edge : edges) {
            const auto point = plane.getIntersection(corners[edge[0]], corners[edge[1]]);
            if (point) {
                pos.push_back(*point);
            }
        }
    }
    if (pos.size() > startIndex + 2) {
        const vec3 midpoint =
            std::accumulate(pos.begin() + startIndex, pos.end(), vec3{0}) / pos.size();
        const vec3 mainVector = glm::normalize(pos.back() - midpoint);
        const vec3 mainNormal = glm::normalize(glm::cross(mainVector, plane.getNormal()));

        std::vector<float> dotProdVals;
        dotProdVals.reserve(pos.size() - startIndex);

        for (auto it = pos.begin() + startIndex; it != pos.end(); ++it) {
            const auto point = *it;
            const vec3 candVector = glm::normalize(point - midpoint);
            const auto dotProdModifier = glm::dot(candVector, mainNormal);
            auto dotProd = glm::dot(candVector, mainVector);
            if (dotProdModifier < 0 && candVector != mainVector) {
                // let everything past 180 deg be in the domain -3 < x < -1
                dotProd = -(dotProd + 2);
            }
            dotProdVals.push_back(dotProd);
        }

        std::vector<unsigned int> idx(dotProdVals.size());
        std::iota(idx.begin(), idx.end(), 0);

        sort(idx.begin(), idx.end(),
             [&dotProdVals](size_t i1, size_t i2) { return dotProdVals[i1] > dotProdVals[i2]; });

        pos.push_back(midpoint);
        for (size_t i = startIndex; i < idx.size() - 1; i++) {
            inds.push_back(startIndex + idx[i]);
            inds.push_back(startIndex + idx[i + 1]);
            inds.push_back(static_cast<unsigned int>(pos.size() - 1));  // midpoint
        }
        inds.push_back(startIndex + idx[idx.size() - 1]);
        inds.push_back(startIndex + idx[0]);                        // wrap-around
        inds.push_back(static_cast<unsigned int>(pos.size() - 1));  // midpoint
    }
}

}  // namespace inviwo::util