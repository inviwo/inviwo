/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/core/datastructures/volume/temporalvolume.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwoapplicationutil.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/stdfuture.h>

#include <algorithm>

#include <fmt/format.h>
#include <fmt/chrono.h>

namespace inviwo {

ProceduralLoader::ProceduralLoader(size_t count, std::vector<Seconds> times, VolumeConfig prototype,
                                   Generator generator)
    : count_{count}
    , times_{std::move(times)}
    , prototype_{std::move(prototype)}
    , generator_{std::move(generator)} {}

std::shared_ptr<Volume> ProceduralLoader::load(size_t index, std::shared_ptr<Volume> reuse,
                                               std::stop_token stop) const {
    const Seconds time =
        (index < times_.size()) ? times_[index] : Seconds{static_cast<double>(index)};
    return generator_(index, time, std::move(reuse), std::move(stop));
}

size_t ProceduralLoader::size() const { return count_; }

Seconds ProceduralLoader::time(size_t index) const {
    return !times_.empty() ? (index < times_.size() ? times_[index] : times_.back())
                           : Seconds{index};
}

VolumeConfig ProceduralLoader::prototype() const { return prototype_; }

TemporalVolume::TemporalVolume(std::unique_ptr<TemporalVolumeLoader> loader, size_t cacheSize)
    : loader_{std::move(loader)}
    , prototype_{loader_ ? loader_->prototype() : VolumeConfig{}}
    , dataMap_{prototype_.dataMap()}
    , cacheSize_{std::max<size_t>(2, cacheSize)} {

    if (!loader_) {
        throw Exception("TemporalVolume requires a non-null VolumeLoader");
    }
}

TemporalVolume::~TemporalVolume() {
    const std::scoped_lock lock{mutex_};
    for (auto& [index, item] : cache_) {
        if (auto* pending = item.pending()) pending->source.request_stop();
    }
}

size_t TemporalVolume::size() const { return loader_->size(); }

bool TemporalVolume::empty() const { return loader_->size() == 0; }

std::pair<Seconds, Seconds> TemporalVolume::timeRange() const {
    if (empty()) {
        return {Seconds{0.0}, Seconds{0.0}};
    }
    return {times().front(), times().back()};
}

const VolumeConfig& TemporalVolume::prototype() const { return prototype_; }
const DataMapper& TemporalVolume::dataMap() const { return dataMap_; }

size_t TemporalVolume::nearestIndex(Seconds time) const {
    auto ts = times();
    const size_t n = ts.size();
    if (n == 0) {
        return 0;
    }
    if (time <= ts.front()) {
        return 0;
    }
    if (time >= ts.back()) {
        return n - 1;
    }

    const auto upper = std::ranges::upper_bound(ts, time);
    const auto ib = static_cast<size_t>(std::distance(ts.begin(), upper));
    const size_t ia = ib - 1;
    return (time - ts[ia] <= ts[ib] - time) ? ia : ib;
}

auto TemporalVolume::find(size_t index) const -> Item* {
    if (auto it = cache_.find(index); it != cache_.end()) {
        return &it->second;
    }
    return nullptr;
}

void TemporalVolume::prefetch(size_t index,
                              std::function<void(std::shared_ptr<Volume>)> callback) const {
    if (index >= size()) return;

    const std::scoped_lock lock{mutex_};

    auto* item = find(index);
    if (item && !item->reuse()) return;

    auto* app = util::getInviwoApplication();
    if (!app) return;

    auto reuse = (item && item->reuse()) ? item->reuse()->volume : takeReuse();
    std::stop_source source;
    auto future = app->dispatchPool(
        [loader = loader_, index, reuse = reuse, callback, stop = source.get_token()]() mutable {
            // Queued pool tasks cannot be discarded, so cancellation is only effective if we bail
            // out here before doing any work.
            if (stop.stop_requested()) return reuse;
            auto volume = loader->load(index, std::move(reuse), std::move(stop));
            if (volume && callback) callback(volume);
            return volume;
        });

    cache_[index] = Pending{future.share(), std::move(source)};
    touch(index);
    evict();
}

std::shared_ptr<Volume> TemporalVolume::load(std::unique_lock<std::mutex>& lock, size_t index,
                                             std::shared_ptr<Volume> reuse,
                                             std::stop_token stop) const {
    std::promise<std::shared_ptr<Volume>> promise;
    cache_[index] = Pending{.future = promise.get_future().share(), .source = {}};

    lock.unlock();
    auto volume = loader_->load(index, reuse, stop);
    lock.lock();

    std::erase(lruOrder_, index);

    if (stop.stop_requested()) {
        promise.set_value(nullptr);
        cache_[index] = Reuse{reuse};
        lruOrder_.insert(lruOrder_.begin(), index);
        return nullptr;
    } else {
        promise.set_value(volume);
        cache_[index] = Valid{volume};
        lruOrder_.insert(lruOrder_.end(), index);
        return volume;
    }
};

std::shared_ptr<const Volume> TemporalVolume::get(size_t index, std::stop_token stop) const {
    if (index >= size() || stop.stop_requested()) return nullptr;

    std::unique_lock lock{mutex_};

    if (auto* item = find(index)) {
        return item->visit(
            [&](Valid& valid) -> std::shared_ptr<Volume> {
                touch(index);
                return valid.volume;
            },
            [&](Pending& pending) -> std::shared_ptr<Volume> {
                auto future = pending.future;

                lock.unlock();
                // Poll so that our own cancellation can bail out without cancelling the shared
                // load, which other callers may still be waiting for.
                while (future.wait_for(std::chrono::milliseconds{10}) !=
                       std::future_status::ready) {
                    if (stop.stop_requested()) return nullptr;
                }
                auto volume = future.get();
                lock.lock();

                if (auto* item2 = find(index)) {
                    if (auto* pending2 = item2->pending()) {
                        if (pending2->source.stop_requested()) {
                            std::erase(lruOrder_, index);
                            if (volume) {
                                *item2 = Reuse{volume};
                                lruOrder_.insert(lruOrder_.begin(), index);
                            } else {
                                cache_.erase(index);
                            }
                            return nullptr;
                        } else {
                            *item2 = Valid{volume};
                        }
                    }
                }
                touch(index);
                return volume;
            },
            [&](Reuse& reuse) -> std::shared_ptr<Volume> {
                return load(lock, index, reuse.volume, stop);
            });

    } else {
        return load(lock, index, takeReuse(), stop);
    }
}

std::shared_ptr<const Volume> TemporalVolume::get(Seconds time, std::stop_token stop) const {
    return get(nearestIndex(time), std::move(stop));
}

TemporalVolume::Frame TemporalVolume::interpolate(Seconds time, std::stop_token stop) const {
    auto ts = times();
    const size_t n = ts.size();
    if (n == 0) {
        return {.a = nullptr, .b = nullptr, .t = 0.0};
    }
    if (n == 1 || time <= ts.front()) {
        auto volume = get(size_t{0}, stop);
        return {.a = volume, .b = volume, .t = 0.0};
    }
    if (time >= ts.back()) {
        auto volume = get(n - 1, stop);
        return {.a = volume, .b = volume, .t = 0.0};
    }

    const auto upper = std::ranges::upper_bound(ts, time);
    const auto ib = static_cast<size_t>(std::distance(ts.begin(), upper));
    const size_t ia = ib - 1;
    const Seconds ta = ts[ia];
    const Seconds tb = ts[ib];
    const double factor = (tb > ta) ? (time - ta) / (tb - ta) : 0.0;
    return {.a = get(ia, stop), .b = get(ib, stop), .t = factor};
}

void TemporalVolume::setCacheSize(size_t n) {
    const std::scoped_lock lock{mutex_};
    cacheSize_ = std::max<size_t>(2, n);
    evict();
}

size_t TemporalVolume::cacheSize() const {
    const std::scoped_lock lock{mutex_};
    return cacheSize_;
}

size_t TemporalVolume::numCached() const {
    const std::scoped_lock lock{mutex_};
    return cache_.size();
}

void TemporalVolume::clearCache() {
    const std::scoped_lock lock{mutex_};
    for (auto& [index, item] : cache_) {
        if (auto* pending = item.pending()) pending->source.request_stop();
    }
    cache_.clear();
    lruOrder_.clear();
}

void TemporalVolume::touch(size_t index) const {
    std::erase(lruOrder_, index);
    lruOrder_.push_back(index);
}

void TemporalVolume::evict() const {
    while (cache_.size() > cacheSize_ && !lruOrder_.empty()) {
        const size_t lru = lruOrder_.front();
        lruOrder_.erase(lruOrder_.begin());
        if (auto it = cache_.find(lru); it != cache_.end()) {
            cache_.erase(it);
        }
    }
}

std::shared_ptr<Volume> TemporalVolume::takeReuse() const {
    if (cache_.size() >= cacheSize_) {
        for (auto index : lruOrder_) {
            if (auto rit = cache_.find(index); rit != cache_.end()) {
                if (auto found = rit->second.visit(
                        [](Valid& valid) -> std::shared_ptr<Volume> {
                            return valid.volume.use_count() == 1 ? valid.volume : nullptr;
                        },
                        [](Pending& pending) -> std::shared_ptr<Volume> {
                            return util::is_future_ready(pending.future) ? pending.future.get()
                                                                         : nullptr;
                        },
                        [](Reuse& reuse) -> std::shared_ptr<Volume> {
                            return reuse.volume.use_count() == 1 ? reuse.volume : nullptr;
                        })) {
                    std::erase(lruOrder_, index);
                    cache_.erase(rit);
                    return found;
                }
            }
        }
    }
    return nullptr;
}

Document DataTraits<TemporalVolume>::info(const TemporalVolume& data) {
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;

    Document doc;
    doc.append("b", "Temporal Volume", {{"style", "color:white;"}});
    utildoc::TableBuilder tb(doc.handle(), P::end());
    tb(H("Frames"), data.size());
    if (!data.empty()) {
        const auto [first, last] = data.timeRange();
        tb(H("Time Range"), fmt::format("[{}, {}]", first, last));

        const VolumeConfig& prototype = data.prototype();
        tb(H("Dimensions"), prototype.dimensions.value_or(VolumeConfig::defaultDimensions));
        tb(H("Format"),
           (prototype.format ? prototype.format : VolumeConfig::defaultFormat)->getString());
    }
    return doc;
}

}  // namespace inviwo
