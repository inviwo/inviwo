/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2020 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <modules/discretedata/sampling/interpolant.h>
#include <inviwo/core/util/zip.h>

namespace inviwo {
namespace discretedata {

TEST(DataSet, Interpolation) {

    using arr2 = std::array<float, 2>;
    SkewedBoxInterpolant<2> interpolant;

    std::vector<float> weights;
    bool inside;
    vec3 baseX(4, 2, 0), baseY(-1, 1, 0), baseT(1, 1, 1);
    mat3 baseMat(baseX, baseY, baseT);

    vec3 position = {0.25, 0.5, 1};
    std::array<vec3, 5> points = {vec3{0, 0, 1}, {1, 0, 1}, {0, 1, 1}, {1, 1, 1}, position};
    std::vector<arr2> coords;
    for (const vec3& p : points) {
        vec3 transformedPoint = baseMat * p;
        coords.push_back(arr2{transformedPoint.x, transformedPoint.y});
        std::cerr << "point " << transformedPoint.x << ", " << transformedPoint.y << std::endl;
    }
    EXPECT_NO_THROW(inside = interpolant.getWeights(discretedata::InterpolationType::Linear,
                                                    {coords[0], coords[1], coords[2], coords[3]},
                                                    weights, coords[4]));

    EXPECT_TRUE(inside);
    EXPECT_EQ(weights.size(), 4);
    EXPECT_DOUBLE_EQ(weights[0], (1 - position[0]) * (1 - position[1]));
    EXPECT_DOUBLE_EQ(weights[1], position[0] * (1 - position[1]));
    EXPECT_DOUBLE_EQ(weights[2], (1 - position[0]) * position[1]);
    EXPECT_DOUBLE_EQ(weights[3], position[0] * position[1]);
}  // namespace discretedata

}  // namespace discretedata
}  // namespace inviwo
