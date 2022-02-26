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
std::array<float, 27> sampledata = {{1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f,
                                     1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f,
                                     1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f}};

std::shared_ptr<inviwo::Volume> createVolume() {
    auto volumeram = std::make_shared<VolumeRAMPrecision<float>>(size3_t{3, 3, 3});
    std::copy(sampledata.begin(), sampledata.end(), volumeram->getDataTyped());
    return std::make_shared<Volume>(volumeram);
}
} // namespace

void remap(std::shared_ptr<Volume>& volume, std::vector<float> src, std::vector<float> dst) {
    auto volRep = volume->getEditableRepresentation<VolumeRAM>();

    volRep->dispatch<void, dispatching::filter::Scalars>([&](auto volram) {
        using ValueType = util::PrecisionValueType<decltype(volram)>;
        ValueType* dataPtr = volram->getDataTyped();

        const auto& dim = volram->getDimensions();

        std::transform(dataPtr, dataPtr + dim.x * dim.y * dim.z, dataPtr, [&](const ValueType& v) {
            for (size_t i = 0; i < src.size(); ++i) {
                if (static_cast<uint32_t>(v) == src[i]) {
                    return static_cast<ValueType>(dst[i]);
                }
            }
            return ValueType{0};
        });
    });
}

TEST(Volume, volume_region_map_test) {
    auto volume = createVolume();
    std::vector<float> src = {1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f};
    std::vector<float> dst = {1.f, 1.f, 1.f, 2.f, 2.f, 2.f, 3.f, 3.f, 3.f};
    remap(volume, src, dst);

    using VolSampler = TemplateVolumeSampler<float, float, float, 1>;
    const VolSampler sampler(volume);

    EXPECT_FLOAT_EQ(1.0f, sampler.sample(vec3(0.f, 0.f, 0.f)));
    EXPECT_FLOAT_EQ(1.0f, sampler.sample(vec3(0.f, 0.f, 0.5f)));
    EXPECT_FLOAT_EQ(1.0f, sampler.sample(vec3(0.f, 0.f, 1.f)));

    EXPECT_FLOAT_EQ(1.0f, sampler.sample(vec3(0.5f, 0.0f, 0.f)));
    EXPECT_FLOAT_EQ(1.0f, sampler.sample(vec3(0.5f, 0.0f, 0.5f)));
    EXPECT_FLOAT_EQ(1.0f, sampler.sample(vec3(0.5f, 0.0f, 1.f)));

    EXPECT_FLOAT_EQ(1.0f, sampler.sample(vec3(1.f, 0.0f, 0.f)));
    EXPECT_FLOAT_EQ(1.0f, sampler.sample(vec3(1.f, 0.0f, 0.5f)));
    EXPECT_FLOAT_EQ(1.0f, sampler.sample(vec3(1.f, 0.0f, 1.f)));

    EXPECT_FLOAT_EQ(2.0f, sampler.sample(vec3(0.f, 0.5f, 0.f)));
    EXPECT_FLOAT_EQ(2.0f, sampler.sample(vec3(0.f, 0.5f, 0.5f)));
    EXPECT_FLOAT_EQ(2.0f, sampler.sample(vec3(0.f, 0.5f, 1.f)));

    EXPECT_FLOAT_EQ(2.0f, sampler.sample(vec3(0.5f, 0.5f, 0.f)));
    EXPECT_FLOAT_EQ(2.0f, sampler.sample(vec3(0.5f, 0.5f, 0.5f)));
    EXPECT_FLOAT_EQ(2.0f, sampler.sample(vec3(0.5f, 0.5f, 1.f)));

    EXPECT_FLOAT_EQ(2.0f, sampler.sample(vec3(1.f, 0.5f, 0.f)));
    EXPECT_FLOAT_EQ(2.0f, sampler.sample(vec3(1.f, 0.5f, 0.5f)));
    EXPECT_FLOAT_EQ(2.0f, sampler.sample(vec3(1.f, 0.5f, 1.f)));

    EXPECT_FLOAT_EQ(3.0f, sampler.sample(vec3(0.f, 1.f, 0.f)));
    EXPECT_FLOAT_EQ(3.0f, sampler.sample(vec3(0.f, 1.f, 0.5f)));
    EXPECT_FLOAT_EQ(3.0f, sampler.sample(vec3(0.f, 1.f, 1.f)));

    EXPECT_FLOAT_EQ(3.0f, sampler.sample(vec3(0.5f, 1.f, 0.f)));
    EXPECT_FLOAT_EQ(3.0f, sampler.sample(vec3(0.5f, 1.f, 0.5f)));
    EXPECT_FLOAT_EQ(3.0f, sampler.sample(vec3(0.5f, 1.f, 1.f)));

    EXPECT_FLOAT_EQ(3.0f, sampler.sample(vec3(1.f, 1.f, 0.f)));
    EXPECT_FLOAT_EQ(3.0f, sampler.sample(vec3(1.f, 1.f, 0.5f)));
    EXPECT_FLOAT_EQ(3.0f, sampler.sample(vec3(1.f, 1.f, 1.f)));
}   

}  // namespace inviwo 
