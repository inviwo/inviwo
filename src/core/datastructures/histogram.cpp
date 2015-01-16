/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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
    : maximumBinCount_(1.f)
    , valid_(false) {
    data_ = new std::vector<double>(numberOfBins, 0.f);
}

NormalizedHistogram::NormalizedHistogram(const NormalizedHistogram* rhs) {
    if (rhs) {
        maximumBinCount_ = rhs->getMaximumBinValue();
        stats_ = rhs->stats_;
        valid_ = rhs->isValid();
        data_ = new std::vector<double>(rhs->getData()->size(), 0.f);
        std::copy(rhs->getData()->begin(), rhs->getData()->end(), data_->begin());
    }
    else {
        data_ = new std::vector<double>(256, 0.f);
        maximumBinCount_ = 1.f;
        valid_ = false;
    }
}

NormalizedHistogram::NormalizedHistogram(const NormalizedHistogram &rhs) {
    maximumBinCount_ = rhs.getMaximumBinValue();
    stats_ = rhs.stats_;
    valid_ = rhs.isValid();
    data_ = new std::vector<double>(rhs.getData()->size(), 0.f);
    std::copy(rhs.getData()->begin(), rhs.getData()->end(), data_->begin());
}

NormalizedHistogram::~NormalizedHistogram() {
    delete data_;
    data_ = NULL;
}

NormalizedHistogram& NormalizedHistogram::operator=(const NormalizedHistogram &rhs) {
    if (this != &rhs) {
        maximumBinCount_ = rhs.getMaximumBinValue();
        stats_ = rhs.stats_;
        valid_ = rhs.isValid();
        if (!data_) {
            data_ = new std::vector<double>(rhs.getData()->size(), 0.f);
        }
        else {
            data_->resize(rhs.getData()->size());
        }
        std::copy(rhs.getData()->begin(), rhs.getData()->end(), data_->begin());
    }
    return *this;
}


std::vector<double>* NormalizedHistogram::getData() {
    return data_;
}

const std::vector<double>* NormalizedHistogram::getData() const {
    return data_;
}

bool NormalizedHistogram::exists() const {
    if (!data_ || data_->empty())
        return true;

    return false;
}

void NormalizedHistogram::setValid(bool valid) {
    valid_ = valid;
}

bool NormalizedHistogram::isValid() const {
    return valid_;
}

void NormalizedHistogram::resize(size_t numberOfBins) {
    if (numberOfBins != data_->size())
        data_->resize(numberOfBins);
}

void NormalizedHistogram::performNormalization() {
    // Find bin with largest count
    maximumBinCount_ = *std::max_element(data_->begin(), data_->end());

    // Normalize all bins with the largest count
    std::transform(data_->begin(), data_->end(), data_->begin(),
                   std::bind2nd(std::multiplies<double>(), 1.0 / maximumBinCount_));
}

double NormalizedHistogram::getMaximumBinValue() const {
    return maximumBinCount_;
}

void NormalizedHistogram::calculatePercentiles() {
    double sum = std::accumulate(data_->begin(), data_->end(), 0.0);
    stats_.percentiles.resize(101);
    double accumulation = 0;
    size_t i = 0;
    for (size_t j = 0; j < data_->size(); ++j){
        accumulation += data_->at(j);
        while (accumulation / sum >= static_cast<double>(i) / 100.0) {
            stats_.percentiles[i] = static_cast<double>(j) / static_cast<double>(data_->size() - 1)
                    * (dataRange_.y - dataRange_.x) + dataRange_.x;
            i++;
        }
    }
}

double sumSquare(double a, double b) {
    return a + b*b;
}

void NormalizedHistogram::calculateHistStats() {
    std::vector<double> temp = std::vector<double>(data_->size(), 0.0);
    std::copy(data_->begin(), data_->end(), temp.begin());
    std::sort(temp.begin(), temp.end());
    double sum = std::accumulate(data_->begin(), data_->end(), 0.0);
    double sum2 = std::accumulate(data_->begin(), data_->end(), 0.0, sumSquare);

    histStats_.min = *temp.begin();
    histStats_.max = *(temp.end() - 1);
    histStats_.mean = sum / static_cast<double>(data_->size());
    histStats_.standardDeviation =
        std::sqrt((data_->size() * sum2 - sum * sum) / (data_->size() * (data_->size() - 1)));

    histStats_.percentiles.resize(101, 0.0);
    for (size_t i = 1; i < histStats_.percentiles.size(); ++i) {
        histStats_.percentiles[i] = temp.at(static_cast<size_t>(
            std::ceil(static_cast<double>(i) / static_cast<double>(histStats_.percentiles.size()-1) *
            static_cast<double>(data_->size()))) - 1);
    }
}

} // namespace

