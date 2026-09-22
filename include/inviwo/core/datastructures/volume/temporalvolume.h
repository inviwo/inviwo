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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/algorithm/histogram1d.h>
#include <inviwo/core/datastructures/datatraits.h>
#include <inviwo/core/datastructures/tfdata.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeconfig.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/util/stdextensions.h>

#include <chrono>
#include <cstddef>
#include <functional>
#include <future>
#include <list>
#include <memory>
#include <mutex>
#include <span>
#include <stop_token>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace inviwo {

/// Physical time value of a TemporalVolume frame, seconds stored as a double.
using Seconds = std::chrono::duration<double>;

/**
 * @ingroup datastructures
 *
 * @brief Abstract interface for lazily loading the individual time steps (frames) of a
 * TemporalVolume.
 *
 * A TemporalVolumeLoader decouples the storage format from the time axis and cache management of
 * TemporalVolume. Implementations are responsible only for loading a single frame by index and
 * for providing the time axis and a metadata-only @c prototype VolumeConfig.
 *
 * @note Implementations of @c load() must be **thread-safe** since it may be called concurrently
 * from a background thread pool. @c prototype() must be cheap and must not load any voxel data.
 *
 * @see TemporalVolume
 */
class IVW_CORE_API TemporalVolumeLoader {
public:
    TemporalVolumeLoader() = default;
    TemporalVolumeLoader(const TemporalVolumeLoader&) = default;
    TemporalVolumeLoader(TemporalVolumeLoader&&) = default;
    TemporalVolumeLoader& operator=(const TemporalVolumeLoader&) = default;
    TemporalVolumeLoader& operator=(TemporalVolumeLoader&&) = default;
    virtual ~TemporalVolumeLoader() = default;

    /**
     * Load the volume at the given @p index. May be called from a background thread, hence
     * implementations must be re-entrant and thread-safe.
     *
     * @param reuse an optional volume whose storage may be reused to avoid reallocation. It may be
     *              null and must be ignored if its format or dimensions do not match the frame.
     * @param stop  cancellation token. Implementations should poll @c stop.stop_requested() at
     *              natural checkpoints and return nullptr as soon as a stop is requested. A
     *              partially written @p reuse volume is discarded by the caller.
     * @return the loaded volume, or nullptr if the load was cancelled.
     */
    virtual std::shared_ptr<Volume> load(size_t index, std::shared_ptr<Volume> reuse,
                                         std::stop_token stop) const = 0;

    /// Total number of frames.
    virtual size_t size() const = 0;

    /**
     * Physical time value for frame @p index. The values are expected to be sorted in ascending
     * order.
     */
    virtual Seconds time(size_t index) const = 0;

    /**
     * A prototype VolumeConfig describing dimensions, format, basis, dataMap, and axes — but no
     * voxel data. Used by downstream consumers that need metadata without triggering a load.
     */
    virtual VolumeConfig prototype() const = 0;
};

/**
 * @ingroup datastructures
 *
 * @brief A VolumeLoader that computes each frame on the fly via a callable.
 *
 * Useful for procedural data, simulations, or testing.
 */
class IVW_CORE_API ProceduralLoader : public TemporalVolumeLoader {
public:
    /// Signature of the generator callable, given a frame @c index, its @c time value, an
    /// optional @c reuse volume whose storage may be reused (may be null), and a @c stop token.
    /// @see TemporalVolumeLoader::load
    using Generator = std::function<std::shared_ptr<Volume>(
        size_t index, Seconds time, std::shared_ptr<Volume> reuse, std::stop_token stop)>;

    /**
     * @param count      number of frames
     * @param times      physical time values, either empty or of size @p count
     * @param prototype  metadata-only prototype config
     * @param generator  callable producing the volume for a given index/time
     */
    ProceduralLoader(size_t count, std::vector<Seconds> times, VolumeConfig prototype,
                     Generator generator);

    virtual std::shared_ptr<Volume> load(size_t index, std::shared_ptr<Volume> reuse,
                                         std::stop_token stop) const override;
    virtual size_t size() const override;
    virtual Seconds time(size_t index) const override;
    virtual VolumeConfig prototype() const override;

private:
    size_t count_;
    std::vector<Seconds> times_;
    VolumeConfig prototype_;
    Generator generator_;
};

/**
 * @ingroup datastructures
 *
 * @brief Data structure for time-dependent volumetric data.
 *
 * TemporalVolume provides access to a sequence of time steps (frames) where only a bounded
 * sliding window of frames is kept in memory at any time. It owns a VolumeLoader, a bounded LRU
 * cache of decoded volumes, and a set of in-flight (prefetch) load futures.
 *
 * Metadata (number of frames, time values, and a @c prototype volume describing dimensions,
 * format, and the DataMapper) is always available without triggering any I/O. Individual frames
 * are loaded on demand via @c get() or @c interpolate(), and can be loaded ahead of time in the
 * background via @c prefetch().
 *
 * TemporalVolume is the object that flows through Inviwo ports, see #TemporalVolumeInport and
 * #TemporalVolumeOutport. It is non-copyable and shared via @c std::shared_ptr.
 *
 * @see VolumeLoader
 */
class IVW_CORE_API TemporalVolume {
public:
    /**
     * @param loader     the loader providing frames, times, and prototype, must not be null
     * @param cacheSize  maximum number of decoded frames to keep in memory (clamped to >= 2)
     */
    explicit TemporalVolume(std::unique_ptr<TemporalVolumeLoader> loader, size_t cacheSize = 8);
    TemporalVolume(const TemporalVolume&) = delete;
    TemporalVolume(TemporalVolume&&) = delete;
    TemporalVolume& operator=(const TemporalVolume&) = delete;
    TemporalVolume& operator=(TemporalVolume&&) = delete;
    ~TemporalVolume();

    /// Number of frames.
    size_t size() const;
    /// Whether there are no frames.
    bool empty() const;
    /// The physical time value of each frame (size equals @c size()).
    auto times() const {
        return std::views::iota(0uz, loader_->size()) |
               std::views::transform([this](size_t i) { return loader_->time(i); });
    }
    /// The time value of the first and last frame, or {0, 0} if empty.
    std::pair<Seconds, Seconds> timeRange() const;

    /// A prototype VolumeConfig describing dimensions, format, basis, and the DataMapper.
    const VolumeConfig& prototype() const;
    const DataMapper& dataMap() const;

    /// Index of the frame whose time is closest to @p time.
    size_t nearestIndex(Seconds time) const;

    /// Frame by index. Blocks if not cached. Returns nullptr if @p index is out of bounds, or if
    /// @p stop is requested while waiting for or performing the load.
    std::shared_ptr<const Volume> get(size_t index, std::stop_token stop = {}) const;
    /// Frame nearest to the given @p time value. Blocks if not cached.
    std::shared_ptr<const Volume> get(Seconds time, std::stop_token stop = {}) const;

    /// Two frames bracketing a requested time together with a blend factor.
    struct Frame {
        std::shared_ptr<const Volume> a;  //!< frame at or before the requested time
        std::shared_ptr<const Volume> b;  //!< frame at or after the requested time
        double t;                         //!< blend factor in [0, 1]; 0 == pure a, 1 == pure b
    };

    /**
     * Return the two frames bracketing @p time together with a blend factor in [0, 1]. Both frames
     * are synchronously loaded if not cached. If @p time is outside the time range, the nearest
     * frame is returned in both @c a and @c b with a blend factor of 0.
     */
    Frame interpolate(Seconds time, std::stop_token stop = {}) const;

    /// Schedule a background load (non-blocking) of the frame at @p index. No-op if already cached
    /// or pending. The @p callback is not invoked if the load is cancelled.
    void prefetch(size_t index,
                  std::function<void(std::shared_ptr<Volume>)> callback = nullptr) const;

    /// Set the maximum number of decoded frames to keep in memory (clamped to >= 2).
    void setCacheSize(size_t n);
    /// The maximum number of decoded frames kept in memory.
    size_t cacheSize() const;
    /// Number of frames currently held in the cache.
    size_t numCached() const;
    /// Drop all cached frames and request cancellation of all in-flight prefetches. Does not block
    /// waiting for them to finish.
    void clearCache();

private:
    template <typename T>
    friend struct TFDataTraits;

    /// An in-flight background load together with the source used to cancel it. The future is
    /// shared so that waiters can copy it and drop @c mutex_ while blocking.
    struct Pending {
        std::shared_future<std::shared_ptr<Volume>> future;
        std::stop_source source;
    };
    struct Valid {
        std::shared_ptr<Volume> volume;
    };
    struct Reuse {
        std::shared_ptr<Volume> volume;
    };

    struct Item : std::variant<Valid, Pending, Reuse> {
        using Base = std::variant<Valid, Pending, Reuse>;
        using Base::Base;
        using Base::operator=;

        Valid* valid() { return index() == 0 ? &std::get<0>(*this) : nullptr; }
        Pending* pending() { return index() == 1 ? &std::get<1>(*this) : nullptr; }
        Reuse* reuse() { return index() == 2 ? &std::get<2>(*this) : nullptr; }

        auto visit(std::invocable<Valid&> auto&& volumeCallback,
                   std::invocable<Pending&> auto&& pendingCallback,
                   std::invocable<Reuse&> auto&& reuseCallback) {
            return std::visit(
                util::overloaded{std::forward<decltype(volumeCallback)>(volumeCallback),
                                 std::forward<decltype(pendingCallback)>(pendingCallback),
                                 std::forward<decltype(reuseCallback)>(reuseCallback)},
                *this);
        }
    };

    /// Find the cache @c Item for @p index or @c nullptr (mutex must be held).
    Item* find(size_t index) const;

    std::shared_ptr<Volume> load(std::unique_lock<std::mutex>& lock, size_t index,
                                 std::shared_ptr<Volume> reuse, std::stop_token stop) const;

    /// Move @p index to the front of the LRU order (mutex must be held).
    void touch(size_t index) const;
    /// Evict least-recently-used entries until the cache fits (mutex must be held).
    void evict() const;
    /// Take a reusable volume from the reuse pool, or @c nullptr if the pool is empty (mutex must
    /// be held).
    std::shared_ptr<Volume> takeReuse() const;

    std::shared_ptr<const TemporalVolumeLoader> loader_;
    VolumeConfig prototype_;
    DataMapper dataMap_;
    size_t cacheSize_;

    mutable std::mutex mutex_;
    mutable std::vector<size_t> lruOrder_;  //!< back == most recently used
    mutable std::unordered_map<size_t, Item> cache_;

    HistogramCache histograms_;
};

template <>
struct DataTraits<TemporalVolume> {
    static constexpr std::string_view classIdentifier() { return "org.inviwo.TemporalVolume"; }
    static constexpr std::string_view dataName() { return "TemporalVolume"; }
    static constexpr uvec3 colorCode() { return uvec3{210, 130, 130}; }
    IVW_CORE_API static Document info(const TemporalVolume& data);
};

template <>
struct TFDataTraits<TemporalVolume> {
    static const DataMapper* getDataMap(const TemporalVolume& data) { return &data.dataMap(); }
    static HistogramCache::Result calculateHistograms(
        const TemporalVolume& data,
        const std::function<void(const std::vector<Histogram1D>&)>& whenDone) {
        if (data.empty()) {
            return {.progress = HistogramCache::Progress::NoData};
        }

        auto first = data.get(0);
        auto last = data.get(data.size() - 1);

        auto calc = [first, last]() {
            auto* firstRAM = first->getRepresentation<VolumeRAM>();
            auto* lastRAM = last->getRepresentation<VolumeRAM>();

            std::vector<Histogram1D> histograms;
            histograms.append_range(firstRAM->dispatch<std::vector<Histogram1D>>(
                [&]<typename T>(const VolumeRAMPrecision<T>* rp) {
                    return util::calculateHistograms(rp->getView(), first->dataMap, 2048);
                }));
            histograms.append_range(lastRAM->dispatch<std::vector<Histogram1D>>(
                [&]<typename T>(const VolumeRAMPrecision<T>* rp) {
                    return util::calculateHistograms(rp->getView(), last->dataMap, 2048);
                }));
            return histograms;
        };

        return data.histograms_.calculateHistograms(calc, whenDone);
    }
};

using TemporalVolumeInport = DataInport<TemporalVolume>;
using TemporalVolumeOutport = DataOutport<TemporalVolume>;

}  // namespace inviwo
