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

#include <modules/hdf5/processors/hdf5temporalvolumesource.h>

#include <inviwo/core/datastructures/unitsystem.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/concat.h>
#include <inviwo/core/util/glm.h>

#include <modules/hdf5/datastructures/hdf5handle.h>
#include <modules/hdf5/hdf5temporalvolumeloader.h>
#include <modules/hdf5/datastructures/hdf5path.h>
#include <modules/hdf5/hdf5utils.h>
#include <modules/hdf5/hdf5read.h>

#include <algorithm>
#include <functional>
#include <numeric>
#include <limits>
#include <ranges>
#include <tuple>

#include <fmt/format.h>

namespace inviwo::hdf5 {

namespace {
constexpr size_t maxRank = 6;
}  // namespace

const ProcessorInfo HDF5ToTemporalVolume::processorInfo_{
    "org.inviwo.hdf5.ToTemporalVolume",       // Class identifier
    "HDF5 To Temporal Volume",                // Display name
    "Data Input",                             // Category
    CodeState::Experimental,                  // Code state
    Tags::CPU | Tag{"HDF5"} | Tag{"Volume"},  // Tags
    "Load a time-varying volume from a HDF5 file handle. Requires a dataset of rank >= 4, "
    "one dimension of which is designated as the time axis."_help,
};
const ProcessorInfo& HDF5ToTemporalVolume::getProcessorInfo() const { return processorInfo_; }

HDF5ToTemporalVolume::HDF5ToTemporalVolume()
    : Processor()
    , inport_("inport")
    , outport_("outport", "The loaded temporal volume"_help)

    , volumeSelection_("volumeSelection", "Volume")

    , basisGroup_("basisGroup", "Basis")
    , basisSelection_("basisSelection", "Source")
    , basis_("basis", "Matrix", mat4(1.0f), inviwo::util::filled<mat4>(-1000.f),
             inviwo::util::filled<mat4>(1000.f))
    , spacing_("spacing", "Spacing", vec3(0.01f), vec3(0.0f), vec3(1.0f))
    , outputGroup_("outputGroup", "Operations")
    , datatype_("convertType", "Convert to type", util::conversionOptions(), 0)
    , adjustBasis_("adjustBasis", "Automatically adjust basis", true)
    , adjustOffset_("adjustOffset", "Automatically adjust offset", true)
    , selection_("selection", "Selection", maxRank)
    , timeGroup_("timeGroup", "Time")
    , timeDimension_(
          "timeDimension", "Time Dimension",
          []() {
              std::vector<OptionPropertyOption<size_t>> opts;
              for (size_t i = 0; i < maxRank; ++i) {
                  opts.emplace_back(fmt::format("dim{:02}", i), fmt::format("Dimension {}", i + 1),
                                    i);
              }
              return opts;
          }(),
          maxRank - 1)
    , dt_("dt", "Time Step (s)", 1.0, 0.0001, 1000.0)
    , cacheSize_("cacheSize", "Cache Size",
                 inviwo::util::ordinalCount<size_t>(8u, 256u).set(
                     "Maximum number of decoded frames kept in memory"_help)) {

    addPort(inport_);
    addPort(outport_);

    volumeSelection_.setSerializationMode(PropertySerializationMode::All);

    basisGroup_.addProperties(basisSelection_, spacing_, basis_);
    basis_.readonlyDependsOn(basisSelection_, [](auto& p) { return p.getSelectedIndex() != 0; });
    spacing_.visibilityDependsOn(basisSelection_,
                                 [](auto& p) { return p.getSelectedIndex() == 1; });
    basisSelection_.setSerializationMode(PropertySerializationMode::All);
    outputGroup_.addProperties(datatype_, adjustBasis_, adjustOffset_);
    timeGroup_.addProperties(timeDimension_, dt_, cacheSize_);

    addProperties(volumeSelection_, basisGroup_, timeGroup_, selection_, outputGroup_);
}

HDF5ToTemporalVolume::~HDF5ToTemporalVolume() = default;

void HDF5ToTemporalVolume::process() try {
    const std::scoped_lock lock{Handle::globalMutex()};

    const auto data = inport_.getData();

    if (inport_.isChanged()) {
        const auto metadata = util::getDataSets(*data);

        volumeMatches_.assign_range(metadata | std::views::filter([](const DataSetInfo& info) {
                                        return info.dimensions.size() >= 4ull &&
                                               std::ranges::fold_left(info.dimensions, size_t{1},
                                                                      std::multiplies{}) > 50000ull;
                                    }));
        basisMatches_.assign_range(metadata | std::views::filter([](const DataSetInfo& info) {
                                       auto dims = info.getColumnMajorDimensions();
                                       static constexpr std::array<size_t, 2> basis{3, 3};
                                       static constexpr std::array<size_t, 2> basisAndOffset{4, 4};
                                       return std::ranges::equal(dims, basis) ||
                                              std::ranges::equal(dims, basisAndOffset);
                                   }));

        // Update Volume Selection
        volumeSelection_.replaceOptions(volumeMatches_ |
                                        std::views::transform(util::dataSetInfoToOption));
        volumeSelection_.setCurrentStateAsDefault();

        // Update Basis Selection
        const std::array<OptionPropertyStringOption, 2> basisOptions{
            {{"user_basis", "User defined basis", "user_basis"},
             {"user_spacing", "User defined spacing", "user_spacing"}}};

        basisSelection_.replaceOptions(views::concat(
            basisOptions, basisMatches_ | std::views::transform(util::dataSetInfoToOption)));
        basisSelection_.setCurrentStateAsDefault();
    }

    if (volumeMatches_.empty()) {
        outport_.clear();
        return;
    }

    const auto& volumeInfo = volumeMatches_[volumeSelection_.getSelectedIndex()];
    selection_.update(volumeInfo);

    const auto* format = util::conversionFormat(datatype_.getSelectedIndex());

    auto selection = selection_.getSelection();
    const size_t timeIdx = timeDimension_.getSelectedValue();
    if (timeIdx < selection.size()) {
        selection[timeIdx].count = 1;
    }

    auto basis = [&]() {
        switch (basisSelection_.getSelectedIndex()) {
            case 0: {  // User defined basis
                return basis_.get();
            }
            case 1: {  // User defined spacing
                const auto dims =
                    util::validSelectionAndDims(selection, volumeInfo.getColumnMajorDimensions()) |
                    std::views::values | std::ranges::to<std::vector>();
                if (dims.size() != 3) {
                    throw Exception{SourceContext{}, "Invalid selection: expected 3, got {}",
                                    dims.size()};
                }
                return util::createBasis(size3_t{dims[0], dims[1], dims[2]}, spacing_.get());
            }
            default: {
                const auto basisInfo = basisMatches_[basisSelection_.getSelectedIndex() - 2];
                return getBasis(*data + basisInfo.path);
            }
        }
    }();
    basis = util::adjustBasis(basis, selection, volumeInfo.getColumnMajorDimensions(),
                              adjustBasis_.get(), adjustOffset_.get());

    if (timeDimension_.getSelectedIndex() >= selection_.rank()) {
        throw Exception{SourceContext{}, "Time dimensions {} does not exist",
                        timeDimension_.getSelectedDisplayName()};
    }

    auto loader = std::make_unique<HDF5TemporalVolumeLoader>(
        *data + volumeInfo.path, selection_.getSelection(), timeDimension_.getSelectedValue(),
        format, basis, dt_.get());

    outport_.setData(std::make_shared<TemporalVolume>(std::move(loader), cacheSize_.get()));

} catch (H5::Exception& e) {
    throw Exception(SourceContext{}, "Error reading HDF5 data: {}", e.getDetailMsg());
}

}  // namespace inviwo::hdf5
