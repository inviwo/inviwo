/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2026 Inviwo Foundation
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

#include <modules/hdf5/processors/hdf5volumesource.h>
#include <modules/hdf5/datastructures/hdf5handle.h>
#include <modules/hdf5/hdf5read.h>
#include <modules/hdf5/datastructures/hdf5path.h>
#include <modules/hdf5/hdf5utils.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/network/networklock.h>

#include <algorithm>
#include <functional>
#include <numeric>
#include <limits>

#include <inviwo/core/datastructures/unitsystem.h>
#include <inviwo/core/util/glm.h>

namespace inviwo {

namespace hdf5 {

const ProcessorInfo HDF5ToVolume::processorInfo_{
    "org.inviwo.hdf5.ToVolume",  // Class identifier
    "HDF5 To Volume",            // Display name
    "Data Input",                // Category
    CodeState::Stable,           // Code state
    Tags::None,                  // Tags
    "Load a volume from a HDF5 file handle."_help,
};
const ProcessorInfo& HDF5ToVolume::getProcessorInfo() const { return processorInfo_; }

HDF5ToVolume::HDF5ToVolume()
    : Processor()
    , inport_("inport")
    , outport_("outport")

    , volumeSelection_("volumeSelection", "Volume")

    , automaticEvaluation_("automaticEvaluation", "Automatic loading", true,
                           InvalidationLevel::Valid)
    , evaluate_("evaluate", "Load", [this]() { dirty_ = true; })

    , basisGroup_("basisGroup", "Basis")
    , basisSelection_("basisSelection", "Source")
    , basis_("basis", "Matrix", mat4(1.0f), inviwo::util::filled<mat4>(-1000.f),
             inviwo::util::filled<mat4>(1000.f))
    , spacing_("spacing", "Spacing", vec3(0.01f), vec3(0.0f), vec3(1.0f))
    , information_("Information", "Data information")
    , outputGroup_("outputGroup", "Operations", InvalidationLevel::Valid)
    , datatype_("convertType", "Convert to type", util::conversionOptions(), 0)
    , adjustBasis_("adjustBasis", "Automatically adjust basis", true)
    , adjustOffset_("adjustOffset", "Automatically adjust offset", true)
    , selection_("selection", "Selection", 6)
    , cache_{}
    , dirty_(false) {

    addPort(inport_);
    addPort(outport_);

    volumeSelection_.onChange([this]() { onSelectionChange(); });
    volumeSelection_.setSerializationMode(PropertySerializationMode::All);

    automaticEvaluation_.onChange([this]() { evaluate_.setReadOnly(automaticEvaluation_); });

    basisGroup_.addProperties(basisSelection_, spacing_, basis_);

    basisSelection_.onChange([this]() { onBasisSelectionChange(); });
    basisSelection_.setSerializationMode(PropertySerializationMode::All);

    outputGroup_.addProperties(datatype_, adjustBasis_, adjustOffset_, selection_);
    outputGroup_.onChange([this]() {
        if (automaticEvaluation_) {
            dirty_ = true;
            this->invalidate(InvalidationLevel::InvalidOutput);
        }
    });

    addProperties(volumeSelection_, automaticEvaluation_, evaluate_, basisGroup_, information_,
                  outputGroup_);
}

HDF5ToVolume::~HDF5ToVolume() = default;

void HDF5ToVolume::process() try {
    const auto data = inport_.getData();

    if (inport_.isChanged()) {

        const auto metadata = util::getDataSets(*data);

        volumeMatches_.assign_range(metadata | std::views::filter([](const DataSetInfo& info) {
                                        return info.dimensions.size() >= 3ull &&
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
        std::vector<OptionPropertyStringOption> volumeOptions;
        for (const auto& info : volumeMatches_) {
            volumeOptions.emplace_back(info.path.toString(), util::dataSetDescription(info),
                                       info.path.toString());
        }

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

        onSelectionChange();
        onBasisSelectionChange();
    }

    const auto& volumeInfo = volumeMatches_[volumeSelection_.getSelectedIndex()];

    if (dirty_) {
        const auto* format = util::conversionFormat(datatype_.getSelectedIndex());
        volume_ = std::shared_ptr<Volume>(getVolumeAtPathAsType(
            *data + volumeInfo.path, selection_.getSelection(), format, std::ref(cache_)));
        information_.updateForNewVolume(*volume_, deserialized_ ? inviwo::util::OverwriteState::Yes
                                                                : inviwo::util::OverwriteState::No);

        dirty_ = false;
        deserialized_ = false;
    }

    if (volume_) {
        information_.updateVolume(*volume_);

        switch (basisSelection_.getSelectedIndex()) {
            case 0: {  // User defined basis
                break;
            }
            case 1: {  // User defined spacing
                const auto dim = volume_->getDimensions();
                const auto diag = dvec4{dvec3(dim) * spacing_.get(), 1.0};
                auto basis = glm::diagonal4x4(diag);
                const auto offset = -0.5 * dvec3(basis[0] + basis[1] + basis[2]);
                basis[3] = dvec4(offset, 1.0);
                basis_.set(basis);
                break;
            }
            default: {
                const auto basis =
                    getBasisFromMeta(basisMatches_[basisSelection_.getSelectedIndex() - 2]);
                basis_.set(basis);
                break;
            }
        }

        if (adjustBasis_) {
            dmat4 basis = basis_;

            auto sels = selection_.getSelection();
            auto dims = volumeInfo.getColumnMajorDimensions();

            auto selAndDims =
                std::views::zip(sels, dims) | std::views::transform([](auto&& item) {
                    return std::tuple{std::apply(clamp, item), std::get<1>(item)};
                }) |
                std::views::filter([](auto&& item) { return std::get<0>(item).count > 1; });

            for (auto&& [i, item] : std::views::zip(std::views::iota(0uz), selAndDims)) {
                if (i > 2) throw Exception("Invalid selection, resulting rank > 3");

                auto&& [sel, dim] = item;
                if (adjustOffset_) {
                    basis[3] +=
                        basis[i] * static_cast<double>(sel.start) / static_cast<double>(dim);
                }
                basis[i] *= static_cast<double>(sel.count * sel.stride) / static_cast<double>(dim);
            }
            if (!adjustOffset_) {
                const vec3 offset = -0.5f * vec3(basis[0] + basis[1] + basis[2]);
                basis[3] = vec4(offset, 1.0f);
            }
            volume_->setModelMatrix(basis);
        } else {
            volume_->setModelMatrix(basis_);
        }

        outport_.setData(volume_);
    }
} catch (H5::Exception& e) {
    throw Exception(SourceContext{}, "Error reading HDF5 data: {}", e.getDetailMsg());
}

dmat4 HDF5ToVolume::getBasisFromMeta(const DataSetInfo& meta) {
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

void HDF5ToVolume::onBasisSelectionChange() {
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
}

void HDF5ToVolume::onSelectionChange() {
    dirty_ = true;
    if (!volumeMatches_.empty()) {
        const DataSetInfo volumeMeta = volumeMatches_[volumeSelection_.getSelectedIndex()];
        selection_.update(volumeMeta);
    }
}

void HDF5ToVolume::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    deserialized_ = true;
}

}  // namespace hdf5

}  // namespace inviwo
