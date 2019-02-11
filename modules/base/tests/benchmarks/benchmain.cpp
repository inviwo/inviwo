/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifdef _MSC_VER
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#endif

#include <inviwo/core/common/inviwo.h>
#include <modules/base/algorithm/volume/volumegeneration.h>

#include <modules/base/algorithm/volume/marchingcubes.h>
#include <modules/base/algorithm/volume/marchingcubesopt.h>

#include <benchmark/benchmark.h>

#include <cmath>

#include <warn/push>
#include <warn/ignore/unused-function>

using namespace inviwo;

static void SphereOld(benchmark::State& state) {
    auto v = std::shared_ptr<Volume>(
        util::makeSphericalVolume(size3_t{static_cast<size_t>(state.range(0))}));

    for (auto _ : state) {
        auto mesh = util::marchingcubes(v, 0.5, {0.5f, 0.0f, 0.0f, 1.0f}, false, false);
        state.counters["Vertices"] = static_cast<double>(mesh->getBuffer(0)->getSize());
        state.counters["Indices"] =
            static_cast<double>(mesh->getIndexBuffers().front().second->getSize());
        benchmark::ClobberMemory();
    }
    state.counters["Voxels"] = state.range(0) * state.range(0) * state.range(0);
}

static void SphereNew(benchmark::State& state) {
    auto v = std::shared_ptr<Volume>(
        util::makeSphericalVolume(size3_t{static_cast<size_t>(state.range(0))}));

    for (auto _ : state) {
        auto mesh = util::marchingCubesOpt(v, 0.5, {0.5f, 0.0f, 0.0f, 1.0f}, false, false);
        state.counters["Vertices"] = static_cast<double>(mesh->getBuffer(0)->getSize());
        state.counters["Indices"] =
            static_cast<double>(mesh->getIndexBuffers().front().second->getSize());
        benchmark::ClobberMemory();
    }
    state.counters["Voxels"] = state.range(0) * state.range(0) * state.range(0);
}

static void RippleOld(benchmark::State& state) {
    auto v = std::shared_ptr<Volume>(
        util::makeRippleVolume(size3_t{static_cast<size_t>(state.range(0))}));

    for (auto _ : state) {
        auto mesh = util::marchingcubes(v, 0.5, {0.5f, 0.0f, 0.0f, 1.0f}, false, false);
        state.counters["Vertices"] = static_cast<double>(mesh->getBuffer(0)->getSize());
        state.counters["Indices"] =
            static_cast<double>(mesh->getIndexBuffers().front().second->getSize());
        benchmark::ClobberMemory();
    }
    state.counters["Voxels"] = state.range(0) * state.range(0) * state.range(0);
}

static void RippleNew(benchmark::State& state) {
    auto v = std::shared_ptr<Volume>(
        util::makeRippleVolume(size3_t{static_cast<size_t>(state.range(0))}));

    for (auto _ : state) {
        auto mesh = util::marchingCubesOpt(v, 0.5, {0.5f, 0.0f, 0.0f, 1.0f}, false, false);
        state.counters["Vertices"] = static_cast<double>(mesh->getBuffer(0)->getSize());
        state.counters["Indices"] =
            static_cast<double>(mesh->getIndexBuffers().front().second->getSize());
        benchmark::ClobberMemory();
    }
    state.counters["Voxels"] = state.range(0) * state.range(0) * state.range(0);
}

static void MiniOld(benchmark::State& state) {
    auto v = std::shared_ptr<Volume>(
        util::makeSingleVoxelVolume(size3_t{static_cast<size_t>(state.range(0))}));

    for (auto _ : state) {
        auto mesh = util::marchingcubes(v, 0.5, {0.5f, 0.0f, 0.0f, 1.0f}, false, false);
        state.counters["Vertices"] = static_cast<double>(mesh->getBuffer(0)->getSize());
        state.counters["Indices"] =
            static_cast<double>(mesh->getIndexBuffers().front().second->getSize());
        benchmark::ClobberMemory();
    }
    state.counters["Voxels"] = state.range(0) * state.range(0) * state.range(0);
}

static void MiniNew(benchmark::State& state) {
    auto v = std::shared_ptr<Volume>(
        util::makeSingleVoxelVolume(size3_t{static_cast<size_t>(state.range(0))}));

    for (auto _ : state) {
        auto mesh = util::marchingCubesOpt(v, 0.5, {0.5f, 0.0f, 0.0f, 1.0f}, false, false);
        state.counters["Vertices"] = static_cast<double>(mesh->getBuffer(0)->getSize());
        state.counters["Indices"] =
            static_cast<double>(mesh->getIndexBuffers().front().second->getSize());
        benchmark::ClobberMemory();
    }
    state.counters["Voxels"] = state.range(0) * state.range(0) * state.range(0);
}

BENCHMARK(SphereOld)->RangeMultiplier(2)->Range(8, 8 << 5);
BENCHMARK(SphereNew)->RangeMultiplier(2)->Range(8, 8 << 6);

BENCHMARK(RippleOld)->RangeMultiplier(2)->Range(8, 8 << 4);
BENCHMARK(RippleNew)->RangeMultiplier(2)->Range(8, 8 << 5);

// BENCHMARK(MiniOld)->RangeMultiplier(2)->Range(8, 8 << 5);
// BENCHMARK(MiniNew)->RangeMultiplier(2)->Range(8, 8 << 5);

// BENCHMARK(MiniOld)->Arg(3);
// BENCHMARK(MiniNew)->Arg(3);

// BENCHMARK(SphereNew)->Arg(5);

int main(int argc, char** argv) {

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();

    return 0;
}

#include <warn/pop>
