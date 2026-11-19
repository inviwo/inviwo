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

#include <modules/hdf5/processors/hdf5directorytemporalvolumesource.h>

#include <inviwo/core/algorithm/markdown.h>
#include <inviwo/core/datastructures/unitsystem.h>
#include <inviwo/core/processors/processorstatus.h>
#include <inviwo/core/util/concat.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/glm.h>

#include <modules/hdf5/datastructures/hdf5handle.h>
#include <modules/hdf5/datastructures/hdf5path.h>
#include <modules/hdf5/hdf5multifiletemporalvolumeloader.h>
#include <modules/hdf5/hdf5read.h>
#include <modules/hdf5/hdf5utils.h>

#include <algorithm>
#include <functional>
#include <optional>
#include <ranges>
#include <string_view>
#include <tuple>

#include <fmt/format.h>

#include <warn/push>
#include <warn/ignore/all>
#include <H5Cpp.h>
#include <warn/pop>

namespace inviwo::hdf5 {

namespace {
constexpr size_t maxRank = 6;

std::optional<std::string_view> toOpt(const StringProperty& prop) {
    if (prop.get().empty()) {
        return std::nullopt;
    } else {
        return prop.get();
    }
}
}  // namespace

const ProcessorInfo HDF5DirectoryToTemporalVolume::processorInfo_{
    "org.inviwo.hdf5.DirectoryToTemporalVolume",  // Class identifier
    "HDF5 Directory To Temporal Volume",          // Display name
    "Data Input",                                 // Category
    CodeState::Experimental,                      // Code state
    Tags::CPU | Tag{"HDF5"} | Tag{"Volume"},      // Tags
    R"(Load a time-varying volume from a folder of HDF5 files with identical structure. The
    time steps of the selected dataset (rank >= 4, one dimension designated as the time axis)
    are concatenated across all matching files, sorted by file name. Dataset, basis, and range
    options are taken from the first file. Files are opened lazily when a frame is requested.)"_unindentHelp,
};
const ProcessorInfo& HDF5DirectoryToTemporalVolume::getProcessorInfo() const {
    return processorInfo_;
}

HDF5DirectoryToTemporalVolume::HDF5DirectoryToTemporalVolume()
    : Processor()
    , outport_{"outport", "The loaded temporal volume"_help}

    , folder_{"folder", "HDF5 Folder", "The folder to look for HDF5 files in"_help}
    , include_{"include", "Include",
               R"(Any path that matches this regular expression will be included.
               If the string is empty, all paths will be included)"_unindentHelp,
               ""}
    , exclude_{"exclude", "Exclude",
               R"(Any path that matches this regular expression will be excluded.
               If the string is empty, all paths will be included)"_unindentHelp,
               ""}
    , reload_{"reload", "Reload", "Rescan the folder and reload the data"_help}
    , fileCount_{"fileCount", "Files Found",
                 inviwo::util::ordinalCount<size_t>(0u, 1000u)
                     .set("Number of HDF5 files matching the include/exclude filters"_help)
                     .set(InvalidationLevel::Valid)
                     .set(PropertySemantics::Text)}

    , volumeSelection_{"volumeSelection", "Volume"}

    , config_{"config", "Config"}
    , basisGroup_{"basisGroup", "Basis"}
    , basisSelection_{"basisSelection", "Source"}
    , customBasis_{"customBasis", "User Basis", dmat4{1.0}, inviwo::util::filled<dmat4>(-1000.0),
                   inviwo::util::filled<dmat4>(1000.0)}
    , customSpacing_{"customSpacing", "User Spacing", dvec3{0.01}, dvec3{0.0f}, dvec3{1.0}}
    , transposeBasis_{"transposeBasis", "Transpose Basis", "Transpose the basis"_help, false}
    , swapBasisAxes_{"swapBasisAxes_", "Swap X & Z basis vectors",
                     "Swap the first and third basis vector"_help, false}
    , basis_{"basis", "Resulting Basis", dmat4{1.0}, inviwo::util::filled<dmat4>(-1000.0),
             inviwo::util::filled<dmat4>(1000.0)}
    , dataRangeGroup_("dataRangeGroup", "Data Range")
    , dataRangeSelection_{"dataRangeSelection", "Source Data Range"}
    , valueRangeSelection_{"valueRangeSelection", "Source Value Range"}
    , dataRange_{"dataRange", "Data Range",
                 inviwo::util::ordinalSymmetricVector(dvec2{0.0, 1.0}).setInc(dvec2{0.00001})}
    , valueRange_{"valueRange", "Value Range",
                  inviwo::util::ordinalSymmetricVector(dvec2{0.0, 1.0}).setInc(dvec2{0.00001})}
    , customDataRange_{"customDataRange", "Override Data Range",
                       inviwo::util::ordinalOptSymmetricVector(std::optional<dvec2>{std::nullopt})
                           .setInc(dvec2{0.00001})}
    , customValueRange_{"customValueRange", "Override Value Range",
                        inviwo::util::ordinalOptSymmetricVector(std::optional<dvec2>{std::nullopt})
                            .setInc(dvec2{0.00001})}
    , valueName_{"valueName", "Value name", ""}
    , valueUnit_{"valueUnit", "Value unit", ""}
    , interpolation_{"interpolation",
                     "Interpolation",
                     {InterpolationType::Linear, InterpolationType::Nearest},
                     0}
    , axesNames_{"axesNames", "Axes Names"}
    , axesUnits_{"axesUnits", "Axes Units"}
    , wrapping_{{
          {"xWrapping", "X Wrapping", {Wrapping::Clamp, Wrapping::Repeat, Wrapping::Mirror}, 0},
          {"yWrapping", "Y Wrapping", {Wrapping::Clamp, Wrapping::Repeat, Wrapping::Mirror}, 0},
          {"zWrapping", "Z Wrapping", {Wrapping::Clamp, Wrapping::Repeat, Wrapping::Mirror}, 0},
      }}

    , outputGroup_{"outputGroup", "Operations"}
    , datatype_{"convertType", "Convert to type", util::conversionOptions(), 0}
    , adjustBasis_{"adjustBasis", "Automatically adjust basis", true}
    , adjustOffset_{"adjustOffset", "Automatically adjust offset", true}
    , selection_{"selection", "Selection", maxRank}
    , timeGroup_{"timeGroup", "Time"}
    , timeDimension_{"timeDimension", "Time Dimension",
                     []() {
                         std::vector<OptionPropertyOption<size_t>> opts;
                         opts.reserve(maxRank);
                         for (size_t i = 0; i < maxRank; ++i) {
                             opts.emplace_back(fmt::format("dim{:02}", i),
                                               fmt::format("Dimension {}", i + 1), i);
                         }
                         return opts;
                     }(),
                     0}
    , dt_{"dt", "Time Step (s)", inviwo::util::ordinalScale(1.0, 1000.0)}
    , cacheSize_{"cacheSize", "Cache Size",
                 inviwo::util::ordinalCount<size_t>(8u, 256u).set(
                     "Maximum number of decoded frames kept in memory"_help)} {

    addPort(outport_);

    folder_.setContentType("volume");
    fileCount_.setReadOnly(true);
    fileCount_.setSerializationMode(PropertySerializationMode::None);

    volumeSelection_.setSerializationMode(PropertySerializationMode::All);

    basisGroup_.addProperties(basisSelection_, customBasis_, customSpacing_, transposeBasis_,
                              swapBasisAxes_, basis_);
    basis_.setReadOnly(true);
    basisSelection_.setSerializationMode(PropertySerializationMode::All);

    dataRangeGroup_.addProperties(dataRangeSelection_, valueRangeSelection_, dataRange_,
                                  valueRange_, customDataRange_, customValueRange_);
    dataRange_.setReadOnly(true);
    valueRange_.setReadOnly(true);
    dataRangeSelection_.setSerializationMode(PropertySerializationMode::All);
    valueRangeSelection_.setSerializationMode(PropertySerializationMode::All);

    outputGroup_.addProperties(datatype_, adjustBasis_, adjustOffset_);
    timeGroup_.addProperties(timeDimension_, dt_, cacheSize_);

    config_.addProperties(basisGroup_, dataRangeGroup_, valueName_, valueUnit_, interpolation_,
                          axesNames_, axesUnits_, wrapping_[0], wrapping_[1], wrapping_[2]);

    addProperties(folder_, include_, exclude_, reload_, fileCount_, volumeSelection_, config_,
                  timeGroup_, selection_, outputGroup_);

    isReady_.setUpdate([this]() -> ProcessorStatus {
        if (folder_.get().empty()) {
            return {ProcessorStatus::NotReady, "No folder selected"};
        }
        if (auto file =
                filesystem::getFileInFolder(folder_.get(), toOpt(include_), toOpt(exclude_))) {
            return ProcessorStatus::Ready;
        } else {
            return {ProcessorStatus::Error, file.error()};
        }
    });
    folder_.onChange([this]() { isReady_.update(); });
    include_.onChange([this]() { isReady_.update(); });
    exclude_.onChange([this]() { isReady_.update(); });
    reload_.onChange([this]() { isReady_.update(); });
}

HDF5DirectoryToTemporalVolume::~HDF5DirectoryToTemporalVolume() = default;

std::vector<std::filesystem::path> HDF5DirectoryToTemporalVolume::getFiles() const {
    return filesystem::getFilesInFolder(folder_.get(), toOpt(include_), toOpt(exclude_))
        .or_else([](std::string_view error)
                     -> std::expected<std::vector<std::filesystem::path>, std::string_view> {
            throw Exception(error, SourceContext{});
        })
        .value();
}

void HDF5DirectoryToTemporalVolume::process() try {
    const std::scoped_lock lock{Handle::globalMutex()};

    // Listing the folder is cheap, so always do it and only rescan the first file's
    // metadata when the file list actually changes (or a reload was requested).
    auto files = getFiles();
    const bool filesChanged = files != files_ || reload_.isModified();
    files_ = std::move(files);
    fileCount_.set(files_.size());

    if (files_.empty()) {
        volumeMatches_.clear();
        outport_.clear();
        return;
    }

    const Handle first{files_.front()};

    if (filesChanged) {
        const auto metadata = util::getDataSets(first);

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
        dataRangeMatches_.assign_range(metadata | std::views::filter([](const DataSetInfo& info) {
                                           auto dims = info.getColumnMajorDimensions();
                                           static constexpr std::array<size_t, 1> range{2};
                                           return std::ranges::equal(dims, range);
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

        // Update Data Range Selection
        const std::array<OptionPropertyStringOption, 1> rangeOptions{
            {{"user_range", "User defined range", "user_range"}}};

        dataRangeSelection_.replaceOptions(views::concat(
            rangeOptions, dataRangeMatches_ | std::views::transform(util::dataSetInfoToOption)));
        dataRangeSelection_.setCurrentStateAsDefault();
        valueRangeSelection_.replaceOptions(views::concat(
            rangeOptions, dataRangeMatches_ | std::views::transform(util::dataSetInfoToOption)));
        valueRangeSelection_.setCurrentStateAsDefault();
    }

    if (volumeMatches_.empty()) {
        outport_.clear();
        return;
    }

    const auto& volumeInfo = volumeMatches_[volumeSelection_.getSelectedIndex()];
    selection_.update(volumeInfo);

    auto selection = selection_.getSelection();
    const size_t timeIdx = timeDimension_.getSelectedValue();
    if (timeIdx < selection.size()) {
        selection[timeIdx].count = 1;
    }

    auto basis = [&]() {
        switch (basisSelection_.getSelectedIndex()) {
            case 0: {  // User defined basis
                return customBasis_.get();
            }
            case 1: {  // User defined spacing
                const auto dims =
                    util::validSelectionAndDims(selection, volumeInfo.getColumnMajorDimensions()) |
                    std::views::values | std::ranges::to<std::vector>();
                if (dims.size() != 3) {
                    throw Exception{SourceContext{}, "Invalid selection: expected rank 3, got {}",
                                    dims.size()};
                }
                return util::createBasis(size3_t{dims[0], dims[1], dims[2]}, customSpacing_.get());
            }
            default: {
                const auto basisInfo = basisMatches_[basisSelection_.getSelectedIndex() - 2];
                return getBasis(first + basisInfo.path);
            }
        }
    }();
    if (transposeBasis_) {
        basis = glm::transpose(basis);
    }
    if (swapBasisAxes_) {
        std::swap(basis[0], basis[2]);
    }
    basis_.set(basis);
    basis = util::adjustBasis(basis, selection, volumeInfo.getColumnMajorDimensions(),
                              adjustBasis_.get(), adjustOffset_.get());

    auto rangeFromOptionProperty = [&](size_t index) {
        switch (index) {
            case 0:  // user-defined range
                return dvec2{0.0, 1.0};
            default: {
                const auto rangeInfo = dataRangeMatches_[index - 1];
                return getRange(first + rangeInfo.path);
            }
        }
    };
    dataRange_.set(rangeFromOptionProperty(dataRangeSelection_.getSelectedIndex()));
    valueRange_.set(rangeFromOptionProperty(valueRangeSelection_.getSelectedIndex()));

    if (timeDimension_.getSelectedIndex() >= selection_.rank()) {
        throw Exception{SourceContext{}, "Time dimensions {} does not exist",
                        timeDimension_.getSelectedDisplayName()};
    }

    const VolumeConfig config{
        .formatId = util::conversionFormat(datatype_.getSelectedIndex()),
        .interpolation = interpolation_.getSelectedValue(),
        .wrapping = Wrapping3D{wrapping_[0].getSelectedValue(), wrapping_[1].getSelectedValue(),
                               wrapping_[2].getSelectedValue()},
        .xAxis = Axis{.name = axesNames_.strings[0].get(),
                      .unit = units::unit_from_string(axesUnits_.strings[0].get())},
        .yAxis = Axis{.name = axesNames_.strings[1].get(),
                      .unit = units::unit_from_string(axesUnits_.strings[1].get())},
        .zAxis = Axis{.name = axesNames_.strings[2].get(),
                      .unit = units::unit_from_string(axesUnits_.strings[2].get())},
        .valueAxis =
            Axis{.name = valueName_.get(), .unit = units::unit_from_string(valueUnit_.get())},
        .dataRange = customDataRange_.get().value_or(dataRange_.get()),
        .valueRange = customValueRange_.get().value_or(valueRange_.get()),
        .model = basis,
        .world = dmat4{1.0}};

    auto loader = std::make_unique<HDF5MultiFileTemporalVolumeLoader>(
        files_, volumeInfo.path, selection_.getSelection(), timeDimension_.getSelectedValue(),
        Seconds{dt_.get()}, config);

    outport_.setData(std::make_shared<TemporalVolume>(std::move(loader), cacheSize_.get()));

} catch (H5::Exception& e) {
    throw Exception(SourceContext{}, "Error reading HDF5 data: {}", e.getDetailMsg());
}

}  // namespace inviwo::hdf5
