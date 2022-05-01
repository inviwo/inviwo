/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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
#include <inviwo/volume/processors/volumeregionmap.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/templatesampler.h>

namespace inviwo {

namespace {
std::array<unsigned int, 27> sampledata = {
    {1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9}};

std::shared_ptr<inviwo::Volume> createVolume() {
    auto volumeram = std::make_shared<VolumeRAMPrecision<float>>(size3_t{3, 3, 3});
    std::copy(sampledata.begin(), sampledata.end(), volumeram->getDataTyped());
    return std::make_shared<Volume>(volumeram);
}

TEST(Volume, volume_region_map_test) {
    auto volume = createVolume();
    std::vector<unsigned int> src = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<unsigned int> dst = {1, 1, 1, 2, 2, 2, 3, 3, 3};
    VolumeRegionMap::remap(volume, src, dst, 0, false);

    using VolSampler = TemplateVolumeSampler<float, float, float, 1>;
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
}  // namespace
}  // namespace inviwo
