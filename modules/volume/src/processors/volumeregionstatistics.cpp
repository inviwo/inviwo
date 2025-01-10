/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2025 Inviwo Foundation
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
#include <span>
#include <numbers>

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeRegionStatistics::processorInfo_{
    "org.inviwo.VolumeRegionStatistics",                          // Class identifier
    "Volume Region Statistics",                                   // Display name
    "Volume Operation",                                           // Category
    CodeState::Stable,                                            // Code state
    Tags::CPU | Tag{"Volume"} | Tag{"Atlas"} | Tag{"DataFrame"},  // Tags
    R"(Calculate statistics for each volume region/segment in a volume.
    The following statistics are calculated for each region:
     * Volume, given in World space
     * Sum for each channel, given in "Value" range. This assumes a density in each voxel,
       since we sum the voxel value times the voxel volume.
     * Mean for each channel, given in "Value" range
     * Min for each channel, given in "Value" range
     * Max for each channel, given in "Value" range
     * Center (x,y,z) mean position in each region, given in `Result Space` coordinates
     * Center of Mass for each channel (x, y, z), given in `Result Space` coordinates
    )"_unindentHelp

};
const ProcessorInfo& VolumeRegionStatistics::getProcessorInfo() const { return processorInfo_; }

VolumeRegionStatistics::VolumeRegionStatistics()
    : PoolProcessor()
    , volume_("volume", "Segmented input volume"_help)
    , atlas_{"atlas", R"(Index volume, of unsigned integer type, assigning a region index to each
        voxel. Has to have the same dimensions as volume. The index range is assumed to be
        [0, dataMap.dataRange.y] and without gaps.)"_unindentHelp}
    , dataFrame_{"statistics", "Data Frame with the statistics for each region."_help}
    , space_{"space",
             "Result Space",
             "The spatial domain of the resulting statistics. Data, Model, World, or  Index, "
             "defaults to World."_help,
             {CoordinateSpace::Data, CoordinateSpace::Model, CoordinateSpace::World,
              CoordinateSpace::Index},
             2} {

    addPorts(volume_, atlas_, dataFrame_);
    addProperties(space_);
}

auto addColumns(DataFrame& df, std::string_view name, size_t size, Unit unit,
                std::optional<dvec2> range) {
    auto* data = &df.addColumn<double>(name, size, unit, range)
                      ->getTypedBuffer()
                      ->getEditableRAMRepresentation()
                      ->getDataContainer();
    return data;
}

auto addColumns(DataFrame& df, size_t extent, std::string_view name, size_t size,
                std::span<const Unit> units, std::span<const std::optional<dvec2>> ranges,
                std::span<const std::string_view> labels) {
    IVW_ASSERT(units.size() >= extent, "Size missmatch");
    IVW_ASSERT(ranges.size() >= extent, "Size missmatch");
    IVW_ASSERT(labels.size() >= extent, "Size missmatch");

    return util::table(
        [&](auto index) {
            const auto fullName = fmt::format("{} {}", name, labels[index]);
            auto* data = &df.addColumn<double>(fullName, size, units[index], ranges[index])
                              ->getTypedBuffer()
                              ->getEditableRAMRepresentation()
                              ->getDataContainer();
            return data;
        },
        0, static_cast<int>(extent));
}

auto addColumns(DataFrame& df, size_t extent, size_t comps, std::string_view name, size_t size,
                std::span<const Unit> units, std::span<const std::optional<dvec2>> ranges,
                std::span<const std::string_view> majorLabels,
                std::span<const std::string_view> minorLabels) {

    IVW_ASSERT(units.size() >= comps, "Size missmatch");
    IVW_ASSERT(ranges.size() >= comps, "Size missmatch");
    IVW_ASSERT(majorLabels.size() >= extent, "Size missmatch");
    IVW_ASSERT(minorLabels.size() >= comps, "Size missmatch");

    return util::table(
        [&](auto index) {
            return util::table(
                [&](auto comp) {
                    const auto fullName =
                        fmt::format("{} {} {}", name, majorLabels[index], minorLabels[comp]);
                    auto* data = &df.addColumn<double>(fullName, size, units[comp], ranges[comp])
                                      ->getTypedBuffer()
                                      ->getEditableRAMRepresentation()
                                      ->getDataContainer();
                    return data;
                },
                0, static_cast<int>(comps));
        },
        0, static_cast<int>(extent));
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
            const auto theta = pos / dim[N] * 2.0 * std::numbers::pi;
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
            const auto theta = std::atan2(-acc.second, -acc.first) + std::numbers::pi;
            return dim[N] * theta / (2.0 * std::numbers::pi);
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
    using DispatchFunctor = Ret (*)(Functor&& func, Args&&...);

    constexpr auto table = build_array<3>([](auto x) constexpr {
        using XT = decltype(x);
        return build_array<3>([](auto y) constexpr {
            using YT = decltype(y);
            return build_array<3>([](auto z) constexpr -> DispatchFunctor {
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
    const size_t nRegions;
    const size_t minRegionId;
    const size_t channels;
    std::shared_ptr<DataFrame> df;

    const VolumeRAM* volumeRep;
    const VolumeRAM* atlasRep;
    const DataMapper map;

    const dvec3 dim;
    const mat4 data2dest;
    const mat4 index2dest;
    const mat4 index2data;
    const double volumeScale;

    std::vector<double>* regionVolumes;
    std::vector<std::vector<double>*> regionSums;
    std::vector<std::vector<double>*> regionMean;
    std::vector<std::vector<double>*> regionMin;
    std::vector<std::vector<double>*> regionMax;
    std::vector<std::vector<double>*> regionCenter;
    std::vector<std::vector<std::vector<double>*>> regionCoM;

    StatsFunctor(const Volume& volume, const Volume& atlas, CoordinateSpace destSpace)
        : nRegions{static_cast<size_t>(atlas.dataMap.dataRange.y - atlas.dataMap.dataRange.x + 1)}
        , minRegionId{static_cast<size_t>(atlas.dataMap.dataRange.x)}
        , channels{volume.getDataFormat()->getComponents()}
        , df{std::make_shared<DataFrame>(static_cast<uint32_t>(nRegions))}
        , volumeRep{volume.getRepresentation<VolumeRAM>()}
        , atlasRep{atlas.getRepresentation<VolumeRAM>()}
        , map{volume.dataMap}
        , dim{static_cast<dvec3>(volume.getDimensions())}
        , data2dest{volume.getCoordinateTransformer().getMatrix(CoordinateSpace::Data, destSpace)}
        , index2dest{volume.getCoordinateTransformer().getMatrix(CoordinateSpace::Index, destSpace)}
        , index2data{volume.getCoordinateTransformer().getMatrix(CoordinateSpace::Index,
                                                                 CoordinateSpace::Data)}
        , volumeScale{voxelVolume(index2dest)} {

        const auto& axes = volume.axes;
        const std::array<std::string_view, 3> axesNames = {axes[0].name, axes[1].name,
                                                           axes[2].name};
        const std::array<Unit, 3> axesUnits = {axes[0].unit, axes[1].unit, axes[2].unit};
        static constexpr std::array<const std::string_view, 4> indexLabels = {"0", "1", "2", "3"};
        const auto channelLabels = std::span<const std::string_view>(indexLabels.data(), channels);

        const auto valueUnits = util::make_array<4>([&](auto) { return map.valueAxis.unit; });

        const auto defaultRanges =
            util::make_array<4>([&](auto) -> std::optional<dvec2> { return {}; });

        const auto volumeUnit = axes[0].unit * axes[1].unit * axes[2].unit;
        const auto sumUnits =
            util::make_array<4>([&](auto) { return volumeUnit * map.valueAxis.unit; });

        const auto posMin = dvec3{data2dest * dvec4{0.0, 0.0, 0.0, 1.0}};
        const auto posMax = dvec3{data2dest * dvec4{1.0, 1.0, 1.0, 1.0}};
        std::array<std::optional<dvec2>, 3> sizeRange = {{dvec2{posMin[0], posMax[0]},
                                                          dvec2{posMin[1], posMax[1]},
                                                          dvec2{posMin[2], posMax[2]}}};

        regionVolumes = addColumns(*df, "Volume", nRegions, volumeUnit, {});
        regionSums =
            addColumns(*df, channels, "Sum", nRegions, sumUnits, defaultRanges, channelLabels);
        regionMean =
            addColumns(*df, channels, "Mean", nRegions, valueUnits, defaultRanges, channelLabels);
        regionMin =
            addColumns(*df, channels, "Min", nRegions, valueUnits, defaultRanges, channelLabels);
        regionMax =
            addColumns(*df, channels, "Max", nRegions, valueUnits, defaultRanges, channelLabels);
        regionCenter = addColumns(*df, 3, "Center", nRegions, axesUnits, sizeRange, axesNames);
        regionCoM = addColumns(*df, channels, 3, "CoM", nRegions, axesUnits, sizeRange,
                               channelLabels, std::span(axesNames));
    }

    size_t getRegion(size_t index) const {
        return atlasRep->dispatch<size_t, dispatching::filter::UnsignedIntegerScalars>(
            [&](auto rep) { return rep->getDataTyped()[index] - minRegionId; });
    }
    double getValue(size_t index, size_t channel) const {
        return volumeRep->dispatch<double, dispatching::filter::All>([&](auto rep) {
            return static_cast<double>(util::glmcomp(rep->getDataTyped()[index], channel));
        });
    }

    template <Wrapping wrapX, Wrapping wrapY, Wrapping wrapZ>
    std::shared_ptr<DataFrame> operator()() const {
        using TStats = Stats<double, wrapX, wrapY, wrapZ>;
        std::vector<TStats> stats(nRegions * channels, TStats{dvec3{1.0}});

        const util::IndexMapper2D regionMapper(size2_t{nRegions, channels});

        const util::IndexMapper3D indexMapper(dim);
        util::forEachVoxel(*volumeRep, [&](const size3_t& pos) {
            const auto p = indexMapper(pos);
            for (size_t c = 0; c < channels; ++c) {
                const auto region = regionMapper(getRegion(p), c);
                const auto value = getValue(p, c);
                const auto dpos = dvec3{index2data * dvec4{pos, 1.0}};
                if (region < stats.size()) {
                    stats[region].add(dpos, value);
                } else {
                    throw Exception(
                        SourceContext{},
                        "Unexpected region index found '{}' expected value in range [0,{})", region,
                        stats.size());
                }
            }
        });

        for (auto&& [i, stat] : util::enumerate(stats)) {
            const auto [region, c] = regionMapper(i);

            if (stat.getVolume() == 0.0) {
                throw Exception("Empty volume!");
            }
            (*regionVolumes)[region] = volumeScale * stat.getVolume();
            (*regionSums[c])[region] = map.mapFromDataToValue(volumeScale * stat.getMass());
            (*regionMean[c])[region] = map.mapFromDataToValue(stat.getMean());
            (*regionMin[c])[region] = map.mapFromDataToValue(stat.getMin());
            (*regionMax[c])[region] = map.mapFromDataToValue(stat.getMax());

            const auto center = dvec3{data2dest * dvec4{stat.getCenter(), 1.0}};
            const auto com = dvec3{data2dest * dvec4{stat.getCenterOfMass(), 1.0}};
            for (int k = 0; k < 3; ++k) {
                (*regionCenter[k])[region] = center[k];
                (*regionCoM[c][k])[region] = com[k];
            }
        }

        df->getIndexColumn()->setHeader("Region Index");
        auto& index = df->getIndexColumn()
                          ->getTypedBuffer()
                          ->getEditableRAMRepresentation()
                          ->getDataContainer();
        std::transform(index.begin(), index.end(), index.begin(),
                       [&](auto index) { return index + static_cast<std::uint32_t>(minRegionId); });

        return df;
    }
};

void VolumeRegionStatistics::process() {
    auto calc = [volume = volume_.getData(), atlas = atlas_.getData(),
                 space = space_.getSelectedValue()]() {
        if (volume->getDimensions() != atlas->getDimensions()) {
            throw Exception(SourceContext{},
                            "Unexpected dimension missmatch. Volume: {}, Atlas: {}",
                            volume->getDimensions(), atlas->getDimensions());
        }
        if (atlas->getDataFormat()->getComponents() != 1 ||
            atlas->getDataFormat()->getNumericType() != NumericType::UnsignedInteger) {
            throw Exception(
                SourceContext{},
                "Unexpected atlas format found, expected an unsigned integer type. Got: {}",
                atlas->getDataFormat()->getString());
        }

        StatsFunctor sf{*volume, *atlas, space};
        return wrappingDispatch<std::shared_ptr<DataFrame>>(sf, volume->getWrapping());
    };

    dataFrame_.setData(nullptr);
    dispatchOne(calc, [this](std::shared_ptr<DataFrame> result) {
        dataFrame_.setData(result);
        newResults();
    });
}
}  // namespace inviwo
