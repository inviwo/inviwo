/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <inviwo/volume/processors/volumeregionstatistics.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/volumeramutils.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/datastructures/unitsystem.h>

#include <numeric>
#include <algorithm>

#include <fmt/format.h>
#include <tcb/span.hpp>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeRegionStatistics::processorInfo_{
    "org.inviwo.VolumeRegionStatistics",  // Class identifier
    "Volume Region Statistics",           // Display name
    "Undefined",                          // Category
    CodeState::Experimental,              // Code state
    Tags::None,                           // Tags
};
const ProcessorInfo VolumeRegionStatistics::getProcessorInfo() const { return processorInfo_; }

VolumeRegionStatistics::VolumeRegionStatistics()
    : PoolProcessor(), volume_("volume"), atlas_{"atlas"}, dataFrame_{"statistics"} {

    addPorts(volume_, atlas_, dataFrame_);
}

template <typename T>
auto addColumns(DataFrame& df, std::string_view name, size_t size, Unit unit,
                std::optional<dvec2> range) {
    auto* data = &df.addColumn<T>(name, size, unit, range)
                      ->getTypedBuffer()
                      ->getEditableRAMRepresentation()
                      ->getDataContainer();
    return data;
}

template <typename T, size_t extent>
auto addColumns(DataFrame& df, std::string_view name, size_t size,
                util::span<const Unit, extent> units,
                util::span<const std::optional<dvec2>, extent> ranges,
                util::span<const std::string_view, extent> labels) {
    return util::make_array<extent>([&](auto index) {
        const auto fullName = fmt::format("{} {}", name, labels[index]);
        auto* data = &df.addColumn<T>(fullName, size, units[index], ranges[index])
                          ->getTypedBuffer()
                          ->getEditableRAMRepresentation()
                          ->getDataContainer();
        return data;
    });
}

template <typename T, size_t extent, size_t comps>
auto addColumns(DataFrame& df, std::string_view name, size_t size,
                util::span<const Unit, comps> units,
                util::span<const std::optional<dvec2>, comps> ranges,
                util::span<const std::string_view, extent> majorLabels,
                util::span<const std::string_view, comps> minorLabels) {
    return util::make_array<extent>([&](auto index) {
        return util::make_array<comps>([&](auto comp) {
            const auto fullName =
                fmt::format("{} {} {}", name, majorLabels[index], minorLabels[comp]);
            auto* data = &df.addColumn<T>(fullName, size, units[comp], ranges[comp])
                              ->getTypedBuffer()
                              ->getEditableRAMRepresentation()
                              ->getDataContainer();
            return data;
        });
    });
}

/**
 * Accumulators to calculate "center of mass" for periodic and non periodic systems
 * See https://en.wikipedia.org/wiki/Center_of_mass (Systems with periodic boundary conditions)
 */
template <Wrapping wrapX, Wrapping wrapY, Wrapping wrapZ>
class Accumulator {
public:
    Accumulator(dvec3 dim) : dim{dim} {}

    void add(const dvec3& pos, double weight) {
        addComp<0, wrapX>(pos[0], weight);
        addComp<1, wrapY>(pos[1], weight);
        addComp<2, wrapZ>(pos[2], weight);
    }
    dvec3 get(double totalWeight) const {
        return dvec3(getComp<0, wrapX>(totalWeight), getComp<1, wrapY>(totalWeight),
                     getComp<2, wrapZ>(totalWeight));
    }

private:
    template <size_t N, Wrapping wrap>
    void addComp(double pos, double weight) {
        auto& acc = std::get<N>(vec);
        if constexpr (wrap == Wrapping::Repeat) {
            const auto theta = pos / dim[N] * 2.0 * M_PI;
            acc.first += weight * std::cos(theta);
            acc.second += weight * std::sin(theta);
        } else {
            acc += weight * pos;
        }
    }

    template <size_t N, Wrapping wrap>
    double getComp(double totalWeight) const {
        auto& acc = std::get<N>(vec);
        if constexpr (wrap == Wrapping::Repeat) {
            const auto theta = std::atan2(-acc.second, -acc.first) + M_PI;
            return dim[N] * theta / (2.0 * M_PI);
        } else {
            return acc / totalWeight;
        }
    }
    template <Wrapping wrapping>
    using Acc = std::conditional_t<wrapping == Wrapping::Repeat, std::pair<double, double>, double>;
    std::tuple<Acc<wrapX>, Acc<wrapY>, Acc<wrapZ>> vec{};

    dvec3 dim{1.0};
};

template <typename T, Wrapping wrapX, Wrapping wrapY, Wrapping wrapZ>
class Stats {
public:
    Stats(dvec3 dim) : center{dim}, centerOfMass{dim} {}
    void add(const dvec3& r, T val) {
        ++volume;
        center.add(r, 1.0);
        centerOfMass.add(r, val);
        mass += val;
        min = glm::min(min, val);
        max = glm::max(max, val);
    }

    double getVolume() const { return volume; }
    double getMass() const { return mass; }
    double getMean() const { return mass / volume; }
    T getMin() const { return min; }
    T getMax() const { return max; }
    dvec3 getCenter() const { return center.get(volume); }
    dvec3 getCenterOfMass() const { return centerOfMass.get(mass); }

private:
    double volume{};
    double mass{};
    T min{std::numeric_limits<T>::max()};
    T max{std::numeric_limits<T>::lowest()};
    Accumulator<wrapX, wrapY, wrapZ> center;
    Accumulator<wrapX, wrapY, wrapZ> centerOfMass;
};

template <typename Index, typename Functor, Index... Is>
constexpr auto build_array_impl(Functor&& func, std::integer_sequence<Index, Is...>) noexcept {
    return std::array{func(std::integral_constant<Index, Is>{})...};
}

template <std::size_t N, typename Index = std::size_t, typename Functor>
constexpr auto build_array(Functor&& func) noexcept {
    return build_array_impl<Index>(std::forward<Functor>(func),
                                   std::make_integer_sequence<Index, N>());
}

template <typename Ret = void, typename Functor, typename... Args>
constexpr auto wrappingDispatch(Functor&& func, const Wrapping3D& wrapping, Args&&... args) {
    using DispatchFunctor = Ret (*)(Functor && func, Args && ...);

    constexpr auto table = build_array<3>([](auto x) constexpr {
        using XT = decltype(x);
        return build_array<3>([](auto y) constexpr {
            using YT = decltype(y);
            return build_array<3>([](auto z) constexpr->DispatchFunctor {
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

    return table[static_cast<std::size_t>(wrapping[0])][static_cast<std::size_t>(wrapping[1])]
                [static_cast<std::size_t>(wrapping[2])](std::forward<Functor>(func),
                                                        std::forward<Args>(args)...);
}

double voxelVolume(const dmat4& transform) {
    const auto a = dvec3{transform * dvec4{dvec3(1.0, 0.0, 0.0), 0.0}};
    const auto b = dvec3{transform * dvec4{dvec3(0.0, 1.0, 0.0), 0.0}};
    const auto c = dvec3{transform * dvec4{dvec3(0.0, 0.0, 1.0), 0.0}};
    return glm::abs(glm::dot(a, glm::cross(b, c)));
}

struct StatsFunctor {
    template <Wrapping wrapX, Wrapping wrapY, Wrapping wrapZ>
    std::shared_ptr<DataFrame> operator()(const Volume& volume, const Volume& atlas) const {
        const auto nRegions = static_cast<size_t>(atlas.dataMap_.dataRange.y);

        auto df = std::make_shared<DataFrame>(nRegions);

        volume.getRepresentation<VolumeRAM>()->dispatch<void, dispatching::filter::All>(
            [&](auto vr) {
                using PrecisionType = util::PrecisionValueType<decltype(vr)>;
                using T = util::value_type_t<PrecisionType>;
                constexpr auto channels = util::extent_v<PrecisionType>;

                const auto src = vr->getDataTyped();

                const util::IndexMapper2D regionMapper(size2_t{nRegions, channels});

                const auto dim = static_cast<dvec3>(volume.getDimensions());
                const auto index2world = volume.getCoordinateTransformer().getMatrix(
                    CoordinateSpace::Index, CoordinateSpace::World);
                const auto volumeScale = voxelVolume(index2world);
                const auto& map = volume.dataMap_;
                const auto& axes = volume.axes;
                
                const auto index2data = volume.getCoordinateTransformer().getMatrix(
                    CoordinateSpace::Index, CoordinateSpace::Data);

                using TStats = Stats<T, wrapX, wrapY, wrapZ>;
                std::vector<TStats> stats(nRegions * channels, TStats{dvec3{1.0}});

                atlas.getRepresentation<VolumeRAM>()
                    ->dispatch<void, dispatching::filter::UnsignedIntegerScalars>(
                        [&](auto atlasrep) {
                            const auto regionData = atlasrep->getDataTyped();
                            const util::IndexMapper3D indexMapper(dim);
                            util::forEachVoxel(*vr, [&](const size3_t& pos) {
                                const auto p = indexMapper(pos);
                                for (size_t c = 0; c < channels; ++c) {
                                    const auto region = regionMapper(regionData[p] - 1, c);
                                    const auto value = util::glmcomp(src[p], c);
                                    
                                    const auto dpos = dvec3{index2data * dvec4{pos, 1.0}};
                                    
                                    stats[region].add(dpos, value);
                                }
                            });
                        });

                const std::array<std::string_view, 3> axesNames = {axes[0].name, axes[1].name,
                                                                   axes[2].name};
                const std::array<Unit, 3> axesUnits = {axes[0].unit, axes[1].unit, axes[2].unit};
                static constexpr std::array<const std::string_view, 4> indexLabels = {"0", "1", "2",
                                                                                      "3"};

                const auto channelLabels =
                    util::span<const std::string_view, channels>(indexLabels.data(), channels);

                const auto valueUnits =
                    util::make_array<channels>([&](auto) { return map.valueAxis.unit; });

                const auto defaultRanges =
                    util::make_array<channels>([&](auto) -> std::optional<dvec2> { return {}; });

                const auto volumeUnit = axes[0].unit * axes[1].unit * axes[2].unit;
                const auto sumUnits = util::make_array<channels>(
                    [&](auto) { return volumeUnit * map.valueAxis.unit; });

                const auto data2world = volume.getCoordinateTransformer().getMatrix(CoordinateSpace::Data,
                                                                             CoordinateSpace::World);

                const auto posMin = dvec3{data2world * dvec4{0.0, 0.0, 0.0, 1.0}};
                const auto posMax = dvec3{data2world * dvec4{1.0, 1.0, 1.0, 1.0}};

                std::array<std::optional<dvec2>, 3> sizeRange = {{dvec2{posMin[0], posMax[0]},
                                                                  dvec2{posMin[1], posMax[1]},
                                                                  dvec2{posMin[2], posMax[2]}}};

                auto regionVolumes = addColumns<double>(*df, "Volume", nRegions, volumeUnit, {});
                auto regionSums = addColumns<double, channels>(*df, "Sum", nRegions, sumUnits,
                                                               defaultRanges, channelLabels);
                auto regionMean = addColumns<double, channels>(*df, "Mean", nRegions, valueUnits,
                                                               defaultRanges, channelLabels);
                auto regionMin = addColumns<double, channels>(*df, "Min", nRegions, valueUnits,
                                                              defaultRanges, channelLabels);
                auto regionMax = addColumns<double, channels>(*df, "Max", nRegions, valueUnits,
                                                              defaultRanges, channelLabels);
                auto regionCenter =
                    addColumns<double, 3>(*df, "Center", nRegions, axesUnits, sizeRange, axesNames);
                auto regionCoM = addColumns<double, channels, 3>(
                    *df, "CoM", nRegions, axesUnits, sizeRange, channelLabels, axesNames);

                for (auto&& [i, stat] : util::enumerate(stats)) {
                    const auto [region, c] = regionMapper(i);

                    (*regionVolumes)[region] = volumeScale * stat.getVolume();
                    (*regionSums[c])[region] = map.mapFromDataToValue(stat.getMass());
                    (*regionMean[c])[region] = map.mapFromDataToValue(stat.getMean());
                    (*regionMin[c])[region] = map.mapFromDataToValue(stat.getMin());
                    (*regionMax[c])[region] = map.mapFromDataToValue(stat.getMax());
                    
                    const auto center = dvec3{data2world * dvec4{stat.getCenter(), 1.0}};
                    const auto com = dvec3{data2world * dvec4{stat.getCenterOfMass(), 1.0}};

                    for (int k = 0; k < 3; ++k) {
                        (*regionCenter[k])[region] = center[k];
                        (*regionCoM[c][k])[region] = com[k];
                    }
                }
            });

        return df;
    }
};

void VolumeRegionStatistics::process() {

    auto calc = [volume = volume_.getData(), atlas = atlas_.getData()]() {
        if (volume->getDimensions() != atlas->getDimensions()) {
            throw Exception(fmt::format("Unexpected dimension missmatch. Volume: {}, Atlas: {}",
                                        volume->getDimensions(), atlas->getDimensions()),
                            IVW_CONTEXT);
        }

        StatsFunctor sf{};
        return wrappingDispatch<std::shared_ptr<DataFrame>>(sf, volume->getWrapping(), *volume,
                                                            *atlas);
    };

    dataFrame_.setData(nullptr);
    dispatchOne(calc, [this](std::shared_ptr<DataFrame> result) {
        dataFrame_.setData(result);
        newResults();
    });
}
}  // namespace inviwo
