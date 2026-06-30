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

namespace inviwo {

ProceduralLoader::ProceduralLoader(size_t count, std::vector<double> times,
                                   std::shared_ptr<const Volume> prototype, Generator generator)
    : count_{count}
    , times_{std::move(times)}
    , prototype_{std::move(prototype)}
    , generator_{std::move(generator)} {}

std::shared_ptr<Volume> ProceduralLoader::load(size_t index) {
    const double time = (index < times_.size()) ? times_[index] : static_cast<double>(index);
    return generator_(index, time);
}

size_t ProceduralLoader::size() const { return count_; }

std::span<const double> ProceduralLoader::times() const { return times_; }

std::shared_ptr<const Volume> ProceduralLoader::prototype() const { return prototype_; }

namespace {

std::vector<double> resolveTimes(const VolumeLoader& loader) {
    const auto loaderTimes = loader.times();
    if (!loaderTimes.empty()) {
        return std::vector<double>(loaderTimes.begin(), loaderTimes.end());
    }
    std::vector<double> result(loader.size());
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = static_cast<double>(i);
    }
    return result;
}

}  // namespace

TemporalVolume::TemporalVolume(std::unique_ptr<VolumeLoader> loader, size_t cacheSize)
    : loader_{std::move(loader)}
    , prototype_{loader_ ? loader_->prototype() : nullptr}
    , times_{loader_ ? resolveTimes(*loader_) : std::vector<double>{}}
    , cacheSize_{std::max<size_t>(2, cacheSize)} {

    if (!loader_) {
        throw Exception("TemporalVolume requires a non-null VolumeLoader");
    }
    if (!prototype_) {
        throw Exception("VolumeLoader::prototype() returned a null Volume");
    }
}

TemporalVolume::~TemporalVolume() {
    // Wait for all in-flight prefetches before destroying the loader, since the pool tasks
    // capture a raw pointer to it.
    std::unordered_map<size_t, std::future<std::shared_ptr<Volume>>> pending;
    {
        const std::scoped_lock lock{mutex_};
        pending = std::move(pending_);
        pending_.clear();
    }
    for (auto& [index, future] : pending) {
        if (future.valid()) {
            future.wait();
        }
    }
}

size_t TemporalVolume::size() const { return times_.size(); }

bool TemporalVolume::empty() const { return times_.empty(); }

std::span<const double> TemporalVolume::times() const { return times_; }

std::pair<double, double> TemporalVolume::timeRange() const {
    if (times_.empty()) {
        return {0.0, 0.0};
    }
    return {times_.front(), times_.back()};
}

const Volume& TemporalVolume::prototype() const { return *prototype_; }

size_t TemporalVolume::nearestIndex(double time) const {
    const size_t n = times_.size();
    if (n == 0) {
        return 0;
    }
    if (time <= times_.front()) {
        return 0;
    }
    if (time >= times_.back()) {
        return n - 1;
    }
    const auto upper = std::upper_bound(times_.begin(), times_.end(), time);
    const auto ib = static_cast<size_t>(std::distance(times_.begin(), upper));
    const size_t ia = ib - 1;
    return (time - times_[ia] <= times_[ib] - time) ? ia : ib;
}

std::shared_ptr<const Volume> TemporalVolume::get(size_t index) const {
    if (index >= times_.size()) {
        return nullptr;
    }

    std::unique_lock lock{mutex_};

    if (auto it = cache_.find(index); it != cache_.end()) {
        touch(index);
        return it->second;
    }

    if (auto it = pending_.find(index); it != pending_.end()) {
        // A prefetch is in flight, take over its future and wait for it without holding the lock.
        auto future = std::move(it->second);
        pending_.erase(it);
        lock.unlock();
        auto volume = future.get();
        lock.lock();
        return insert(index, std::move(volume));
    }

    // Not cached and not pending, load synchronously without holding the lock.
    lock.unlock();
    auto volume = loader_->load(index);
    lock.lock();
    return insert(index, std::move(volume));
}

std::shared_ptr<const Volume> TemporalVolume::get(double time) const {
    return get(nearestIndex(time));
}

TemporalVolume::Frame TemporalVolume::interpolate(double time) const {
    const size_t n = times_.size();
    if (n == 0) {
        return {nullptr, nullptr, 0.0};
    }
    if (n == 1 || time <= times_.front()) {
        auto volume = get(size_t{0});
        return {volume, volume, 0.0};
    }
    if (time >= times_.back()) {
        auto volume = get(n - 1);
        return {volume, volume, 0.0};
    }

    const auto upper = std::upper_bound(times_.begin(), times_.end(), time);
    const auto ib = static_cast<size_t>(std::distance(times_.begin(), upper));
    const size_t ia = ib - 1;
    const double ta = times_[ia];
    const double tb = times_[ib];
    const double factor = (tb > ta) ? (time - ta) / (tb - ta) : 0.0;
    return {get(ia), get(ib), factor};
}

void TemporalVolume::prefetch(size_t index) const {
    if (index >= times_.size()) {
        return;
    }

    const std::scoped_lock lock{mutex_};
    if (cache_.contains(index) || pending_.contains(index)) {
        return;
    }

    auto* app = util::getInviwoApplication();
    if (!app) {
        // No thread pool available, prefetching is a no-op. Frames will be loaded synchronously
        // on demand in get().
        return;
    }

    pending_.emplace(
        index, app->dispatchPool([loader = loader_.get(), index]() { return loader->load(index); }));
}

void TemporalVolume::prefetch(size_t first, size_t count) const {
    for (size_t i = 0; i < count; ++i) {
        prefetch(first + i);
    }
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
        return it->second;
    }

    std::shared_ptr<const Volume> stored = std::move(volume);
    cache_.emplace(index, stored);
    lruOrder_.push_front(index);
    evict();
    return stored;
}

void TemporalVolume::touch(size_t index) const {
    lruOrder_.remove(index);
    lruOrder_.push_front(index);
}

void TemporalVolume::evict() const {
    while (cache_.size() > cacheSize_ && !lruOrder_.empty()) {
        const size_t lru = lruOrder_.back();
        lruOrder_.pop_back();
        cache_.erase(lru);
    }
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

        const Volume& prototype = data.prototype();
        tb(H("Dimensions"), prototype.getDimensions());
        tb(H("Format"), prototype.getDataFormat()->getString());
    }
    return doc;
}

}  // namespace inviwo
