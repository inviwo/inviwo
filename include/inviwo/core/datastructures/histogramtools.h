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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/dispatcher.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/datastructures/volume/volumeram.h>

#include <atomic>
#include <memory>
#include <vector>

namespace inviwo {

class HistogramSupplier;

class IVW_CORE_API HistogramCalculationState {
public:
    friend HistogramSupplier;
    HistogramCalculationState(std::weak_ptr<HistogramContainer> container, size_t bins,
                              dvec2 dataRange)
        : container_{container}
        , stop_{std::make_shared<std::atomic<bool>>(false)}
        , bins_{bins}
        , dataRange_{dataRange} {}

    ~HistogramCalculationState() { *stop_ = true; }

    void whenDone(std::function<void(const HistogramContainer&)> callback);

    size_t getBins() const { return bins_; }
    dvec2 getDataRange() const { return dataRange_; }

private:
    std::weak_ptr<HistogramContainer> container_;
    Dispatcher<void(const HistogramContainer&)> callbacks_;
    std::vector<std::shared_ptr<std::function<void(const HistogramContainer&)>>> callbackHandles_;
    std::shared_ptr<std::atomic<bool>> stop_;
    bool done = false;

    size_t bins_;
    dvec2 dataRange_;
};

class IVW_CORE_API HistogramSupplier {
public:
    HistogramSupplier();
    HistogramSupplier(const HistogramSupplier& rhs);
    HistogramSupplier(HistogramSupplier&& rhs) = default;
    HistogramSupplier& operator=(const HistogramSupplier& that);
    HistogramSupplier& operator=(HistogramSupplier&& that) = default;

    virtual ~HistogramSupplier() = default;

    bool hasHistograms() const { return !histograms_->empty(); }
    const HistogramContainer& getHistograms() const { return *histograms_; }
    HistogramContainer& getHistograms() { return *histograms_; }

protected:
    std::shared_ptr<HistogramCalculationState> startCalculation(
        std::shared_ptr<const VolumeRAM> volumeRam, dvec2 dataRange, size_t bins) const;

private:
    static void done(std::shared_ptr<HistogramCalculationState> state,
                     HistogramContainer histograms);

    mutable std::shared_ptr<HistogramCalculationState> calculation_;
    mutable std::shared_ptr<HistogramContainer> histograms_;
};

}  // namespace inviwo
