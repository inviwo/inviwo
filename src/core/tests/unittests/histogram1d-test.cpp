/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <inviwo/core/algorithm/histogram1d.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/datastructures/datamapper.h>

#include <vector>
#include <ranges>
#include <random>
#include <span>

namespace inviwo {

bool isHistogramUniform(const Histogram1D& histogram) {
    if (histogram.counts.size() <= 1) {
        return true;
    }
    // compare all bins except last one since this one may contain less elements
    return std::ranges::all_of(std::span(histogram.counts.begin(), histogram.counts.end() - 1),
                               [first = histogram.counts[0]](auto& v) { return v == first; }) &&
           (histogram.counts.back() <= histogram.counts[0]);
}

class Histogram1DTest : public ::testing::Test {
public:
    Histogram1DTest() : rand_{} {}

    std::vector<Histogram1D> createHistogram(double minValue, double maxValue, size_t numValues,
                                             size_t binCount) {
        std::vector<double> data(numValues);
        std::ranges::generate(data,
                              [&]() { return dis_(rand_) * (maxValue - minValue) + minValue; });
        const DataMapper dataMap{dvec2{minValue, maxValue}};
        return util::calculateHistograms<double>(data, dataMap, binCount);
    }

    template <typename T>
    std::vector<Histogram1D> createUniformHistogram(int maxValue, int inc, size_t binCount) const {
        auto seq = util::make_sequence<int>(0, maxValue, inc);
        std::vector<T> data{seq.begin(), seq.end()};
        const DataMapper dataMap{dvec2{0.0, static_cast<double>(maxValue)}};
        return util::calculateHistograms<T>(data, dataMap, binCount);
    }

protected:
    virtual void SetUp() override { rand_.seed(static_cast<std::mt19937::result_type>(0)); };

private:
    std::mt19937 rand_;
    std::uniform_real_distribution<double> dis_;
};

TEST_F(Histogram1DTest, intRangeBelowBinCount) {
    const size_t binCount = 64;

    auto histograms = createUniformHistogram<int>(62, 1, binCount);
    EXPECT_EQ(63, histograms[0].counts.size()) << "number of bins differs";
    const double effectiveRange =
        histograms[0].effectiveDataRange.y - histograms[0].effectiveDataRange.x;
    EXPECT_DOUBLE_EQ(62.0, effectiveRange);

    EXPECT_TRUE(isHistogramUniform(histograms[0])) << "different counts per bin";
}

TEST_F(Histogram1DTest, intRangeExactBinCount) {
    const size_t binCount = 64;

    auto histograms = createUniformHistogram<int>(63, 1, binCount);
    EXPECT_EQ(binCount, histograms[0].counts.size()) << "number of bins differs";
    const double effectiveRange =
        histograms[0].effectiveDataRange.y - histograms[0].effectiveDataRange.x;
    EXPECT_DOUBLE_EQ(63.0, effectiveRange);

    EXPECT_TRUE(isHistogramUniform(histograms[0])) << "different counts per bin";
}

TEST_F(Histogram1DTest, intRangeMultipleBinCount) {
    const size_t binCount = 32;

    auto histograms = createUniformHistogram<int>(62, 1, binCount);
    EXPECT_EQ(binCount, histograms[0].counts.size()) << "number of bins differs";
    const double effectiveRange =
        histograms[0].effectiveDataRange.y - histograms[0].effectiveDataRange.x;
    EXPECT_DOUBLE_EQ(62.0, effectiveRange);

    EXPECT_TRUE(isHistogramUniform(histograms[0])) << "different counts per bin";
}

TEST_F(Histogram1DTest, intRangeGreaterThanBinCount) {
    const size_t binCount = 32;

    auto histograms = createUniformHistogram<int>(63, 1, binCount);
    EXPECT_EQ(22, histograms[0].counts.size()) << "number of bins differs";
    const double effectiveRange =
        histograms[0].effectiveDataRange.y - histograms[0].effectiveDataRange.x;
    EXPECT_DOUBLE_EQ(63.0, effectiveRange);

    EXPECT_TRUE(isHistogramUniform(histograms[0])) << "different counts per bin";
}

TEST_F(Histogram1DTest, floatRangeBelowBinCount) {
    const size_t binCount = 64;

    auto histograms = createUniformHistogram<double>(62, 1, binCount);
    EXPECT_EQ(binCount, histograms[0].counts.size()) << "number of bins differs";
    const double dataRange = histograms[0].dataMap.dataRange.y - histograms[0].dataMap.dataRange.x;
    const double effectiveRange =
        histograms[0].effectiveDataRange.y - histograms[0].effectiveDataRange.x;
    EXPECT_DOUBLE_EQ(dataRange, effectiveRange) << "effective range does not match data range";

    EXPECT_EQ(62, histograms[0].totalCounts) << "different total counts";
}

TEST_F(Histogram1DTest, floatRangeExactBinCount) {
    const size_t binCount = 64;

    auto histograms = createUniformHistogram<double>(63, 1, binCount);
    EXPECT_EQ(binCount, histograms[0].counts.size()) << "number of bins differs";
    const double effectiveRange =
        histograms[0].effectiveDataRange.y - histograms[0].effectiveDataRange.x;
    const double dataRange = histograms[0].dataMap.dataRange.y - histograms[0].dataMap.dataRange.x;
    EXPECT_DOUBLE_EQ(dataRange, effectiveRange) << "effective range does not match data range";

    EXPECT_TRUE(isHistogramUniform(histograms[0])) << "different counts per bin";
}

TEST_F(Histogram1DTest, floatRangeGreaterThanBinCount) {
    const size_t binCount = 64;

    auto histograms = createUniformHistogram<double>(64, 1, binCount);
    EXPECT_EQ(33, histograms[0].counts.size()) << "number of bins differs";
    const double effectiveRange =
        histograms[0].effectiveDataRange.y - histograms[0].effectiveDataRange.x;
    EXPECT_DOUBLE_EQ(64.0, effectiveRange) << "effective range does not match data range";

    EXPECT_TRUE(isHistogramUniform(histograms[0])) << "different counts per bin";
}

TEST_F(Histogram1DTest, floatNonfractionalRange) {
    const size_t binCount = 16;

    auto histograms = createUniformHistogram<double>(20, 1, binCount);
    EXPECT_EQ(11, histograms[0].counts.size()) << "number of bins differs";
    const double effectiveRange =
        histograms[0].effectiveDataRange.y - histograms[0].effectiveDataRange.x;
    EXPECT_DOUBLE_EQ(20.0, effectiveRange);

    EXPECT_TRUE(isHistogramUniform(histograms[0])) << "different counts per bin";
}

TEST_F(Histogram1DTest, floatFractionalRangeLower) {
    const size_t binCount = 16;

    auto histograms = createHistogram(0.123, 67.0, 20, binCount);
    EXPECT_EQ(binCount, histograms[0].counts.size()) << "number of bins differs";

    EXPECT_EQ(20, histograms[0].totalCounts) << "different total counts";
}

TEST_F(Histogram1DTest, floatFractionalRangeUpper) {
    const size_t binCount = 16;

    auto histograms = createHistogram(0.0, 67.998, 20, binCount);
    EXPECT_EQ(binCount, histograms[0].counts.size()) << "number of bins differs";

    EXPECT_EQ(20, histograms[0].totalCounts) << "different total counts";
}

}  // namespace inviwo
