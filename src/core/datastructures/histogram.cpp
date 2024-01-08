/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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

#include <inviwo/core/datastructures/histogram.h>
#include <algorithm>
#include <numeric>
#include <functional>

namespace inviwo {

NormalizedHistogram::NormalizedHistogram() : stats_{0.0, 0.0, 0.0, 0.0, {}}, dataRange_{0.0, 0.0} {}

NormalizedHistogram::NormalizedHistogram(dvec2 dataRange, std::vector<double> counts, double min,
                                         double max, double mean, double standardDeviation)
    : stats_{min, max, mean, standardDeviation, {}}
    , dataRange_{dataRange}
    , data_{std::move(counts)} {

    // calculatePercentiles
    {
        double sum = std::accumulate(data_.begin(), data_.end(), 0.0);
        stats_.percentiles.resize(101);
        double accumulation = 0;
        size_t i = 0;
        for (size_t j = 0; j < data_.size(); ++j) {
            accumulation += data_[j];
            while (accumulation / sum >= static_cast<double>(i) / 100.0) {
                stats_.percentiles[i] = static_cast<double>(j) /
                                            static_cast<double>(data_.size() - 1) *
                                            (dataRange_.y - dataRange_.x) +
                                        dataRange_.x;
                i++;
            }
        }
    }

    // Find bin with largest count
    maximumBinCount_ = *std::max_element(data_.begin(), data_.end());

    // Normalize all bins with the largest count
    std::transform(data_.begin(), data_.end(), data_.begin(),
                   [factor = 1.0 / maximumBinCount_](auto val) { return val * factor; });

    {
        std::vector<double> temp{data_};
        std::sort(temp.begin(), temp.end());
        double sum = std::accumulate(data_.begin(), data_.end(), 0.0);
        double sum2 = std::accumulate(data_.begin(), data_.end(), 0.0,
                                      [](double a, double b) { return a + b * b; });

        histStats_.min = *temp.begin();
        histStats_.max = *(temp.end() - 1);
        histStats_.mean = sum / static_cast<double>(data_.size());
        histStats_.standardDeviation =
            std::sqrt((data_.size() * sum2 - sum * sum) / (data_.size() * (data_.size() - 1)));

        histStats_.percentiles.resize(101, 0.0);
        for (size_t i = 1; i < histStats_.percentiles.size(); ++i) {
            histStats_.percentiles[i] =
                temp.at(static_cast<size_t>(
                            std::ceil(static_cast<double>(i) /
                                      static_cast<double>(histStats_.percentiles.size() - 1) *
                                      static_cast<double>(data_.size()))) -
                        1);
        }
    }
}

double NormalizedHistogram::getMaximumBinValue() const { return maximumBinCount_; }

std::vector<double>& NormalizedHistogram::getData() { return data_; }

const std::vector<double>& NormalizedHistogram::getData() const { return data_; }

double& NormalizedHistogram::operator[](size_t i) { return data_[i]; }

const double& NormalizedHistogram::operator[](size_t i) const { return data_[i]; }

size_t HistogramContainer::size() const { return histograms_.size(); }

bool HistogramContainer::empty() const { return histograms_.empty(); }

const NormalizedHistogram& HistogramContainer::operator[](size_t i) const { return histograms_[i]; }

const NormalizedHistogram& HistogramContainer::get(size_t i) const { return histograms_[i]; }

NormalizedHistogram& HistogramContainer::operator[](size_t i) { return histograms_[i]; }

NormalizedHistogram& HistogramContainer::get(size_t i) { return histograms_[i]; }

void HistogramContainer::clear() { histograms_.clear(); }

}  // namespace inviwo
