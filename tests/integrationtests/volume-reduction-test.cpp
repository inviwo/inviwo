/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2021 Inviwo Foundation
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

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <modules/opengl/volume/volumegl.h>

#include <inviwo/core/common/inviwoapplication.h>

#include <modules/base/algorithm/dataminmax.h>
#include <modules/basegl/algorithm/gpureduction.h>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

namespace inviwo {

namespace {

std::shared_ptr<inviwo::Volume> createVolume() {
    std::vector<float> sampledata(8 * 16 * 4);
    std::generate(std::begin(sampledata), std::end(sampledata),
                  [n = 1]() mutable { return static_cast<float>(n++); });
    auto volumeram = std::make_shared<VolumeRAMPrecision<float>>(size3_t{8, 16, 4});
    std::copy(sampledata.begin(), sampledata.end(), volumeram->getDataTyped());
    return std::make_shared<Volume>(volumeram);
}

}  // namespace

TEST(VolumeReduction, VolumeMax) {
    auto vol = createVolume();
    auto [minCPU, maxCPU] = util::volumeMinMax(vol.get());

    GPUReduction reducer;
    auto res = reducer.reduce(vol, ReductionOperator::Max);
    auto maxVal = reducer.reduce_v(vol, ReductionOperator::Max);
    auto [minGPU, maxGPU] = util::volumeMinMax(res.get());

    EXPECT_DOUBLE_EQ(maxCPU.x, maxGPU.x);
    EXPECT_DOUBLE_EQ(maxGPU.x, maxVal.x);
}

TEST(VolumeReduction, VolumeMin) {
    auto vol = createVolume();
    auto [minCPU, maxCPU] = util::volumeMinMax(vol.get());

    GPUReduction reducer;
    auto res = reducer.reduce(vol, ReductionOperator::Min);
    auto minVal = reducer.reduce_v(vol, ReductionOperator::Min);
    auto [minGPU, maxGPU] = util::volumeMinMax(res.get());

    EXPECT_DOUBLE_EQ(minCPU.x, minGPU.x);
    EXPECT_DOUBLE_EQ(minGPU.x, minVal.x);
}

}  // namespace inviwo