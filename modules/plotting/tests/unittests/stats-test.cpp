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

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <modules/plotting/utils/statsutils.h>

namespace inviwo {

TEST(StatsUtilsTest, init) {
    Buffer<double> X;
    Buffer<double> Y;
    auto &vecX = X.getEditableRAMRepresentation()->getDataContainer();
    auto &vecY = Y.getEditableRAMRepresentation()->getDataContainer();
    vecX.emplace_back(95);
    vecX.emplace_back(85);
    vecX.emplace_back(80);
    vecX.emplace_back(70);
    vecX.emplace_back(60);

    vecY.emplace_back(85);
    vecY.emplace_back(95);
    vecY.emplace_back(70);
    vecY.emplace_back(65);
    vecY.emplace_back(70);
    auto res = statsutil::linearRegresion(X, Y);

    EXPECT_DOUBLE_EQ(0.64383561643835918, res.k) << " value of K";
    EXPECT_DOUBLE_EQ(26.780821917808225, res.m) << " value of m";
    EXPECT_DOUBLE_EQ(0.48032180908893229, res.r2) << " value of r";

    // https://en.wikipedia.org/wiki/Percentile
    auto data = std::vector<double>({20., 15., 50., 40., 35.});
    auto percentiles = statsutil::percentiles(data, {0.05, .30, 0.40, 0.5, 1.0});
    EXPECT_DOUBLE_EQ(15., percentiles[0]) << " 5 percentile";
    EXPECT_DOUBLE_EQ(20., percentiles[1]) << " 30 percentile";
    EXPECT_DOUBLE_EQ(20., percentiles[2]) << " 40 percentile";
    EXPECT_DOUBLE_EQ(35., percentiles[3]) << " 50 percentile";
    EXPECT_DOUBLE_EQ(50., percentiles[4]) << " 100 percentile";
}

}  // namespace inviwo
