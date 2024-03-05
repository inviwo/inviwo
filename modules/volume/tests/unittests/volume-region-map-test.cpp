/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2024 Inviwo Foundation
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
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/templatesampler.h>
#include <inviwo/volume/algorithm/volumemap.h>

namespace inviwo {

namespace {
std::array<int, 27> sampledata = {
    {1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9}};

std::array<int, 8> sampledata2 = {{1, 1, 3, 3, 4, 4, 5, 5}};

std::shared_ptr<inviwo::Volume> createVolume3x3x3() {
    auto volumeram = std::make_shared<VolumeRAMPrecision<int>>(size3_t{3, 3, 3});
    std::copy(sampledata.begin(), sampledata.end(), volumeram->getDataTyped());
    return std::make_shared<Volume>(volumeram);
}

std::shared_ptr<inviwo::Volume> createVolume2x2x2() {
    auto volumeram = std::make_shared<VolumeRAMPrecision<int>>(size3_t{2, 2, 2});
    std::copy(sampledata2.begin(), sampledata2.end(), volumeram->getDataTyped());
    return std::make_shared<Volume>(volumeram);
}

TEST(Volume, volume_region_map_test_sorted_continuous_sequence) {
    auto volume = createVolume3x3x3();
    std::vector<int> src = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<int> dst = {1, 1, 1, 2, 2, 2, 3, 3, 3};
    util::remap(*volume, src, dst, 0, false);

    using VolSampler = TemplateVolumeSampler<int, int>;
    const VolSampler sampler(volume);

    EXPECT_EQ(1, sampler.sample(vec3(0.f, 0.f, 0.f)));
    EXPECT_EQ(1, sampler.sample(vec3(0.f, 0.f, 0.5f)));
    EXPECT_EQ(1, sampler.sample(vec3(0.f, 0.f, 1.f)));

    EXPECT_EQ(1, sampler.sample(vec3(0.5f, 0.0f, 0.f)));
    EXPECT_EQ(1, sampler.sample(vec3(0.5f, 0.0f, 0.5f)));
    EXPECT_EQ(1, sampler.sample(vec3(0.5f, 0.0f, 1.f)));

    EXPECT_EQ(1, sampler.sample(vec3(1.f, 0.0f, 0.f)));
    EXPECT_EQ(1, sampler.sample(vec3(1.f, 0.0f, 0.5f)));
    EXPECT_EQ(1, sampler.sample(vec3(1.f, 0.0f, 1.f)));

    EXPECT_EQ(2, sampler.sample(vec3(0.f, 0.5f, 0.f)));
    EXPECT_EQ(2, sampler.sample(vec3(0.f, 0.5f, 0.5f)));
    EXPECT_EQ(2, sampler.sample(vec3(0.f, 0.5f, 1.f)));

    EXPECT_EQ(2, sampler.sample(vec3(0.5f, 0.5f, 0.f)));
    EXPECT_EQ(2, sampler.sample(vec3(0.5f, 0.5f, 0.5f)));
    EXPECT_EQ(2, sampler.sample(vec3(0.5f, 0.5f, 1.f)));

    EXPECT_EQ(2, sampler.sample(vec3(1.f, 0.5f, 0.f)));
    EXPECT_EQ(2, sampler.sample(vec3(1.f, 0.5f, 0.5f)));
    EXPECT_EQ(2, sampler.sample(vec3(1.f, 0.5f, 1.f)));

    EXPECT_EQ(3, sampler.sample(vec3(0.f, 1.f, 0.f)));
    EXPECT_EQ(3, sampler.sample(vec3(0.f, 1.f, 0.5f)));
    EXPECT_EQ(3, sampler.sample(vec3(0.f, 1.f, 1.f)));

    EXPECT_EQ(3, sampler.sample(vec3(0.5f, 1.f, 0.f)));
    EXPECT_EQ(3, sampler.sample(vec3(0.5f, 1.f, 0.5f)));
    EXPECT_EQ(3, sampler.sample(vec3(0.5f, 1.f, 1.f)));

    EXPECT_EQ(3, sampler.sample(vec3(1.f, 1.f, 0.f)));
    EXPECT_EQ(3, sampler.sample(vec3(1.f, 1.f, 0.5f)));
    EXPECT_EQ(3, sampler.sample(vec3(1.f, 1.f, 1.f)));
}

TEST(Volume, volume_region_map_test_unordered_map) {
    auto volume = createVolume3x3x3();
    std::vector<int> src = {1, 3, 2, 4, 5, 6, 7, 8, 9};
    std::vector<int> dst = {1, 1, 1, 2, 2, 2, 3, 3, 3};
    util::remap(*volume, src, dst, 0, false);

    using VolSampler = TemplateVolumeSampler<int, int>;
    const VolSampler sampler(volume);

    EXPECT_EQ(1, sampler.sample(vec3(0.f, 0.f, 0.f)));
    EXPECT_EQ(1, sampler.sample(vec3(0.f, 0.f, 0.5f)));
    EXPECT_EQ(1, sampler.sample(vec3(0.f, 0.f, 1.f)));

    EXPECT_EQ(1, sampler.sample(vec3(0.5f, 0.0f, 0.f)));
    EXPECT_EQ(1, sampler.sample(vec3(0.5f, 0.0f, 0.5f)));
    EXPECT_EQ(1, sampler.sample(vec3(0.5f, 0.0f, 1.f)));

    EXPECT_EQ(1, sampler.sample(vec3(1.f, 0.0f, 0.f)));
    EXPECT_EQ(1, sampler.sample(vec3(1.f, 0.0f, 0.5f)));
    EXPECT_EQ(1, sampler.sample(vec3(1.f, 0.0f, 1.f)));

    EXPECT_EQ(2, sampler.sample(vec3(0.f, 0.5f, 0.f)));
    EXPECT_EQ(2, sampler.sample(vec3(0.f, 0.5f, 0.5f)));
    EXPECT_EQ(2, sampler.sample(vec3(0.f, 0.5f, 1.f)));

    EXPECT_EQ(2, sampler.sample(vec3(0.5f, 0.5f, 0.f)));
    EXPECT_EQ(2, sampler.sample(vec3(0.5f, 0.5f, 0.5f)));
    EXPECT_EQ(2, sampler.sample(vec3(0.5f, 0.5f, 1.f)));

    EXPECT_EQ(2, sampler.sample(vec3(1.f, 0.5f, 0.f)));
    EXPECT_EQ(2, sampler.sample(vec3(1.f, 0.5f, 0.5f)));
    EXPECT_EQ(2, sampler.sample(vec3(1.f, 0.5f, 1.f)));

    EXPECT_EQ(3, sampler.sample(vec3(0.f, 1.f, 0.f)));
    EXPECT_EQ(3, sampler.sample(vec3(0.f, 1.f, 0.5f)));
    EXPECT_EQ(3, sampler.sample(vec3(0.f, 1.f, 1.f)));

    EXPECT_EQ(3, sampler.sample(vec3(0.5f, 1.f, 0.f)));
    EXPECT_EQ(3, sampler.sample(vec3(0.5f, 1.f, 0.5f)));
    EXPECT_EQ(3, sampler.sample(vec3(0.5f, 1.f, 1.f)));

    EXPECT_EQ(3, sampler.sample(vec3(1.f, 1.f, 0.f)));
    EXPECT_EQ(3, sampler.sample(vec3(1.f, 1.f, 0.5f)));
    EXPECT_EQ(3, sampler.sample(vec3(1.f, 1.f, 1.f)));
}

TEST(Volume, volume_region_map_test_binary_search) {
    auto volume = createVolume2x2x2();
    std::vector<int> src = {1, 3, 4, 5};
    std::vector<int> dst = {1, 1, 2, 2};
    util::remap(*volume, src, dst, 0, false);

    using VolSampler = TemplateVolumeSampler<int, int>;
    const VolSampler sampler(volume);

    EXPECT_EQ(1, sampler.sample(vec3(0.f, 0.f, 0.f)));
    EXPECT_EQ(1, sampler.sample(vec3(1.0f, 0.0f, 0.f)));

    EXPECT_EQ(1, sampler.sample(vec3(0.f, 1.f, 0.f)));
    EXPECT_EQ(1, sampler.sample(vec3(1.0f, 1.0f, 0.f)));

    EXPECT_EQ(2, sampler.sample(vec3(0.f, 0.f, 1.0f)));
    EXPECT_EQ(2, sampler.sample(vec3(1.0f, 0.0f, 1.0f)));

    EXPECT_EQ(2, sampler.sample(vec3(0.f, 1.0f, 1.0f)));
    EXPECT_EQ(2, sampler.sample(vec3(1.0f, 1.0f, 1.0f)));
}
}  // namespace
}  // namespace inviwo
