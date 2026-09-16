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

std::shared_ptr<Volume> ProceduralLoader::load(size_t index, std::shared_ptr<Volume> reuse) {
    const Seconds time =
        (index < times_.size()) ? times_[index] : Seconds{static_cast<double>(index)};
    return generator_(index, time, std::move(reuse));
}

size_t ProceduralLoader::size() const { return count_; }

Seconds ProceduralLoader::time(size_t index) const { return times_[index]; }

VolumeConfig ProceduralLoader::prototype() const { return prototype_; }

TemporalVolume::TemporalVolume(std::unique_ptr<VolumeLoader> loader, size_t cacheSize)
    : loader_{std::move(loader)}
    , prototype_{loader_ ? loader_->prototype() : VolumeConfig{}}
    , dataMap_{prototype_.dataMap()}
    , cacheSize_{std::max<size_t>(2, cacheSize)} {

    if (!loader_) {
        throw Exception("TemporalVolume requires a non-null VolumeLoader");
    }
}

TemporalVolume::~TemporalVolume() {
    // Wait for all in-flight prefetches before destroying the loader, since the pool tasks
    // capture a raw pointer to it.
    std::unordered_map<size_t, Item> cache;
    {
        const std::scoped_lock lock{mutex_};
        cache = std::move(cache_);
        cache_.clear();
    }
    for (auto& [index, item] : cache) {
        item.visit(
            [](std::shared_ptr<Volume>& volume) -> void {
                // Do nothing, volume is already in the cache
            },
            [](std::future<std::shared_ptr<Volume>>& future) -> void {
                // Wait for the future to complete
                if (future.valid()) future.wait();
            });
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

std::shared_ptr<const Volume> TemporalVolume::get(size_t index) const {
    if (index >= size()) {
        return nullptr;
    }

    std::unique_lock lock{mutex_};

    if (auto it = cache_.find(index); it != cache_.end()) {
        touch(index);

        return it->second.visit(
            [](std::shared_ptr<Volume>& volume) -> std::shared_ptr<Volume> { return volume; },
            [&, i = it->first](
                std::future<std::shared_ptr<Volume>>& future) -> std::shared_ptr<Volume> {
                lock.unlock();
                auto volume = future.get();
                lock.lock();
                cache_[i] = volume;
                return volume;
            });

    } else {
        auto reuse = takeReuse();
        lock.unlock();
        auto volume = loader_->load(index, std::move(reuse));
        lock.lock();
        return insert(index, std::move(volume));
    }
}

std::shared_ptr<const Volume> TemporalVolume::get(Seconds time) const {
    return get(nearestIndex(time));
}

TemporalVolume::Frame TemporalVolume::interpolate(Seconds time) const {
    auto ts = times();
    const size_t n = ts.size();
    if (n == 0) {
        return {.a = nullptr, .b = nullptr, .t = 0.0};
    }
    if (n == 1 || time <= ts.front()) {
        auto volume = get(size_t{0});
        return {.a = volume, .b = volume, .t = 0.0};
    }
    if (time >= ts.back()) {
        auto volume = get(n - 1);
        return {.a = volume, .b = volume, .t = 0.0};
    }

    const auto upper = std::ranges::upper_bound(ts, time);
    const auto ib = static_cast<size_t>(std::distance(ts.begin(), upper));
    const size_t ia = ib - 1;
    const Seconds ta = ts[ia];
    const Seconds tb = ts[ib];
    const double factor = (tb > ta) ? (time - ta) / (tb - ta) : 0.0;
    return {.a = get(ia), .b = get(ib), .t = factor};
}

void TemporalVolume::prefetch(size_t index,
                              std::function<void(std::shared_ptr<Volume>)> callback) const {
    if (index >= size()) {
        return;
    }

    const std::scoped_lock lock{mutex_};
    if (cache_.contains(index)) {
        return;
    }

    auto* app = util::getInviwoApplication();
    if (!app) {
        // No thread pool available, prefetching is a no-op. Frames will be loaded synchronously
        // on demand in get().
        return;
    }

    cache_.emplace(index, app->dispatchPool([loader = loader_.get(), index, reuse = takeReuse(),
                                             callback]() mutable {
        auto volume = loader->load(index, std::move(reuse));
        if (callback) callback(volume);
        return volume;
    }));
    touch(index);
    evict();
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
    cache_.clear();
    lruOrder_.clear();
}

std::shared_ptr<const Volume> TemporalVolume::insert(size_t index,
                                                     std::shared_ptr<Volume> volume) const {
    if (auto it = cache_.find(index); it != cache_.end()) {
        // Another thread already inserted this frame while we were loading.
        touch(index);
        if (it->second.ready()) {
            return it->second.volume();
        } else {
            it->second = volume;
            return volume;
        }
    }

    cache_.emplace(index, volume);
    lruOrder_.push_front(index);
    evict();
    return volume;
}

void TemporalVolume::touch(size_t index) const {
    lruOrder_.remove(index);
    lruOrder_.push_front(index);
}

void TemporalVolume::evict() const {
    while (cache_.size() > cacheSize_ && !lruOrder_.empty()) {
        const size_t lru = lruOrder_.back();
        lruOrder_.pop_back();
        if (auto it = cache_.find(lru); it != cache_.end()) {
            cache_.erase(it);
        }
    }
}

std::shared_ptr<Volume> TemporalVolume::takeReuse() const {
    std::shared_ptr<Volume> reuse = nullptr;
    if (cache_.size() > 2 && !lruOrder_.empty()) {
        const auto lru = lruOrder_.back();
        if (auto rit = cache_.find(lru); rit != cache_.end()) {
            if (rit->second.ready() && rit->second.volume().use_count() == 1) {
                reuse = rit->second.volume();
                lruOrder_.pop_back();
                cache_.erase(rit);
            }
        }
    }
    return reuse;
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
