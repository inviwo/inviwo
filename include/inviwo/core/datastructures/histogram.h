/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2026 Inviwo Foundation
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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/datastructures/datatraits.h>

#include <iterator>
#include <vector>
#include <bitset>
#include <array>

namespace inviwo {
enum class HistogramMode : int { Off = 0, All, P99, P95, P90, Log };
constexpr size_t numberOfHistogramModes = 6;

using HistogramSelection = std::bitset<32>;
constexpr HistogramSelection histogramSelectionAll{0xffffffff};

struct IVW_CORE_API Statistics {
    double min{0.0};
    double max{0.0};
    double mean{0.0};
    double standardDeviation{0.0};
    std::vector<double> percentiles;
};

struct IVW_CORE_API Histogram1D {
    std::vector<size_t> counts;
    size_t totalCounts{0};
    size_t maxCount{0};
    DataMapper dataMap{};
    size_t underflow{0};
    size_t overflow{0};

    Statistics dataStats;
    Statistics histStats;
    std::string name;
};

struct IVW_CORE_API Histogram2D {
    std::vector<size_t> counts;
    size2_t dimensions{0};
    size_t totalCounts{0};
    size_t maxCount{0};
    std::array<DataMapper, 2> dataMap{};
    size_t underflow{0};
    size_t overflow{0};
    std::array<Statistics, 2> dataStats;
    std::array<Statistics, 2> histStats;
    std::string name;
};

template <>
struct DataTraits<Histogram1D> {
    static constexpr std::string_view classIdentifier() { return "org.inviwo.Histogram1D"; }
    static constexpr std::string_view dataName() { return "Histogram1D"; }
    static constexpr uvec3 colorCode() { return {235, 20, 88}; }
    IVW_CORE_API static Document info(const Histogram1D& histogram);
};

template <>
struct DataTraits<Histogram2D> {
    static constexpr std::string_view classIdentifier() { return "org.inviwo.Histogram2D"; }
    static constexpr std::string_view dataName() { return "Histogram2D"; }
    static constexpr uvec3 colorCode() { return {235, 88, 20}; }
    IVW_CORE_API static Document info(const Histogram2D& histogram);
};

IVW_CORE_API std::string format_as(const Statistics& stats);
IVW_CORE_API std::string format_as(const Histogram1D& stats);
IVW_CORE_API std::string format_as(const Histogram2D& stats);


}  // namespace inviwo
