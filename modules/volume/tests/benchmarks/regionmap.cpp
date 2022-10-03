/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2022 Inviwo Foundation
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

#include <inviwo/core/datastructures/volume/volume.h>
#include <modules/base/algorithm/volume/volumegeneration.h>
#include <benchmark/benchmark.h>
#include <array>
#include <vector>
#include <inviwo/volume/algorithm/volumemap.h>

using namespace inviwo;

namespace {
// A 10x10xzSize volume
class Volume10 {
public:
    Volume10(int64_t zSize) { benchVolume = createVolume(zSize); }
    Volume10(int x, int y, int z) { benchVolume = createVolume(x, y, z); }

    Volume createVolume(int64_t zSize) {
        size3_t dims{10, 10, zSize};
        const uint32_t size = static_cast<uint32_t>(glm::compMul(dims));
        std::vector<int> sampledata;
        sampledata.reserve(size);
        uint32_t value = 0;
        for (uint32_t i = 0; i < size; ++i) {
            if (value % 100 == 0) {
                value++;
                if (value > 20) {
                    value -= 20;
                }
            }
            sampledata.push_back(value);
        }

        auto volumeram = std::make_shared<VolumeRAMPrecision<uint32_t>>(dims);
        std::copy(sampledata.begin(), sampledata.end(), volumeram->getDataTyped());
        return Volume(volumeram);
    }

    Volume createVolume(int x, int y, int z) {
        size3_t dims{x, y, z};
        const uint32_t size = static_cast<uint32_t>(glm::compMul(dims));
        std::vector<int> sampledata;
        sampledata.reserve(size);
        uint32_t value = 0;
        for (uint32_t i = 0; i < size; ++i) {
            if (value % 100 == 0) {
                value++;
                if (value > 20) {
                    value -= 20;
                }
            }
            sampledata.push_back(value);
        }

        auto volumeram = std::make_shared<VolumeRAMPrecision<uint32_t>>(dims);
        std::copy(sampledata.begin(), sampledata.end(), volumeram->getDataTyped());
        return Volume(volumeram);
    }

    Volume benchVolume;
};

// Sorted continuous size 10, growing volume
void b1(benchmark::State& state) {
    Volume10 v1 = Volume10{static_cast<int>(state.range(0))};
    std::vector<int> src{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<int> dst{10, 9, 8, 7, 6, 5, 4, 3, 2, 1};

    for (auto _ : state) {
        Volume tmp(v1.benchVolume);
        util::remap(tmp, src, dst, 0, true);
    }
}

// Sorted non-continuous size 10, growing volume
void b2(benchmark::State& state) {
    Volume10 v1 = Volume10{static_cast<int>(state.range(0))};
    std::vector<int> src{1, 3, 5, 7, 9, 11, 14, 16, 18, 20};
    std::vector<int> dst{10, 9, 8, 7, 6, 5, 4, 3, 2, 1};

    for (auto _ : state) {
        Volume tmp(v1.benchVolume);
        util::remap(tmp, src, dst, 0, true);
    }
}

// Unsorted non-continuous size 10, growing volume
void b3(benchmark::State& state) {
    Volume10 v1 = Volume10{static_cast<int>(state.range(0))};
    std::vector<int> src{20, 2, 9, 5, 4, 6, 7, 8, 9, 1};
    std::vector<int> dst{10, 9, 8, 7, 6, 5, 4, 3, 2, 1};

    for (auto _ : state) {
        Volume tmp(v1.benchVolume);
        util::remap(tmp, src, dst, 0, true);
    }
}

// Increased vector size, sorted + continuous (directly indexing)
void b4(benchmark::State& state) {

    Volume10 v1 = Volume10{1, 1, 10000};
    std::vector<int> src;
    std::vector<int> dst;
    src.reserve(10000);
    dst.reserve(10000);
    for (int i = 0; i < state.range(0); ++i) {
        src.push_back(i);
        dst.push_back(1000 - i);
    }

    for (auto _ : state) {
        Volume tmp(v1.benchVolume);
        util::remap(tmp, src, dst, 0, true);
    }
}

// Increased vector size, sorted + non-continuous (binary search)
void b5(benchmark::State& state) {

    Volume10 v1 = Volume10{1, 1, 10000};
    std::vector<int> src;
    std::vector<int> dst;
    src.reserve(10000);
    dst.reserve(10000);
    src.push_back(0);
    dst.push_back(1);
    for (int i = 2; i < state.range(0) + 1; ++i) {
        src.push_back(i);
        dst.push_back(1001 - i);
    }

    for (auto _ : state) {
        Volume tmp(v1.benchVolume);
        util::remap(tmp, src, dst, 0, true);
    }
}
// Increased vector size, non-sorted + non-continuous
void b6(benchmark::State& state) {

    Volume10 v1 = Volume10{1, 1, 10000};
    std::vector<int> src;
    std::vector<int> dst;
    src.reserve(10000);
    dst.reserve(10000);
    src.push_back(0);
    dst.push_back(2);
    src.push_back(1);
    dst.push_back(1);
    src.push_back(2);
    dst.push_back(0);
    for (int i = 3; i < state.range(0); ++i) {
        src.push_back(i);
        dst.push_back(1000 - i);
    }

    for (auto _ : state) {
        Volume tmp(v1.benchVolume);
        util::remap(tmp, src, dst, 0, true);
    }
}

}  // namespace

BENCHMARK(b1)->RangeMultiplier(2)->Range(8, 1024);
BENCHMARK(b2)->RangeMultiplier(2)->Range(8, 1024);
BENCHMARK(b3)->RangeMultiplier(2)->Range(8, 1024);
BENCHMARK(b4)->RangeMultiplier(2)->Range(8, 10000);
BENCHMARK(b5)->RangeMultiplier(2)->Range(8, 10000);
BENCHMARK(b6)->RangeMultiplier(2)->Range(8, 10000);

int main(int argc, char** argv) {
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    return 0;
}
