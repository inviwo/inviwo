/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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
#include <gmock/gmock.h>
#include <warn/pop>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/datastructures/volume/volume.h>

#include <inviwo/core/util/templatesampler.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/algorithm/samplevolume.h>

#include <ranges>
#include <array>

namespace inviwo {

namespace {

constexpr auto dims = size3_t{3, 3, 3};
constexpr auto dim = dims.x * dims.y * dims.z;
// clang-format off
constexpr std::array<double, dim> ref{
    0,  1,  2,
    10, 11, 12,
    20, 21, 22,

    100, 101, 102,
    110, 111, 112,
    120, 121, 122,

    200, 201, 202,
    210, 211, 212,
    220, 221, 222
};
constexpr std::array<dvec3, dim> positions{{
    {0,  0,  0},
    {1,  0,  0},
    {2,  0,  0},
    {0,  1,  0},
    {1,  1,  0},
    {2,  1,  0},
    {0,  2,  0},
    {1,  2,  0},
    {2,  2,  0},

    {0,  0,  1},
    {1,  0,  1},
    {2,  0,  1},
    {0,  1,  1},
    {1,  1,  1},
    {2,  1,  1},
    {0,  2,  1},
    {1,  2,  1},
    {2,  2,  1},

    {0,  0,  2},
    {1,  0,  2},
    {2,  0,  2},
    {0,  1,  2},
    {1,  1,  2},
    {2,  1,  2},
    {0,  2,  2},
    {1,  2,  2},
    {2,  2,  2}
}};
// clang-format on

auto createRAM() {
    auto ram = std::make_shared<VolumeRAMPrecision<double>>(dims);
    auto data = ram->getView();
    std::ranges::copy(ref, data.data());
    return ram;
}
}  // namespace

TEST(VolumeSampling, identity_linear_clamp) {
    const auto ram = createRAM();
    const auto vol = Volume(ram);

    const auto state = sample::createState(vol, CoordinateSpace::Data, DataMapper::Space::Data);

    std::array<double, dim> samples{};
    sample::sample<1>(state, *ram, positions, samples);

    // since coordinateTransformer only uses float we need a large tolerance
    EXPECT_THAT(samples, ::testing::Pointwise(::testing::DoubleNear(1e-4), ref));
}
TEST(VolumeSampling, identity_linear_repeat) {
    const auto ram = createRAM();
    ram->setWrapping({Wrapping::Repeat, Wrapping::Repeat, Wrapping::Repeat});
    const auto vol = Volume(ram);
    const auto state = sample::createState(vol, CoordinateSpace::Data, DataMapper::Space::Data);
    std::array<double, dim> samples{};
    sample::sample<1>(state, *ram, positions, samples);

    // since coordinateTransformer only uses float we need a large tolerance
    EXPECT_THAT(samples, ::testing::Pointwise(::testing::DoubleNear(1e-4), ref));
}

TEST(VolumeSampling, identity_linear_mirror) {
    const auto ram = createRAM();
    ram->setWrapping({Wrapping::Mirror, Wrapping::Mirror, Wrapping::Mirror});
    const auto vol = Volume(ram);
    const auto state = sample::createState(vol, CoordinateSpace::Data, DataMapper::Space::Data);
    std::array<double, dim> samples{};
    sample::sample<1>(state, *ram, positions, samples);

    // since coordinateTransformer only uses float we need a large tolerance
    EXPECT_THAT(samples, ::testing::Pointwise(::testing::DoubleNear(1e-4), ref));
}

TEST(VolumeSampling, identity_nearest_clamp) {
    const auto ram = createRAM();
    ram->setInterpolation(InterpolationType::Nearest);
    const auto vol = Volume(ram);
    const auto state = sample::createState(vol, CoordinateSpace::Data, DataMapper::Space::Data);
    std::array<double, dim> samples{};
    sample::sample<1>(state, *ram, positions, samples);

    // since coordinateTransformer only uses float we need a large tolerance
    EXPECT_THAT(samples, ::testing::Pointwise(::testing::DoubleNear(1e-4), ref));
}

TEST(VolumeSampling, identity_nearest_repeat) {
    const auto ram = createRAM();
    ram->setWrapping({Wrapping::Repeat, Wrapping::Repeat, Wrapping::Repeat});
    ram->setInterpolation(InterpolationType::Nearest);
    const auto vol = Volume(ram);
    const auto state = sample::createState(vol, CoordinateSpace::Data, DataMapper::Space::Data);
    std::array<double, dim> samples{};
    sample::sample<1>(state, *ram, positions, samples);

    // since coordinateTransformer only uses float we need a large tolerance
    EXPECT_THAT(samples, ::testing::Pointwise(::testing::DoubleNear(1e-4), ref));
}

TEST(VolumeSampling, identity_nearest_mirror) {
    const auto ram = createRAM();
    ram->setWrapping({Wrapping::Mirror, Wrapping::Mirror, Wrapping::Mirror});
    ram->setInterpolation(InterpolationType::Nearest);
    const auto vol = Volume(ram);
    const auto state = sample::createState(vol, CoordinateSpace::Data, DataMapper::Space::Data);
    std::array<double, dim> samples{};
    sample::sample<1>(state, *ram, positions, samples);

    // since coordinateTransformer only uses float we need a large tolerance
    EXPECT_THAT(samples, ::testing::Pointwise(::testing::DoubleNear(1e-4), ref));
}

TEST(VolumeSampling, zPlusPoint5) {

    const auto pos = positions | std::views::transform([](dvec3 p) {
                         return p + dvec3{0, 0, 0.5};
                     }) |
                     std::ranges::to<std::vector>();

    // clang-format off
    constexpr std::array<double, dim> ref2{
        50, 51, 52,
        60, 61, 62,
        70, 71, 72,

        150, 151, 152,
        160, 161, 162,
        170, 171, 172,

        200, 201, 202,
        210, 211, 212,
        220, 221, 222
    };
    // clang-format on

    const auto ram = createRAM();
    Volume vol(ram);
    const auto state = sample::createState(vol, CoordinateSpace::Data, DataMapper::Space::Data);
    std::array<double, dim> samples{};
    sample::sample<1>(state, *ram, pos, samples);

    // since coordinateTransformer only uses float we need a large tolerance
    EXPECT_THAT(samples, ::testing::Pointwise(::testing::DoubleNear(1e-4), ref2));
}
TEST(VolumeSampling, zMinusPoint5) {

    const auto pos = positions | std::views::transform([](dvec3 p) {
                         return p - dvec3{0, 0, 0.5};
                     }) |
                     std::ranges::to<std::vector>();

    // clang-format off
    constexpr std::array<double, dim> ref2{
        0,  1,  2,
        10, 11, 12,
        20, 21, 22,

        50, 51, 52,
        60, 61, 62,
        70, 71, 72,

        150, 151, 152,
        160, 161, 162,
        170, 171, 172
    };
    // clang-format on

    const auto ram = createRAM();
    Volume vol(ram);
    const auto state = sample::createState(vol, CoordinateSpace::Data, DataMapper::Space::Data);
    std::array<double, dim> samples{};
    sample::sample<1>(state, *ram, pos, samples);

    // since coordinateTransformer only uses float we need a large tolerance
    EXPECT_THAT(samples, ::testing::Pointwise(::testing::DoubleNear(1e-4), ref2));
}

TEST(VolumeSampling, xline) {

    // clang-format off
    // in texture coordinates
    constexpr std::array<dvec3, 21> line{{
        {-2.00,  0.5,  0.5},
        {-1.75,  0.5,  0.5},
        {-1.50,  0.5,  0.5},
        {-1.25,  0.5,  0.5},
        {-1.00,  0.5,  0.5},
        {-0.75,  0.5,  0.5},
        {-0.50,  0.5,  0.5},
        {-0.25,  0.5,  0.5},

        { 0.00,  0.5,  0.5},
        { 1/3.0,  0.5,  0.5},
        { 0.50,  0.5,  0.5},
        { 2/3.0,  0.5,  0.5},
        { 1.00,  0.5,  0.5},

        { 1.25,  0.5,  0.5},
        { 1.50,  0.5,  0.5},
        { 1.75,  0.5,  0.5},
        { 2.00,  0.5,  0.5},
        { 2.25,  0.5,  0.5},
        { 2.50,  0.5,  0.5},
        { 2.75,  0.5,  0.5},
        { 3.00,  0.5,  0.5}
    }};
    constexpr std::array<double, 21> ref2{
        110,
        110,
        110,
        110,
        110,
        110,
        110,
        110,

        110,
        110.5,
        111,
        111.5,
        112,

        112,
        112,
        112,
        112,
        112,
        112,
        112,
        112
    };
    // clang-format on

    const auto ram = createRAM();
    Volume vol(ram);
    const auto state = sample::createState(vol, CoordinateSpace::Data, DataMapper::Space::Data);
    std::array<double, line.size()> samples{};
    sample::sample<1>(state, *ram, line, samples);

    // since coordinateTransformer only uses float we need a large tolerance
    EXPECT_THAT(samples, ::testing::Pointwise(::testing::DoubleNear(1e-4), ref2));

    //sample::sample(vol, line, samples, CoordinateSpace::Data, DataMapper::Space::Data);
    //EXPECT_THAT(samples, ::testing::Pointwise(::testing::DoubleNear(1e-4), ref2));
}

}  // namespace inviwo
// 110, 110, 110, 110, 110, 110, 110, 110, 110, 110.5, 111, 111.5, 112, 112, 112, 112, 112, 112,
// 112, 112, 112 110, 110, 110, 110, 110, 110, 110, 110, 110, 110.25, 111, 111.75, 112, 112, 112,
// 112, 112, 112, 112, 112, 112
