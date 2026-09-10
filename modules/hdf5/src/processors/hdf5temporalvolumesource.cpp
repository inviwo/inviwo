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
#include <modules/hdf5/datastructures/hdf5handle.h>
#include <modules/hdf5/hdf5temporalvolumeloader.h>
#include <modules/hdf5/datastructures/hdf5path.h>
#include <modules/hdf5/hdf5utils.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/network/networklock.h>

#include <algorithm>
#include <functional>
#include <numeric>
#include <limits>
#include <ranges>
#include <tuple>

#include <inviwo/core/datastructures/unitsystem.h>
#include <inviwo/core/util/glm.h>

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
              constexpr char last = 'Z';
              for (size_t i = 0; i < maxRank; ++i) {
                  const auto ind = fmt::to_string(static_cast<char>(last - maxRank + i + 1));
                  opts.emplace_back(ind, ind, i);
              }
              return opts;
          }(),
          0)
    , dt_("dt", "Time Step (s)", 1.0, 0.0001, 1000.0)
    , cacheSize_("cacheSize", "Cache Size",
                 inviwo::util::ordinalCount<size_t>(8u, 256u).set(
                     "Maximum number of decoded frames kept in memory"_help)) {

    addPort(inport_);
    addPort(outport_);

    volumeSelection_.setSerializationMode(PropertySerializationMode::All);

    basisGroup_.addProperties(basisSelection_, spacing_, basis_);
    basisSelection_.onChange([this]() {
        switch (basisSelection_.getSelectedIndex()) {
            case 0: {  // User defined basis
                basis_.setReadOnly(false);
                spacing_.setVisible(false);
                break;
            }
            case 1: {  // User defined spacing
                basis_.setReadOnly(true);
                spacing_.setVisible(true);
                break;
            }
            default: {
                basis_.setReadOnly(true);
                spacing_.setVisible(false);
                break;
            }
        }
    });
    basisSelection_.setSerializationMode(PropertySerializationMode::All);
    outputGroup_.addProperties(datatype_, adjustBasis_, adjustOffset_, selection_);
    timeGroup_.addProperties(timeDimension_, dt_, cacheSize_);

    addProperties(volumeSelection_, basisGroup_, outputGroup_, timeGroup_);
}

HDF5ToTemporalVolume::~HDF5ToTemporalVolume() = default;

void HDF5ToTemporalVolume::process() try {
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
        const auto volumeOptions =
            volumeMatches_ | std::views::transform([](const auto& info) {
                return OptionPropertyStringOption{
                    info.path.toString(), util::dataSetDescription(info), info.path.toString()};
            }) |
            std::ranges::to<std::vector>();

        volumeSelection_.replaceOptions(volumeOptions);
        volumeSelection_.setCurrentStateAsDefault();

        // Update Basis Selection
        std::vector<OptionPropertyStringOption> basisOptions;
        basisOptions.emplace_back("default", "User defined basis", "default");
        basisOptions.emplace_back("default", "User defined spacing", "default");
        for (const auto& meta : basisMatches_) {
            const auto path = meta.path.toString();
            basisOptions.emplace_back(path, util::dataSetDescription(meta), path);
        }
        basisSelection_.replaceOptions(basisOptions);
        basisSelection_.setCurrentStateAsDefault();
    }

    if (volumeMatches_.empty()) {
        outport_.clear();
        return;
    }

    const auto& volumeInfo = volumeMatches_[volumeSelection_.getSelectedIndex()];
    selection_.update(volumeInfo);

    const auto* format = util::conversionFormat(datatype_.getSelectedIndex());
    const auto basis = computeBasis(volumeInfo);

    auto loader = std::make_unique<HDF5TemporalVolumeLoader>(
        *data + volumeInfo.path, selection_.getSelection(),
        static_cast<size_t>(timeDimension_.getSelectedValue()), format, basis, dt_.get());

    outport_.setData(std::make_shared<TemporalVolume>(std::move(loader), cacheSize_.get()));

} catch (H5::Exception& e) {
    throw Exception(SourceContext{}, "Error reading HDF5 data: {}", e.getDetailMsg());
}

dmat4 HDF5ToTemporalVolume::computeBasis(const DataSetInfo& volumeInfo) {
    dmat4 basis = basis_;
    const auto cmdimsView = volumeInfo.getColumnMajorDimensions();
    const std::vector<size_t> cmdims(cmdimsView.begin(), cmdimsView.end());

    switch (basisSelection_.getSelectedIndex()) {
        case 0: {  // User defined basis
            break;
        }
        case 1: {  // User defined spacing
            size3_t outDims(1u);
            for (size_t k = 0; k < 3; ++k) {
                const size_t idx = cmdims.size() - 3 + k;
                outDims[k] = clamp(selection_.getSelection()[idx], cmdims[idx]).count;
            }
            const auto diag = dvec4{dvec3(outDims) * spacing_.get(), 1.0};
            auto b = glm::diagonal4x4(diag);
            const auto offset = -0.5 * dvec3(b[0] + b[1] + b[2]);
            b[3] = dvec4(offset, 1.0);
            basis = b;
            break;
        }
        default: {
            basis = getBasisFromMeta(basisMatches_[basisSelection_.getSelectedIndex() - 2]);
            break;
        }
    }

    if (adjustBasis_) {
        auto sels = selection_.getSelection();
        // the time dimension varies per frame but must not be treated as a spatial axis here
        const size_t numExtraDims = sels.size() - 3;
        const size_t timeIdx = static_cast<size_t>(timeDimension_.getSelectedValue());
        if (timeIdx < numExtraDims) {
            sels[timeIdx].count = 1;
        }

        auto selAndDims =
            std::views::zip(sels, cmdims) | std::views::transform([](auto&& item) {
                return std::tuple{std::apply(clamp, item), std::get<1>(item)};
            }) |
            std::views::filter([](auto&& item) { return std::get<0>(item).count > 1; });

        for (auto&& [i, item] : std::views::zip(std::views::iota(0uz), selAndDims)) {
            if (i > 2) throw Exception("Invalid selection, resulting rank > 3");

            auto&& [sel, dim] = item;
            if (adjustOffset_) {
                basis[3] += basis[i] * static_cast<double>(sel.start) / static_cast<double>(dim);
            }
            basis[i] *= static_cast<double>(sel.count * sel.stride) / static_cast<double>(dim);
        }
        if (!adjustOffset_) {
            const vec3 offset = -0.5f * vec3(basis[0] + basis[1] + basis[2]);
            basis[3] = vec4(offset, 1.0f);
        }
    }

    return basis;
}

dmat4 HDF5ToTemporalVolume::getBasisFromMeta(const DataSetInfo& meta) {
    dmat4 basis(1.0);

    if (inport_.hasData()) {
        const auto data = inport_.getData();
        const H5::DataSet dataset = data->open(meta.path);
        const H5::DataSpace space = dataset.getSpace();
        const int rank = space.getSimpleExtentNdims();
        if (rank != 2)
            throw DataReaderException(SourceContext{},
                                      "Could not create Basis from: {} Invalid rank",
                                      meta.path.toString());
        std::vector<hsize_t> dims(rank);
        space.getSimpleExtentDims(dims.data());

        static constexpr std::array<size_t, 2> basisDim{3, 3};
        static constexpr std::array<size_t, 2> basisAndOffsetDim{4, 4};

        if (std::ranges::equal(dims, basisDim)) {
            dmat3 bas;
            dataset.read(glm::value_ptr(bas), H5::PredType::NATIVE_DOUBLE);
            basis = dmat4{bas};
            const auto offset = -0.5 * (bas[0] + bas[1] + bas[2]);
            basis[3] = dvec4{offset, 1.0};

        } else if (std::ranges::equal(dims, basisAndOffsetDim)) {
            dataset.read(glm::value_ptr(basis), H5::PredType::NATIVE_DOUBLE);
        } else {
            throw DataReaderException(SourceContext{},
                                      "Could not create Basis from: {} Invalid dimensions",
                                      meta.path.toString());
        }
    }
    return basis;
}

}  // namespace inviwo::hdf5
