# TemporalVolume — Design Document

This document describes the design for `TemporalVolume`, a data structure for
time-dependent volumetric data in Inviwo. It covers the motivation, the layered
architecture, the caching and prefetch strategy, the processor pattern, and the
rationale for key design decisions.

> **Audience:** developers adding time-varying volume support, writing
> `VolumeLoader` implementations, or authoring processors that consume
> `TemporalVolume` data.

---

## 1. Motivation

Inviwo already has [`VolumeSequence`](../../include/inviwo/core/datastructures/volume/volume.h)
(`DataSequence<Volume>`) for sequences of volumes. This is sufficient when all
frames fit comfortably in memory. For large-scale time-varying data (e.g. 4D
CT/MRI, CFD simulations, time-lapse microscopy) this is not feasible:

- A single 512³ `float` volume is ~512 MB. A 100-frame sequence is ~50 GB.
- Downstream processors (e.g. a raycaster) only ever need one or two frames at
  a time.
- Interactive playback benefits from having the *next* frame ready before it is
  requested — i.e. prefetching.
- Smooth interpolation between frames requires simultaneous access to two
  adjacent volumes.

`TemporalVolume` addresses all of these without requiring changes to existing
single-frame processors.

---

## 2. Relationship to Existing Types

```
                          DataSequence<Volume>
                               (VolumeSequence)
                          all frames in memory,
                          no lazy loading

                          TemporalVolume
                          metadata always available,
                          frames loaded on demand,
                          bounded LRU cache,
                          background prefetch
```

Both types can coexist. `VolumeSequence` remains the right choice for small
sequences. `TemporalVolume` is for data sets where only a sliding window of
frames fits in memory.

---

## 3. Architecture Overview

The design has three layers:

```
┌─────────────────────────────────────────────────────┐
│                   TemporalVolume                    │  ← port type / data object
│                                                     │
│  times[]   prototype   LRU cache   pending futures  │
│                             │            │          │
│                     ┌───────┴────────────┘          │
│                     ▼                               │
│               VolumeLoader  (abstract)              │  ← storage abstraction
│                     │                               │
└─────────────────────┼───────────────────────────────┘
                      │
         ┌────────────┼────────────┐
         ▼            ▼            ▼
  FileSequence    HDF5Temporal  Procedural
  Loader          Loader        Loader
```

### 3.1 `VolumeLoader` — Abstract Interface

`VolumeLoader` decouples the storage format from the time axis and cache
management. Implementations are responsible only for loading a single frame by
index.

```cpp
class VolumeLoader {
public:
    virtual ~VolumeLoader() = default;

    /// Load the volume at the given index. May be called from a background thread.
    virtual std::shared_ptr<Volume> load(size_t index) = 0;

    /// Total number of frames.
    virtual size_t size() const = 0;

    /// Physical time value for each frame (e.g. seconds, milliseconds).
    /// Size must equal size().
    virtual std::span<const double> times() const = 0;

    /// A prototype Volume with correct dimensions, format, basis, dataMap,
    /// and axes — but no voxel data. Used by downstream processors that need
    /// metadata without triggering a load.
    virtual std::shared_ptr<const Volume> prototype() const = 0;
};
```

Key properties:
- `load()` **must be thread-safe** — it may be called concurrently from the
  prefetch thread pool.
- `prototype()` must be **cheap** — it should not load voxel data.
- `times()` returns physical time values so that callers can request a frame by
  time (e.g. seconds) rather than index.

### 3.2 `TemporalVolume` — Data Object

`TemporalVolume` is the object that flows through Inviwo ports. It owns a
`VolumeLoader`, a bounded LRU cache of loaded volumes, and a map of in-flight
load futures.

```cpp
class TemporalVolume {
public:
    explicit TemporalVolume(std::unique_ptr<VolumeLoader> loader,
                            size_t cacheSize = 8);

    // ── Metadata (no I/O) ───────────────────────────────────────────────────

    size_t size() const;
    std::span<const double> times() const;
    const Volume& prototype() const;  // dims, format, basis — no voxel data

    // ── Synchronous access ──────────────────────────────────────────────────

    /// Get frame by index. Blocks if not cached. Returns nullptr if index OOB.
    std::shared_ptr<const Volume> get(size_t index) const;

    /// Get frame nearest to the given time value.
    std::shared_ptr<const Volume> get(double time) const;

    // ── Interpolated access ─────────────────────────────────────────────────

    struct Frame {
        std::shared_ptr<const Volume> a;  // frame at or before `time`
        std::shared_ptr<const Volume> b;  // frame at or after  `time`
        double t;                         // blend factor in [0, 1]; 0 = pure a
    };

    /// Return the two frames bracketing `time` and a blend factor.
    /// Both frames are synchronously loaded if not cached.
    Frame interpolate(double time) const;

    // ── Prefetch (non-blocking) ─────────────────────────────────────────────

    /// Schedule background load of frame at `index`.
    void prefetch(size_t index) const;

    /// Schedule background loads for `count` frames starting at `first`.
    void prefetch(size_t first, size_t count) const;

    // ── Cache control ───────────────────────────────────────────────────────

    void setCacheSize(size_t n);
    size_t cacheSize() const;
    void clearCache();

private:
    std::unique_ptr<VolumeLoader> loader_;
    size_t cacheSize_;
    mutable std::mutex mutex_;
    mutable std::list<size_t> lruOrder_;
    mutable std::unordered_map<size_t, std::shared_ptr<Volume>> cache_;
    mutable std::unordered_map<size_t, std::future<std::shared_ptr<Volume>>> pending_;
};
```

#### Cache and Prefetch Logic

`get(index)` follows this decision tree:

```
get(index)
    │
    ├─ in cache? ──────────────────────────────► return cached, bump LRU
    │
    ├─ pending future? ────────────────────────► future.get(), insert cache, return
    │
    └─ neither ────────────────────────────────► loader_->load(index) (sync),
                                                  insert cache, evict LRU if full,
                                                  return
```

`prefetch(index)` follows:

```
prefetch(index)
    │
    ├─ in cache?   ──► no-op
    ├─ in pending? ──► no-op
    └─ submit dispatchPool([loader_, index]{ return loader_->load(index); })
       store future in pending_
```

When a pending future is promoted to cache (by `get()`), it is removed from
`pending_` and inserted into `cache_`. LRU eviction applies only to `cache_`;
pending futures are never evicted (they represent committed I/O work).

---

## 4. `VolumeLoader` Implementations

### 4.1 `FileSequenceLoader`

The most common case: one volume file per frame (e.g. a set of `.vti`, `.dat`,
or `.nii` files discovered by glob or explicit list).

```cpp
class FileSequenceLoader : public VolumeLoader {
public:
    /// paths.size() == times.size() (or times may be empty → 0,1,2,…)
    FileSequenceLoader(std::vector<std::filesystem::path> paths,
                       std::vector<double> times,
                       std::shared_ptr<DataReaderFactory> factory);
    // ...
};
```

- Uses Inviwo's `DataReaderFactory` so that any registered volume reader works.
- `prototype()` loads only header/metadata from the first file.

### 4.2 `HDF5TemporalLoader`

For a single HDF5/NetCDF file with a time dimension stored as a 4D dataset or
as a group of 3D datasets.

```cpp
class HDF5TemporalLoader : public VolumeLoader {
public:
    HDF5TemporalLoader(std::filesystem::path file,
                       std::string datasetPath,
                       std::string timePath = "");
    // ...
};
```

- Opens the file once; `load(i)` reads only the slice for frame `i` using HDF5
  hyperslab selection — no full file re-read.
- `times()` reads the time coordinate dataset if `timePath` is provided,
  otherwise falls back to frame index.

### 4.3 `ProceduralLoader`

For simulation or test purposes: computes each frame on the fly.

```cpp
class ProceduralLoader : public VolumeLoader {
public:
    using Generator = std::function<std::shared_ptr<Volume>(size_t index, double time)>;
    ProceduralLoader(size_t count, std::vector<double> times,
                     std::shared_ptr<const Volume> prototype,
                     Generator generator);
    // ...
};
```

---

## 5. Processor Pattern

The standard pipeline for time-varying volume data looks like:

```
┌──────────────────────────┐
│  TemporalVolumeSource    │  Loads a FileSequenceLoader / HDF5TemporalLoader.
│                          │  Outport: TemporalVolume
│  Properties:             │
│    • path / glob         │
│    • cache size          │
└────────────┬─────────────┘
             │ TemporalVolume
             ▼
┌──────────────────────────┐
│  TemporalVolumePlayer    │  Resolves current time → single Volume.
│                          │  Inport:  TemporalVolume
│  Properties:             │  Outport: Volume
│    • time (float)        │
│    • prefetch ahead (N)  │
│    • interpolation mode  │
└────────────┬─────────────┘
             │ Volume
             ▼
┌──────────────────────────┐
│  (any existing Volume    │  Unchanged — raycasters, iso-surface extractors,
│   processor)             │  slice viewers, etc. require no modification.
└──────────────────────────┘
```

### 5.1 `TemporalVolumePlayer` — process() sketch

```cpp
void TemporalVolumePlayer::process() {
    const auto& tv = inport_.getData();
    const double t  = timeProp_.get();

    if (interpolate_.get()) {
        auto [a, b, blend] = tv->interpolate(t);
        outport_.setData(blend(a, b, blend));  // voxel-level lerp
    } else {
        outport_.setData(tv->get(t));
    }

    // Prefetch next N frames
    const size_t cur = tv->nearestIndex(t);
    tv->prefetch(cur + 1, prefetchCount_.get());
}
```

`prefetch()` is a non-blocking fire-and-forget call; it uses
`InviwoApplication::dispatchPool()` internally so it participates in Inviwo's
standard thread pool management.

### 5.2 Interpolation

For GPU-based interpolation (e.g. in a raycaster that holds both frames as
3D textures), `TemporalVolumePlayer` can expose a second `Volume` outport
(`volumeB`) and a `float` blend factor outport, so that the renderer can
perform interpolation in the shader rather than on the CPU.

```
TemporalVolumePlayer
  ├── outport: Volume A  (frame at or before time)
  ├── outport: Volume B  (frame at or after  time)
  └── outport: float t   (blend factor 0→1)
```

This avoids a full CPU-side voxel copy for large volumes.

---

## 6. Port and DataTraits Registration

`TemporalVolume` registers with Inviwo's data traits so that it gets a port
color, display name, and info panel like any other data type:

```cpp
template <>
struct DataTraits<TemporalVolume> {
    static constexpr std::string_view classIdentifier() {
        return "org.inviwo.TemporalVolume";
    }
    static constexpr std::string_view dataName() { return "TemporalVolume"; }
    static constexpr uvec3 colorCode() {
        // Slightly lighter than Volume's {188, 101, 101}
        return {210, 130, 130};
    }
    static Document info(const TemporalVolume& tv);
};

// Port aliases
using TemporalVolumeInport  = DataInport<TemporalVolume>;
using TemporalVolumeOutport = DataOutport<TemporalVolume>;
```

---

## 7. Threading Model

```
Main thread                    Thread pool (dispatchPool)
──────────────────────────     ──────────────────────────────
process() calls
  tv->prefetch(i)  ──────────► loader_->load(i)  [runs here]
                                 └─► future stored in pending_

... later ...
  tv->get(i)
    └─ future.get() ◄──────── future resolved, shared_ptr<Volume> returned
    └─ insert into cache_
    └─ evict LRU if needed
    └─ return to process()
```

- `VolumeLoader::load()` must be **re-entrant and thread-safe**.
- The `mutex_` in `TemporalVolume` protects `cache_`, `lruOrder_`, and
  `pending_`. It is held only briefly (map lookups, insertions) — not during
  the actual I/O, which happens inside the `std::future`.
- `get()` must not be called from a background thread that already holds
  `mutex_` (no recursive locking).

---

## 8. Memory Budget

Cache size is specified as a frame count. A helper can also accept a byte
budget:

```cpp
// Construct with explicit frame count
TemporalVolume tv(std::move(loader), /*cacheSize=*/8);

// Or derive frame count from a byte budget
size_t framesForBudget(const VolumeLoader& loader, size_t bytes) {
    const size_t bytesPerFrame = loader.prototype()->getNumberOfBytes();
    return std::max<size_t>(2, bytes / bytesPerFrame);
}
```

A minimum of **2 frames** must always be cached so that interpolation (`Frame`
struct) never needs to evict `a` while loading `b`.

---

## 9. Design Decisions and Alternatives

### Why not extend `VolumeSequence` with lazy loading?

`DataSequence<Volume>` stores `shared_ptr<Volume>` directly. Lazy loading would
require wrapping each element in a proxy or adding an optional loader per slot,
complicating the existing API and the `DataTraits` registration. A separate
class keeps concerns clearly separated and avoids breaking existing code.

### Why not use the VTK/ParaView temporal pipeline model?

ParaView propagates time as a *pipeline request key*: processors receive a
`TIME_REQUEST` and re-execute with new data. This requires all processors in the
pipeline to be time-aware. Inviwo's design goal is that **existing single-frame
processors require no modification**. The player-processor pattern achieves this
by resolving the time axis at a single point and emitting a plain `Volume`
downstream.

### Why store physical time values (doubles) rather than just indices?

Many data sets have non-uniform time sampling (e.g. adaptive time stepping in
simulations, variable acquisition rate in 4D CT). Using physical time values
lets interpolation and `get(double time)` work correctly in all cases. Index
access (`get(size_t)`) remains available for cases where indices are more
natural.

### Why a `prototype()` volume?

Downstream processors (e.g. those setting up transfer functions, computing
histogram ranges, or allocating GPU textures) need to know dimensions, data
format, and the `DataMapper` before the first frame is loaded. Without
`prototype()` they would have to trigger a full frame load on pipeline
initialization, negating the benefits of lazy loading.

---

## 10. Summary of Types

| Type | Responsibility |
|---|---|
| `VolumeLoader` | Abstract interface: load frame `i`, expose `times[]`, expose `prototype` |
| `FileSequenceLoader` | One file per frame, uses `DataReaderFactory` |
| `HDF5TemporalLoader` | Single HDF5/NetCDF file, hyperslab reads |
| `ProceduralLoader` | On-the-fly generation via a callable |
| `TemporalVolume` | LRU cache + prefetch + port type |
| `TemporalVolumeSource` | Processor: creates `VolumeLoader`, outputs `TemporalVolume` |
| `TemporalVolumePlayer` | Processor: resolves time → `Volume`, triggers prefetch |
| `TemporalVolumeInport` | `DataInport<TemporalVolume>` |
| `TemporalVolumeOutport` | `DataOutport<TemporalVolume>` |

---

## 11. Suggested File Layout

```
include/inviwo/core/datastructures/volume/
    temporalvolume.h       ← TemporalVolume + VolumeLoader

src/core/datastructures/volume/
    temporalvolume.cpp

modules/base/include/modules/base/io/
    filesequenceloader.h
    hdf5temporalloader.h   (or in hdf5 module)

modules/base/processors/
    temporalvolumesource.h / .cpp
    temporalvolumeplayer.h / .cpp
```

`TemporalVolume` and `VolumeLoader` live in `inviwo-core` (no module
dependency). The concrete loaders and processors live in modules so that
module-specific dependencies (HDF5, specific file formats) do not affect the
core.
