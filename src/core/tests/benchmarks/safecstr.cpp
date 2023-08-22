/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2023 Inviwo Foundation
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

#include <benchmark/benchmark.h>

#include <string_view>
#include <string>
#include <array>
#include <variant>
#include <vector>
#include <memory>
#include <cstring>

namespace {

std::string_view view =
    "01234567890123456789012345678901234567890123456789"
    "01234567890123456789012345678901234567890123456789"
    "01234567890123456789012345678901234567890123456789"
    "01234567890123456789012345678901234567890123456789";

template <size_t N>
class SafeCStr1 {
public:
    SafeCStr1(std::string_view sv) {
        static_assert(N > 0);
        if (sv.size() < N - 1) {
            auto& arr = storage.template emplace<std::array<char, N>>();
            std::copy(sv.begin(), sv.end(), arr.begin());
            arr[sv.size()] = 0;
        } else {
            auto& arr = storage.template emplace<std::vector<char>>(sv.size() + 1);
            std::memcpy(arr.data(), sv.data(), sv.size());
            arr[sv.size()] = 0;
        }
    }

    const char* c_str() const {
        return std::visit([](const auto& str) { return str.data(); }, storage);
    }
    operator const char*() const { return c_str(); }

private:
    std::variant<std::array<char, N>, std::vector<char>> storage;
};

template <size_t N>
class SafeCStr2 {
public:
    SafeCStr2(std::string_view sv) {
        static_assert(N > 0);
        if (sv.size() < N - 1) {
            std::memcpy(stack.data(), sv.data(), sv.size());
            stack[sv.size()] = 0;
            heap = nullptr;
        } else {
            heap = std::make_unique<char[]>(sv.size() + 1);
            std::memcpy(heap.get(), sv.data(), sv.size());
            heap[sv.size()] = 0;
        }
    }

    const char* c_str() const { return heap ? heap.get() : stack.data(); }
    operator const char*() const { return heap ? heap.get() : stack.data(); }

private:
    std::array<char, N> stack;  // Do not initialize!
    std::unique_ptr<char[]> heap;
};

static void StringConv(benchmark::State& state) {
    std::string_view sv{view.data(), static_cast<size_t>(state.range(0))};
    for (auto _ : state) {
        auto str = std::string(sv);
        auto ptr = str.c_str();
        benchmark::DoNotOptimize(ptr);
    }
}

static void ArrayConvInit(benchmark::State& state) {
    std::string_view sv{view.data(), static_cast<size_t>(state.range(0))};
    for (auto _ : state) {
        std::array<char, 256> buff = {0};
        std::memcpy(buff.data(), sv.data(), sv.size());
        auto ptr = buff.data();
        benchmark::DoNotOptimize(ptr);
    }
}

static void ArrayConvZero(benchmark::State& state) {
    std::string_view sv{view.data(), static_cast<size_t>(state.range(0))};
    for (auto _ : state) {
        std::array<char, 256> buff;
        std::memcpy(buff.data(), sv.data(), sv.size());
        buff[sv.size()] = 0;
        auto ptr = buff.data();
        benchmark::DoNotOptimize(ptr);
    }
}

static void SafeCStr1_256(benchmark::State& state) {
    std::string_view sv{view.data(), static_cast<size_t>(state.range(0))};
    for (auto _ : state) {
        auto str = SafeCStr1<256>(sv);
        auto ptr = str.c_str();
        benchmark::DoNotOptimize(ptr);
    }
}

static void SafeCStr2_56(benchmark::State& state) {
    std::string_view sv{view.data(), static_cast<size_t>(state.range(0))};
    for (auto _ : state) {
        auto str = SafeCStr2<56>(sv);
        auto ptr = str.c_str();
        benchmark::DoNotOptimize(ptr);
    }
}

}  // namespace

constexpr int64_t min = 5;
constexpr int64_t max = 150;
constexpr int64_t step = 5;

BENCHMARK(StringConv)->DenseRange(min, max, step);
BENCHMARK(ArrayConvInit)->DenseRange(min, max, step);
BENCHMARK(ArrayConvZero)->DenseRange(min, max, step);
BENCHMARK(SafeCStr1_256)->DenseRange(min, max, step);
BENCHMARK(SafeCStr2_56)->DenseRange(min, max, step);

BENCHMARK_MAIN();
