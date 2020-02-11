/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2020 Inviwo Foundation
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

#include <inviwo/core/datastructures/histogramtools.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

void HistogramCalculationState::whenDone(std::function<void(const HistogramContainer&)> callback) {
    if (auto container = container_.lock(); container && done) {
        callback(*container);
    } else {
        callbackHandles_.push_back(callbacks_.add(callback));
    }
}

HistogramSupplier::HistogramSupplier() : histograms_{std::make_shared<HistogramContainer>()} {}

HistogramSupplier::HistogramSupplier(const HistogramSupplier& rhs)
    : histograms_{std::make_shared<HistogramContainer>(*rhs.histograms_)} {}

HistogramSupplier& HistogramSupplier::operator=(const HistogramSupplier& that) {
    if (this != &that) {
        histograms_ = std::make_shared<HistogramContainer>(*that.histograms_);
    }
    return *this;
}

std::shared_ptr<HistogramCalculationState> HistogramSupplier::startCalculation(
    std::shared_ptr<const VolumeRAM> volumeRam, dvec2 dataRange, size_t bins) const {
    if (!calculation_ || calculation_->getBins() != bins ||
        calculation_->getDataRange() != dataRange) {

        histograms_ = std::make_shared<HistogramContainer>();
        calculation_ = std::make_shared<HistogramCalculationState>(histograms_, bins, dataRange);

        dispatchPool([weakState = std::weak_ptr<HistogramCalculationState>(calculation_),
                      stop = calculation_->stop_, volumeRam, dataRange, bins]() {
            auto histograms = volumeRam->dispatch<HistogramContainer>([&](auto vr) {
                return HistogramContainer(dataRange, bins, vr->getDataTyped(),
                                          vr->getDataTyped() + glm::compMul(vr->getDimensions()));
            });
            if (*stop) return;
            dispatchFrontAndForget([hist = std::move(histograms), weakState]() {
                if (auto s = weakState.lock()) {
                    done(s, std::move(hist));
                }
            });
        });
    }
    return calculation_;
}

void HistogramSupplier::done(std::shared_ptr<HistogramCalculationState> state,
                             HistogramContainer histograms) {
    state->callbacks_.invoke(histograms);
    state->done = true;
    if (auto container = state->container_.lock()) {
        *container = std::move(histograms);
    }
}

}  // namespace inviwo
