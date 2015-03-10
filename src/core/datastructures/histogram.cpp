/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

NormalizedHistogram::NormalizedHistogram(size_t numberOfBins)
    : maximumBinCount_(1.f), valid_(false), data_(numberOfBins, 0.f) {}

NormalizedHistogram::NormalizedHistogram(const NormalizedHistogram& rhs)
    : maximumBinCount_(rhs.maximumBinCount_)
    , stats_(rhs.stats_)
    , valid_(rhs.valid_)
    , data_(rhs.data_) {}

NormalizedHistogram& NormalizedHistogram::operator=(const NormalizedHistogram &that) {
    if (this != &that) {
        maximumBinCount_ = that.getMaximumBinValue();
        stats_ = that.stats_;
        valid_ = that.valid_;
        data_.resize(that.data_.size());
        std::copy(that.data_.begin(), that.data_.end(), data_.begin());
    }
    return *this;
}

NormalizedHistogram* NormalizedHistogram::clone() const {
    return new NormalizedHistogram(*this);
}

NormalizedHistogram::~NormalizedHistogram() {}

std::vector<double>* NormalizedHistogram::getData() {
    return &data_;
}

const std::vector<double>* NormalizedHistogram::getData() const {
    return &data_;
}

double& NormalizedHistogram::operator[](size_t i) {
    return data_[i];
}

const double& NormalizedHistogram::operator[](size_t i) const {
    return data_[i];
}

bool NormalizedHistogram::exists() const {
    return data_.empty();
}

void NormalizedHistogram::setValid(bool valid) {
    valid_ = valid;
}

bool NormalizedHistogram::isValid() const {
    return valid_;
}

void NormalizedHistogram::resize(size_t numberOfBins) {
    if (numberOfBins != data_.size()) {
        data_.resize(numberOfBins);
        valid_ = false;
    }
}

void NormalizedHistogram::performNormalization() {
    // Find bin with largest count
    maximumBinCount_ = *std::max_element(data_.begin(), data_.end());

    // Normalize all bins with the largest count
    std::transform(data_.begin(), data_.end(), data_.begin(),
                   std::bind2nd(std::multiplies<double>(), 1.0 / maximumBinCount_));
}

double NormalizedHistogram::getMaximumBinValue() const {
    return maximumBinCount_;
}

void NormalizedHistogram::calculatePercentiles() {
    double sum = std::accumulate(data_.begin(), data_.end(), 0.0);
    stats_.percentiles.resize(101);
    double accumulation = 0;
    size_t i = 0;
    for (size_t j = 0; j < data_.size(); ++j){
        accumulation += data_[j];
        while (accumulation / sum >= static_cast<double>(i) / 100.0) {
            stats_.percentiles[i] = static_cast<double>(j) / static_cast<double>(data_.size() - 1)
                    * (dataRange_.y - dataRange_.x) + dataRange_.x;
            i++;
        }
    }
}

void NormalizedHistogram::calculateHistStats() {
    std::vector<double> temp = std::vector<double>(data_.size(), 0.0);
    std::copy(data_.begin(), data_.end(), temp.begin());
    std::sort(temp.begin(), temp.end());
    double sum = std::accumulate(data_.begin(), data_.end(), 0.0);
    double sum2 = std::accumulate(data_.begin(), data_.end(), 0.0, 
        [](double a, double b) { return a + b*b; });

    histStats_.min = *temp.begin();
    histStats_.max = *(temp.end() - 1);
    histStats_.mean = sum / static_cast<double>(data_.size());
    histStats_.standardDeviation =
        std::sqrt((data_.size() * sum2 - sum * sum) / (data_.size() * (data_.size() - 1)));

    histStats_.percentiles.resize(101, 0.0);
    for (size_t i = 1; i < histStats_.percentiles.size(); ++i) {
        histStats_.percentiles[i] = temp.at(static_cast<size_t>(
            std::ceil(static_cast<double>(i) / static_cast<double>(histStats_.percentiles.size()-1) *
            static_cast<double>(data_.size()))) - 1);
    }
}

HistogramContainer::HistogramContainer() {}

HistogramContainer::HistogramContainer(const HistogramContainer& rhs) {
    for (auto h : rhs.histograms_)
        histograms_.push_back(h->clone());
}

HistogramContainer::HistogramContainer(HistogramContainer&& rhs) : histograms_(rhs.histograms_) {
    rhs.histograms_.clear();
}

HistogramContainer& HistogramContainer::operator=(HistogramContainer&& that) {
    if (this != &that) {
        histograms_ = that.histograms_;
        that.histograms_.clear();
    }
    return *this;
}

NormalizedHistogram& HistogramContainer::operator[](size_t i) const {
    return *histograms_[i];
}

HistogramContainer* HistogramContainer::clone() const {
    return new HistogramContainer(*this);
}

HistogramContainer::~HistogramContainer() {
    for (auto h : histograms_)
        delete h;
}

size_t HistogramContainer::size() const {
    return histograms_.size();
}

bool HistogramContainer::empty() const {
    return histograms_.empty();
}

NormalizedHistogram* HistogramContainer::get(size_t i) const {
    return histograms_[i];
}

void HistogramContainer::add(NormalizedHistogram* hist) {
    histograms_.push_back(hist);
}

void HistogramContainer::setValid(bool valid) {
    for (auto h : histograms_) h->setValid(valid);
}

bool HistogramContainer::isValid() const {
    for (auto h : histograms_)
        if(!h->isValid()) return false;

    return true;
}

HistogramContainer& HistogramContainer::operator=(const HistogramContainer& that) {
    if (this != &that) {
        for (auto h : histograms_) delete h;
        histograms_.clear();

        for (auto h : that.histograms_) histograms_.push_back(h->clone());
    }
    return *this;
}

} // namespace

