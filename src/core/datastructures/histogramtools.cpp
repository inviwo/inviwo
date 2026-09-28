/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2026 Inviwo Foundation
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
#include <inviwo/core/util/threadutil.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/stdfuture.h>

#include <utility>

namespace inviwo {

HistogramCache::HistogramCache() : state_{std::make_shared<State>()} {}
HistogramCache::HistogramCache(const HistogramCache& rhs) : state_{std::make_shared<State>()} {
    const std::scoped_lock lock{rhs.state_->mutex};
    state_->histograms = rhs.state_->histograms;
    state_->callbacks = rhs.state_->callbacks;
}
HistogramCache::HistogramCache(HistogramCache&& rhs) noexcept : state_{std::move(rhs.state_)} {}
HistogramCache& HistogramCache::operator=(const HistogramCache& that) {
    if (this != &that) {
        state_ = std::make_shared<State>();
        const std::scoped_lock lock{that.state_->mutex};
        state_->histograms = that.state_->histograms;
        state_->callbacks = that.state_->callbacks;
    }
    return *this;
}
HistogramCache& HistogramCache::operator=(HistogramCache&& that) noexcept {
    if (this != &that) {
        state_ = std::move(that.state_);
    }
    return *this;
}

auto HistogramCache::calculateHistograms(const std::function<std::vector<Histogram1D>()>& calculate,
                                         const std::function<Callback>& whenDone) const -> Result {
    const std::scoped_lock lock{state_->mutex};

    Result result;

    if (util::is_future_ready(state_->histograms)) {
        if (whenDone) whenDone(state_->histograms.get());
        result.progress = Progress::Done;
    } else if (state_->histograms.valid()) {
        if (whenDone) result.handle = state_->callbacks.add(whenDone);
        result.progress = Progress::Calculating;
    } else {
        std::promise<std::shared_ptr<const std::vector<Histogram1D>>> promise{};
        state_->histograms = promise.get_future();
        if (whenDone) result.handle = state_->callbacks.add(whenDone);
        result.progress = Progress::Calculating;

        util::dispatchPool([calculate, promise = std::move(promise),
                            weakState = std::weak_ptr<State>(state_)]() mutable {
            if (auto state = weakState.lock()) {
                const auto newHistograms =
                    std::make_shared<const std::vector<Histogram1D>>(calculate());
                promise.set_value(newHistograms);

                util::dispatchFrontAndForget(
                    [weakState = std::weak_ptr<State>(state), newHistograms]() mutable {
                        if (auto state = weakState.lock()) {
                            const std::scoped_lock lock{state->mutex};
                            state->callbacks.invoke(newHistograms);
                        }
                    });
            } else {
                promise.set_value({});
            }
        });
    }

    return result;
}

std::shared_ptr<const std::vector<Histogram1D>> HistogramCache::calculateHistograms(
    const std::function<std::vector<Histogram1D>()>& calculate) const {

    std::promise<std::shared_ptr<const std::vector<Histogram1D>>> promise{};

    {
        const std::scoped_lock lock{state_->mutex};
        if (state_->histograms.valid()) {
            return state_->histograms.get();
        } else {
            state_->histograms = promise.get_future();
        }
    }

    // Calculate histogram without holding any lock
    const auto newHistograms = std::make_shared<const std::vector<Histogram1D>>(calculate());
    promise.set_value(newHistograms);

    util::dispatchFrontAndForget(
        [weakState = std::weak_ptr<State>(state_), newHistograms]() mutable {
            if (auto state = weakState.lock()) {
                const std::scoped_lock lock{state->mutex};
                state->callbacks.invoke(newHistograms);
            }
        });

    return newHistograms;
}

void HistogramCache::forEach(
    const std::function<void(const Histogram1D&, size_t)>& callback) const {
    const std::scoped_lock lock{state_->mutex};

    if (util::is_future_ready(state_->histograms)) {
        if (auto histograms = state_->histograms.get()) {
            for (auto&& [channel, histogram] : util::enumerate(*histograms)) {
                callback(histogram, channel);
            }
        }
    }
}

void HistogramCache::clear() {
    const std::scoped_lock lock{state_->mutex};

    if (!state_->histograms.valid()) {
        return;
    } else {
        state_->histograms = std::shared_future<std::shared_ptr<const std::vector<Histogram1D>>>{};
    }
}

void HistogramCache::recalculate(const std::function<std::vector<Histogram1D>()>& calculate) {
    const std::scoped_lock lock{state_->mutex};

    if (!state_->histograms.valid()) {
        return;
    } else {
        state_->histograms = std::shared_future<std::shared_ptr<const std::vector<Histogram1D>>>{};
        calculateHistograms(calculate, nullptr);
    }
}

}  // namespace inviwo
