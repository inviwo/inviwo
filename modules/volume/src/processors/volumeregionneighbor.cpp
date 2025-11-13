/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <inviwo/volume/processors/volumeregionneighbor.h>

#include <inviwo/core/algorithm/buildarray.h>
#include <inviwo/core/util/volumeramutils.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/threadutil.h>

#include <vector>
#include <set>
#include <algorithm>
#include <ranges>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeRegionNeighbor::processorInfo_{
    "org.inviwo.VolumeRegionNeighbor",                            // Class identifier
    "Volume Region Neighbor",                                     // Display name
    "Volume Operation",                                           // Category
    CodeState::Stable,                                            // Code state
    Tags::CPU | Tag{"Volume"} | Tag{"Atlas"} | Tag{"DataFrame"},  // Tags
    R"(Given an atlas volume, constructs a list of all neighboring region index pairs)"_unindentHelp,
};

const ProcessorInfo& VolumeRegionNeighbor::getProcessorInfo() const { return processorInfo_; }

VolumeRegionNeighbor::VolumeRegionNeighbor()
    : PoolProcessor{}
    , inport_{"inputVolume", "The input volume"_help}
    , scalarField_{"scalarField",
                   "Used together with the cutoff to only consider neighbors "
                   "that connects at sufficiently high value"_help}
    , outport_{"seedPoints"}
    , useCutoff_{"useCutoff", "Use Cutoff", false}
    , cutoff_{"cutoff", "Cutoff", util::ordinalSymmetricVector(100.0)} {

    scalarField_.setOptional(true);

    addPorts(inport_, scalarField_, outport_);
    addProperties(useCutoff_, cutoff_);
}

namespace {

template <typename Ret = void, typename Functor, typename... Args>
constexpr auto wrappingDispatch(const Wrapping3D& wrapping, Functor&& func, Args&&... args) {
    using DispatchFunctor = Ret (*)(Functor && func, Args && ...);

    constexpr auto table = util::build_array<2>([](auto x) constexpr {
        using XT = decltype(x);
        return util::build_array<2>([](auto y) constexpr {
            using YT = decltype(y);
            return util::build_array<2>([](auto z) constexpr -> DispatchFunctor {
                using ZT = decltype(z);
                return [](Functor&& func, Args&&... args) {
                    constexpr auto X = static_cast<Wrapping>(XT::value);
                    constexpr auto Y = static_cast<Wrapping>(YT::value);
                    constexpr auto Z = static_cast<Wrapping>(ZT::value);
                    return std::forward<Functor>(func).template operator()<X, Y, Z>(
                        std::forward<Args>(args)...);
                };
            });
        });
    });

    const size3_t i{static_cast<std::size_t>(wrapping[0]), static_cast<std::size_t>(wrapping[1]),
                    static_cast<std::size_t>(wrapping[2])};

    if (glm::any(glm::greaterThanEqual(i, size3_t{2, 2, 2}))) {
        throw Exception("Mirror Wrapping is not supported");
    }

    return table[i.x][i.y][i.z](std::forward<Functor>(func), std::forward<Args>(args)...);
}

template <Wrapping W, typename T>
T wrap(T pos, size_t dimM1) {
    if constexpr (W == Wrapping::Clamp) {
        return std::clamp(pos, T{0}, static_cast<T>(dimM1));
    } else if constexpr (W == Wrapping::Repeat) {
        if (pos >= dimM1) return pos - dimM1;
        if (pos < 0) return pos + dimM1;
        return pos;
    } else {
        static_assert(util::alwaysFalse<T>(), "Mirror Wrapping is not supported");
    }
}

template <Wrapping X, Wrapping Y, Wrapping Z, typename T, glm::qualifier Q = glm::defaultp>
glm::vec<3, T, Q> wrap(glm::vec<3, T, Q> pos, size3_t dimsM1) {
    pos.x = wrap<X>(pos.x, dimsM1.x);
    pos.y = wrap<Y>(pos.y, dimsM1.y);
    pos.z = wrap<Z>(pos.z, dimsM1.z);
    return pos;
}

template <typename C>
void forEachVoxelParallelState(const size3_t dims, size_t jobs, C callback) {
    std::vector<std::future<void>> futures;
    for (size_t job = 0; job < jobs; ++job) {
        const size3_t start = size3_t(0, 0, job * dims.z / jobs);
        const size3_t stop = size3_t(dims.x, dims.y, std::min(dims.z, (job + 1) * dims.z / jobs));

        futures.push_back(util::dispatchPool([&callback, start, stop, job]() {
            size3_t pos{0};

            for (pos.z = start.z; pos.z < stop.z; ++pos.z) {
                for (pos.y = start.y; pos.y < stop.y; ++pos.y) {
                    for (pos.x = start.x; pos.x < stop.x; ++pos.x) {
                        callback(pos, job);
                    }
                }
            }
        }));
    }

    for (const auto& e : futures) {
        e.wait();
    }
}

constexpr auto neighbors =
    std::array{size3_t{0, 0, 1}, size3_t{0, 1, 0}, size3_t{0, 1, 1}, size3_t{1, 0, 0},
               size3_t{1, 0, 1}, size3_t{1, 1, 0}, size3_t{1, 1, 1}};

auto calc(const std::shared_ptr<const Volume>& volume) -> std::shared_ptr<DataFrame> {
    auto df = std::make_shared<DataFrame>();

    const size_t poolSize = util::getPoolSize();

    if (poolSize == 0) {
        throw Exception("Need a non-zero thread pool to run");
    }

    const auto dims = volume->getDimensions();
    const util::IndexMapper3D im{dims};

    volume->getRepresentation<VolumeRAM>()
        ->dispatch<void, dispatching::filter::UnsignedIntegerScalars>([&](auto vr) {
            using ValueType = util::PrecisionValueType<decltype(vr)>;

            const auto pairs = wrappingDispatch<std::vector<std::pair<ValueType, ValueType>>>(
                vr->getWrapping(), [&]<Wrapping X, Wrapping Y, Wrapping Z>() {
                    const auto dimsM1 = dims - size3_t{1, 1, 1};

                    const auto jobs = 4 * poolSize;
                    std::vector<std::set<std::pair<ValueType, ValueType>>> sets(jobs);

                    const auto data = vr->getView();
                    forEachVoxelParallelState(dims, jobs, [&](const size3_t& voxelPos, size_t job) {
                        const auto center = data[im(voxelPos)];

                        for (const auto& nn : neighbors) {
                            const auto val = data[im(wrap<X, Y, Z>(voxelPos + nn, dimsM1))];
                            if (center < val) {
                                sets[job].emplace(center, val);
                            } else if (center > val) {
                                sets[job].emplace(val, center);
                            }
                        }
                    });

                    std::vector<std::pair<ValueType, ValueType>> res;
                    std::vector<std::pair<ValueType, ValueType>> tmp;
                    for (const auto& set : sets) {
                        std::swap(tmp, res);
                        res.clear();
                        std::ranges::set_union(set, tmp, std::back_inserter(res));
                    }

                    return res;
                });

            df->addColumn("Pair First", pairs | std::views::transform([](const auto& p) {
                                            return p.first;
                                        }) | std::ranges::to<std::vector>());
            df->addColumn("Pair Second", pairs | std::views::transform([](const auto& p) {
                                             return p.second;
                                         }) | std::ranges::to<std::vector>());
        });

    df->updateIndexBuffer();
    return df;
}

auto calc(const std::shared_ptr<const Volume>& volume, const std::shared_ptr<const Volume>& scalars,
          double cutoff) -> std::shared_ptr<DataFrame> {
    auto df = std::make_shared<DataFrame>();

    const size_t poolSize = util::getPoolSize();
    const auto dims = volume->getDimensions();
    const util::IndexMapper3D im{dims};

    if (scalars->getDimensions() != dims) {
        throw Exception("Expected both volumes to have the same dimensions");
    }

    const auto* sf = scalars->getRepresentation<VolumeRAM>();

    volume->getRepresentation<VolumeRAM>()
        ->dispatch<void, dispatching::filter::UnsignedIntegerScalars>([&](auto vr) {
            using ValueType = util::PrecisionValueType<decltype(vr)>;

            const auto pairs = wrappingDispatch<std::vector<std::pair<ValueType, ValueType>>>(
                vr->getWrapping(), [&]<Wrapping X, Wrapping Y, Wrapping Z>() {
                    const auto dimsM1 = dims - size3_t{1, 1, 1};

                    const auto jobs = 4 * poolSize;
                    std::vector<std::set<std::pair<ValueType, ValueType>>> sets(jobs);

                    const auto data = vr->getView();
                    forEachVoxelParallelState(dims, jobs, [&](const size3_t& voxelPos, size_t job) {
                        if (sf->getAsNormalizedDouble(voxelPos) < cutoff) return;

                        const auto center = data[im(voxelPos)];

                        for (const auto& nn : neighbors) {
                            const auto npos = wrap<X, Y, Z>(voxelPos + nn, dimsM1);
                            if (sf->getAsNormalizedDouble(npos) < cutoff) continue;

                            const auto val = data[im(npos)];
                            if (center < val) {
                                sets[job].emplace(center, val);
                            } else if (center > val) {
                                sets[job].emplace(val, center);
                            }
                        }
                    });

                    std::vector<std::pair<ValueType, ValueType>> res;
                    std::vector<std::pair<ValueType, ValueType>> tmp;
                    for (const auto& set : sets) {
                        std::swap(tmp, res);
                        res.clear();
                        std::ranges::set_union(set, tmp, std::back_inserter(res));
                    }

                    return res;
                });

            df->addColumn("Pair First", pairs | std::views::transform([](const auto& p) {
                                            return p.first;
                                        }) | std::ranges::to<std::vector>());
            df->addColumn("Pair Second", pairs | std::views::transform([](const auto& p) {
                                             return p.second;
                                         }) | std::ranges::to<std::vector>());
        });

    df->updateIndexBuffer();
    return df;
}

}  // namespace

void VolumeRegionNeighbor::process() {
    if (useCutoff_) {
        if (!scalarField_.isReady()) {
            throw Exception("To use the cutoff a scalar field needs to be connected");
        }
        outport_.setData(nullptr);
        dispatchOne([vol = inport_.getData(), sf = scalarField_.getData(),
                     cutoff = cutoff_.get()]() { return calc(vol, sf, cutoff); },
                    [this](const std::shared_ptr<DataFrame>& result) {
                        outport_.setData(result);
                        newResults();
                    });
    } else {
        outport_.setData(nullptr);
        dispatchOne([vol = inport_.getData()]() { return calc(vol); },
                    [this](const std::shared_ptr<DataFrame>& result) {
                        outport_.setData(result);
                        newResults();
                    });
    }
}

}  // namespace inviwo
