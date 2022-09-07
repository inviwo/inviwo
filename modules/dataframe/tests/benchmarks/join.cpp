/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2022 Inviwo Foundation
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
#include <vector>
#include <unordered_map>

#include <random>
#include <algorithm>

#include <inviwo/core/datastructures/bitset.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/zip.h>

#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/dataframe/util/dataframeutil.h>
#include <inviwo/dataframe/util/filters.h>

namespace {

std::vector<int> left_;
std::vector<int> right_;

using namespace inviwo;

template <typename T, bool firstMatchOnly, typename Cont1, typename Cont2>
std::vector<std::vector<std::uint32_t>> matchingRowsPrev(const Cont1& left, const Cont2& right) {
    std::vector<std::vector<std::uint32_t>> rows(left.size());
    for (auto&& [i, key] : util::enumerate<std::uint32_t>(left)) {
        // find all matching rows in right
        std::vector<std::uint32_t> matches;
        for (auto&& [r, value] : util::enumerate<std::uint32_t>(right)) {
            if (key == value) {
                matches.emplace_back(r);
                if constexpr (firstMatchOnly) break;
            }
        }
        rows[i] = std::move(matches);
    }
    return rows;
}

template <typename T, bool firstMatchOnly, typename Cont1, typename Cont2>
std::vector<std::vector<std::uint32_t>> matchingRows(const Cont1& left, const Cont2& right) {
    std::unordered_map<T, BitSet> matches;
    for (auto&& [r, value] : util::enumerate<std::uint32_t>(right)) {
        if constexpr (firstMatchOnly) {
            if (matches.find(value) == matches.end()) {
                matches[value].add(r);
            }
        } else {
            matches[value].add(r);
        }
    }
    std::vector<std::vector<std::uint32_t>> rows(std::distance(left.begin(), left.end()));
    for (auto&& [i, key] : util::enumerate<std::uint32_t>(left)) {
        if (auto it = matches.find(key); it != matches.end()) {
            rows[i] = it->second.toVector();
        }
    }
    return rows;
}

template <typename T, bool firstMatchOnly, typename Cont1, typename Cont2>
std::vector<std::vector<std::uint32_t>> matchingRowsVector(const Cont1& left, const Cont2& right) {
    std::unordered_map<T, std::vector<std::uint32_t>> matches;
    for (auto&& [r, value] : util::enumerate<std::uint32_t>(right)) {
        if constexpr (firstMatchOnly) {
            if (matches.find(value) == matches.end()) {
                matches[value].push_back(r);
            }
        } else {
            matches[value].push_back(r);
        }
    }
    std::vector<std::vector<std::uint32_t>> rows(std::distance(left.begin(), left.end()));
    for (auto&& [i, key] : util::enumerate<std::uint32_t>(left)) {
        rows[i] = matches[key];
    }
    return rows;
}

[[maybe_unused]] void MatchingRowsPrev(benchmark::State& st) {
    auto right = util::as_range(right_.begin(), right_.begin() + st.range(0));

    for (auto _ : st) {
        auto result = matchingRowsPrev<int, false>(left_, right);
        benchmark::DoNotOptimize(result);
    }
}

[[maybe_unused]] void MatchingRows(benchmark::State& st) {
    auto right = util::as_range(right_.begin(), right_.begin() + st.range(0));

    for (auto _ : st) {
        auto result = matchingRows<int, false>(left_, right);
        benchmark::DoNotOptimize(result);
    }
}

[[maybe_unused]] void MatchingRowsVector(benchmark::State& st) {
    auto right = util::as_range(right_.begin(), right_.begin() + st.range(0));

    for (auto _ : st) {
        auto result = matchingRowsVector<int, false>(left_, right);
        benchmark::DoNotOptimize(result);
    }
}

[[maybe_unused]] void MatchingRowsFirstPrev(benchmark::State& st) {
    auto right = util::as_range(right_.begin(), right_.begin() + st.range(0));

    for (auto _ : st) {
        auto result = matchingRowsPrev<int, true>(left_, right);
        benchmark::DoNotOptimize(result);
    }
}

[[maybe_unused]] void MatchingRowsFirstVector(benchmark::State& st) {
    auto right = util::as_range(right_.begin(), right_.begin() + st.range(0));

    for (auto _ : st) {
        auto result = matchingRowsVector<int, true>(left_, right);
        benchmark::DoNotOptimize(result);
    }
}

template <typename T, typename Cont>
std::vector<std::uint32_t> selectRows(const Cont& cont, dataframefilters::Filters filters) {
    std::vector<std::uint32_t> rows;
    for (auto&& [row, value] : util::enumerate<std::uint32_t>(cont)) {
        auto test = util::overloaded{
            [v = value]([[maybe_unused]] const std::function<bool(std::int64_t)>& func) {
                if constexpr (std::is_integral_v<T>) {
                    return func(static_cast<std::int64_t>(v));
                } else {
                    (void)v;
                    return false;
                }
            },
            [v = value]([[maybe_unused]] const std::function<bool(double)>& func) {
                if constexpr (std::is_floating_point_v<T>) {
                    return func(v);
                } else {
                    (void)v;
                    return false;
                }
            },
            [](const std::function<bool(std::string_view)>&) { return false; }};
        auto op = [&](const auto& f) { return std::visit(test, f.filter); };
        if ((std::any_of(filters.include.begin(), filters.include.end(), op) ||
             filters.include.empty()) &&
            std::none_of(filters.exclude.begin(), filters.exclude.end(), op)) {
            rows.push_back(row);
        }
    }
    return rows;
}

[[maybe_unused]] void SelectRows(benchmark::State& st) {
    auto right = util::as_range(right_.begin(), right_.begin() + st.range(0));

    dataframefilters::Filters filters;
    filters.include.push_back(filters::intMatch(0, filters::NumberComp::Less, 32));
    // filters.include.push_back(filters::intMatch(0, filters::NumberComp::Greater, 48));

    for (auto _ : st) {
        auto result = selectRows<int>(right, filters);
        benchmark::DoNotOptimize(result);
    }
}

std::shared_ptr<DataFrame> createDataFrame(int size, int maxValue) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, maxValue);

    auto columnData = [&]() {
        std::vector<int> data(size);
        std::generate(data.begin(), data.end(), [&]() { return distrib(gen); });
        return data;
    };

    auto df = std::make_shared<DataFrame>();
    df->addColumn("col1", columnData());
    df->addColumn("col2", columnData());
    df->addColumn("col3", columnData());
    df->updateIndexBuffer();
    return df;
}

static void SelectRowsDataFrame(benchmark::State& st) {
    auto df = createDataFrame(static_cast<int>(st.range(0)), 100);

    dataframefilters::Filters filters;
    filters.include.push_back(filters::intMatch(1, filters::NumberComp::Less, 32));
    filters.include.push_back(filters::intMatch(1, filters::NumberComp::Greater, 48));
    filters.include.push_back(filters::intMatch(3, filters::NumberComp::Greater, 64));
    filters.exclude.push_back(filters::intMatch(2, filters::NumberComp::Greater, 20));

    for (auto _ : st) {
        auto result = dataframe::selectRows(*df.get(), filters);
        benchmark::DoNotOptimize(result);
    }
}

}  // namespace

constexpr int maxValue = 32;
constexpr int lenLeft = 32;
constexpr int lenRight = 4096 << 1;

// BENCHMARK(MatchingRowsPrev)->RangeMultiplier(2)->Range(8, lenRight);
// BENCHMARK(MatchingRows)->RangeMultiplier(2)->Range(8, lenRight);
// BENCHMARK(MatchingRowsVector)->RangeMultiplier(2)->Range(8, lenRight);
// BENCHMARK(MatchingRowsVector)->RangeMultiplier(2)->Range(8, lenRight);

// BENCHMARK(SelectRows)->RangeMultiplier(2)->Range(64, lenRight);
BENCHMARK(SelectRowsDataFrame)->RangeMultiplier(2)->Range(64, lenRight);

int main(int argc, char** argv) {
    benchmark::Initialize(&argc, argv);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, maxValue);

    for (int i = 0; i < lenLeft; ++i) {
        left_.push_back(i % maxValue);
    }
    std::shuffle(left_.begin(), left_.end(), gen);

    right_.resize(lenRight);
    std::generate(right_.begin(), right_.end(), [&]() { return distrib(gen); });

    benchmark::RunSpecifiedBenchmarks();
    return 0;
}
