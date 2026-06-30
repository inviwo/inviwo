/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/core/datastructures/volume/temporalvolume.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/glmvec.h>

#include <memory>
#include <vector>

namespace inviwo {

namespace {

constexpr std::string_view frameKey = "frameIndex";

std::shared_ptr<const Volume> makePrototype() {
    return std::make_shared<Volume>(size3_t{2, 2, 2}, DataUInt8::get());
}

// Generates volumes tagged with their frame index and time so tests can verify which frame is
// returned without inspecting voxel data.
std::unique_ptr<ProceduralLoader> makeLoader(size_t count, std::vector<double> times = {}) {
    return std::make_unique<ProceduralLoader>(
        count, std::move(times), makePrototype(),
        [](size_t index, double time) -> std::shared_ptr<Volume> {
            auto volume = std::make_shared<Volume>(size3_t{2, 2, 2}, DataUInt8::get());
            volume->setMetaData<DoubleMetaData, double>(frameKey, static_cast<double>(index));
            volume->setMetaData<DoubleMetaData, double>("time", time);
            return volume;
        });
}

int frameIndexOf(const std::shared_ptr<const Volume>& volume) {
    if (!volume) return -1;
    return static_cast<int>(volume->getMetaData<DoubleMetaData>(frameKey, -1.0));
}

}  // namespace

TEST(TemporalVolumeTest, Metadata) {
    TemporalVolume tv{makeLoader(5)};
    EXPECT_EQ(tv.size(), 5u);
    EXPECT_FALSE(tv.empty());
    EXPECT_EQ(tv.numCached(), 0u);
    EXPECT_EQ(tv.prototype().getDimensions(), (size3_t{2, 2, 2}));
}

TEST(TemporalVolumeTest, DefaultTimes) {
    TemporalVolume tv{makeLoader(4)};
    const auto times = tv.times();
    ASSERT_EQ(times.size(), 4u);
    EXPECT_DOUBLE_EQ(times[0], 0.0);
    EXPECT_DOUBLE_EQ(times[3], 3.0);
    EXPECT_DOUBLE_EQ(tv.timeRange().first, 0.0);
    EXPECT_DOUBLE_EQ(tv.timeRange().second, 3.0);
}

TEST(TemporalVolumeTest, CustomTimes) {
    TemporalVolume tv{makeLoader(4, {0.0, 10.0, 20.0, 30.0})};
    const auto times = tv.times();
    ASSERT_EQ(times.size(), 4u);
    EXPECT_DOUBLE_EQ(times[1], 10.0);
    EXPECT_DOUBLE_EQ(tv.timeRange().second, 30.0);
}

TEST(TemporalVolumeTest, GetByIndex) {
    TemporalVolume tv{makeLoader(5)};
    EXPECT_EQ(frameIndexOf(tv.get(size_t{0})), 0);
    EXPECT_EQ(frameIndexOf(tv.get(size_t{3})), 3);
    EXPECT_EQ(tv.get(size_t{5}), nullptr);  // out of bounds
}

TEST(TemporalVolumeTest, GetByTime) {
    TemporalVolume tv{makeLoader(4, {0.0, 10.0, 20.0, 30.0})};
    EXPECT_EQ(frameIndexOf(tv.get(0.0)), 0);
    EXPECT_EQ(frameIndexOf(tv.get(9.0)), 1);   // nearest to 10
    EXPECT_EQ(frameIndexOf(tv.get(14.0)), 1);  // nearest to 10
    EXPECT_EQ(frameIndexOf(tv.get(16.0)), 2);  // nearest to 20
    EXPECT_EQ(frameIndexOf(tv.get(100.0)), 3);
}

TEST(TemporalVolumeTest, NearestIndex) {
    TemporalVolume tv{makeLoader(4, {0.0, 10.0, 20.0, 30.0})};
    EXPECT_EQ(tv.nearestIndex(-5.0), 0u);
    EXPECT_EQ(tv.nearestIndex(4.0), 0u);
    EXPECT_EQ(tv.nearestIndex(6.0), 1u);
    EXPECT_EQ(tv.nearestIndex(25.0), 2u);  // tie breaks toward the earlier frame
    EXPECT_EQ(tv.nearestIndex(50.0), 3u);
}

TEST(TemporalVolumeTest, InterpolateWithinRange) {
    TemporalVolume tv{makeLoader(4, {0.0, 10.0, 20.0, 30.0})};

    auto frame = tv.interpolate(5.0);
    EXPECT_EQ(frameIndexOf(frame.a), 0);
    EXPECT_EQ(frameIndexOf(frame.b), 1);
    EXPECT_DOUBLE_EQ(frame.t, 0.5);

    frame = tv.interpolate(22.5);
    EXPECT_EQ(frameIndexOf(frame.a), 2);
    EXPECT_EQ(frameIndexOf(frame.b), 3);
    EXPECT_DOUBLE_EQ(frame.t, 0.25);
}

TEST(TemporalVolumeTest, InterpolateAtExactFrame) {
    TemporalVolume tv{makeLoader(4, {0.0, 10.0, 20.0, 30.0})};
    auto frame = tv.interpolate(10.0);
    EXPECT_EQ(frameIndexOf(frame.a), 1);
    EXPECT_EQ(frameIndexOf(frame.b), 2);
    EXPECT_DOUBLE_EQ(frame.t, 0.0);
}

TEST(TemporalVolumeTest, InterpolateClampsOutsideRange) {
    TemporalVolume tv{makeLoader(4, {0.0, 10.0, 20.0, 30.0})};

    auto before = tv.interpolate(-100.0);
    EXPECT_EQ(frameIndexOf(before.a), 0);
    EXPECT_EQ(frameIndexOf(before.b), 0);
    EXPECT_DOUBLE_EQ(before.t, 0.0);

    auto after = tv.interpolate(100.0);
    EXPECT_EQ(frameIndexOf(after.a), 3);
    EXPECT_EQ(frameIndexOf(after.b), 3);
    EXPECT_DOUBLE_EQ(after.t, 0.0);
}

TEST(TemporalVolumeTest, CacheRespectsSize) {
    TemporalVolume tv{makeLoader(8), 3};
    EXPECT_EQ(tv.cacheSize(), 3u);

    for (size_t i = 0; i < 8; ++i) {
        EXPECT_EQ(frameIndexOf(tv.get(i)), static_cast<int>(i));
        EXPECT_LE(tv.numCached(), 3u);
    }
    EXPECT_EQ(tv.numCached(), 3u);
}

TEST(TemporalVolumeTest, MinimumCacheSize) {
    TemporalVolume tv{makeLoader(8), 0};
    EXPECT_EQ(tv.cacheSize(), 2u);
    tv.setCacheSize(1);
    EXPECT_EQ(tv.cacheSize(), 2u);
}

TEST(TemporalVolumeTest, ClearCache) {
    TemporalVolume tv{makeLoader(5)};
    tv.get(size_t{0});
    tv.get(size_t{1});
    EXPECT_GT(tv.numCached(), 0u);
    tv.clearCache();
    EXPECT_EQ(tv.numCached(), 0u);
    // still retrievable after clearing
    EXPECT_EQ(frameIndexOf(tv.get(size_t{0})), 0);
}

TEST(TemporalVolumeTest, SetCacheSizeEvicts) {
    TemporalVolume tv{makeLoader(8), 6};
    for (size_t i = 0; i < 6; ++i) {
        tv.get(i);
    }
    EXPECT_EQ(tv.numCached(), 6u);
    tv.setCacheSize(2);
    EXPECT_EQ(tv.numCached(), 2u);
}

TEST(TemporalVolumeTest, Prefetch) {
    TemporalVolume tv{makeLoader(5)};
    tv.prefetch(2);
    tv.prefetch(3);
    // get() must return the correct frame regardless of whether the prefetch already completed.
    EXPECT_EQ(frameIndexOf(tv.get(size_t{2})), 2);
    EXPECT_EQ(frameIndexOf(tv.get(size_t{3})), 3);
}

TEST(TemporalVolumeTest, PrefetchRange) {
    TemporalVolume tv{makeLoader(10), 8};
    tv.prefetch(0, 4);
    for (size_t i = 0; i < 4; ++i) {
        EXPECT_EQ(frameIndexOf(tv.get(i)), static_cast<int>(i));
    }
}

TEST(TemporalVolumeTest, ProceduralLoaderDirect) {
    ProceduralLoader loader{
        3, {1.0, 2.0, 3.0}, makePrototype(),
        [](size_t index, double) { return std::make_shared<Volume>(size3_t{index + 1}); }};
    EXPECT_EQ(loader.size(), 3u);
    ASSERT_EQ(loader.times().size(), 3u);
    EXPECT_DOUBLE_EQ(loader.times()[2], 3.0);
    EXPECT_EQ(loader.load(1)->getDimensions(), (size3_t{2}));
    EXPECT_NE(loader.prototype(), nullptr);
}

}  // namespace inviwo
