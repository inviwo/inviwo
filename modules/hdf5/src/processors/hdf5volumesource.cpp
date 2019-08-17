/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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
#include <modules/hdf5/datastructures/hdf5path.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datareaderexception.h>
#include <functional>
#include <numeric>
#include <limits>

namespace inviwo {

namespace hdf5 {

const ProcessorInfo HDF5ToVolume::processorInfo_{
    "org.inviwo.hdf5.ToVolume",  // Class identifier
    "HDF5 To Volume",            // Display name
    "Data Input",                // Category
    CodeState::Stable,           // Code state
    Tags::None,                  // Tags
};
const ProcessorInfo HDF5ToVolume::getProcessorInfo() const { return processorInfo_; }

HDF5ToVolume::HDF5ToVolume()
    : Processor()
    , inport_("inport")
    , outport_("outport")

    , volumeSelection_("volumeSelection", "Volume")

    , automaticEvaluation_("automaticEvaluation", "Automatic loading", true,
                           InvalidationLevel::Valid)
    , evaluate_("evaluate", "Load")

    , basisGroup_("basisGroup", "Basis")
    , basisSelection_("basisSelection", "Source")
    , basis_("basis", "Matrix", mat4(1.0f), inviwo::util::filled<mat4>(-1000.f),
             inviwo::util::filled<mat4>(1000.f))
    , spacing_("spacing", "Spacing", vec3(0.01f), vec3(0.0f), vec3(1.0f))

    , information_("Information", "Data information")
    , dataRange_("dataRange", "Range", 0., 255.0, -DataFloat64::max(), DataFloat64::max(), 0.0, 0.0,
                 InvalidationLevel::InvalidOutput, PropertySemantics("Text"))
    , dataDimensions_("dataDimensions", "Dimensions")

    , outputGroup_("outputGroup", "Operations", InvalidationLevel::Valid)
    , overrideRange_("overrideRange", "Use Range", {{"data", "Data", 0}, {"custom", "Custom", 1}},
                     0)
    , outDataRange_("dataRange", "Custom Data Range", 0., 1.0, -DataFloat64::max(),
                    DataFloat64::max(), 0.0, 0.0, InvalidationLevel::InvalidOutput,
                    PropertySemantics("Text"))
    , valueRange_("valueRange", "Value range", 0., 1.0, -DataFloat64::max(), DataFloat64::max(),
                  0.0, 0.0, InvalidationLevel::InvalidOutput, PropertySemantics("Text"))
    , valueUnit_("valueUnit", "Value unit", "arb. unit.")

    , datatype_("convertType", "Convert to type",
                {{"none", "No conversion", 0},
                 {"float", "Float", 0},
                 {"double", "Double", 1},
                 {"uchar", "Unsigned Char", 2},
                 {"ushort", "Unsigned Short", 3}},
                0)
    , selection_("selection", "Selection", 6)
    , dirty_(false) {

    addPort(inport_);
    addPort(outport_);
    inport_.onChange([this]() { onDataChange(); });

    addProperty(volumeSelection_);
    volumeSelection_.onChange([this]() { onSelectionChange(); });
    volumeSelection_.setSerializationMode(PropertySerializationMode::All);

    addProperty(automaticEvaluation_);
    automaticEvaluation_.onChange([this]() { evaluate_.setReadOnly(automaticEvaluation_); });
    addProperty(evaluate_);
    evaluate_.onChange([this]() { dirty_ = true; });

    basisGroup_.addProperty(basisSelection_);
    basisGroup_.addProperty(spacing_);
    basisGroup_.addProperty(basis_);
    addProperty(basisGroup_);
    basisSelection_.onChange([this]() { onBasisSelecionChange(); });
    basisSelection_.setSerializationMode(PropertySerializationMode::All);

    dataDimensions_.setReadOnly(true);
    dataRange_.setReadOnly(true);
    information_.addProperty(dataDimensions_);
    information_.addProperty(dataRange_);
    addProperty(information_);

    outputGroup_.addProperty(datatype_);
    outputGroup_.addProperty(overrideRange_);

    outputGroup_.addProperty(outDataRange_);
    outputGroup_.addProperty(valueRange_);
    outputGroup_.addProperty(valueUnit_);
    outputGroup_.addProperty(selection_);

    outputGroup_.onChange([this]() {
        if (automaticEvaluation_) {
            dirty_ = true;
            this->invalidate(InvalidationLevel::InvalidOutput);
        }
    });

    addProperty(outputGroup_);
}

HDF5ToVolume::~HDF5ToVolume() = default;

void HDF5ToVolume::process() {
    if (dirty_) {
        dirty_ = false;
        makeVolume();
    }

    switch (basisSelection_.getSelectedIndex()) {
        case 0: {  // User defined basis
            break;
        }
        case 1: {  // User defined spacing
            const auto dim = volume_->getDimensions();
            const vec4 diag(vec3(dim) * spacing_.get(), 1.0f);
            mat4 basis = glm::diagonal4x4(diag);
            vec3 offset = -0.5f * vec3(basis[0] + basis[1] + basis[2]);
            basis[3] = vec4(offset, 1.0f);
            basis_.set(basis);
            break;
        }
        default: {
            mat4 basis = getBasisFromMeta(basisMatches_[basisSelection_.getSelectedIndex() - 2]);
            basis_.set(basis);
            break;
        }
    }

    if (selection_.adjustBasis_) {
        mat4 basis = basis_;

        auto sel = selection_.getSelection();
        auto maxSel = selection_.getMaxSelection();

        int j = 0;
        for (size_t i = 0; i < sel.size(); ++i) {
            if (sel[i].end - sel[i].start <= 1) continue;
            if (j > 2) throw Exception("Invalid selection, resulting rank > 3", IVW_CONTEXT);

            basis[j] *= static_cast<float>(sel[i].end - sel[i].start) /
                        static_cast<float>(maxSel[i].end - maxSel[i].start);
            ++j;
        }

        vec3 offset = -0.5f * vec3(basis[0] + basis[1] + basis[2]);
        basis[3] = vec4(offset, 1.0f);
        volume_->setModelMatrix(basis);
    } else {
        volume_->setModelMatrix(basis_);
    }
    switch (overrideRange_.getSelectedIndex()) {
        case 0:  // Data
            volume_->dataMap_.dataRange = dataRange_.get();
            break;
        case 1:  // Custom
            volume_->dataMap_.dataRange = outDataRange_.get();
            break;
        default:
            break;
    }
    volume_->dataMap_.valueRange = valueRange_.get();
    volume_->dataMap_.valueUnit = valueUnit_.get();
}

mat4 HDF5ToVolume::getBasisFromMeta(MetaData meta) {
    mat4 basis(1.0f);

    if (inport_.hasData()) {
        const auto data = inport_.getData();
        H5::DataSet dataset = data->getGroup().openDataSet(meta.path_);
        H5::DataSpace space = dataset.getSpace();
        int rank = space.getSimpleExtentNdims();
        if (rank != 2)
            throw DataReaderException(
                "Could not create Basis from: " + meta.path_.toString() + "Invalid rank",
                IVW_CONTEXT);
        std::vector<hsize_t> dims(rank);
        space.getSimpleExtentDims(dims.data());
        if (dims[0] == 3 && dims[1] == 3) {
            mat3 bas;
            dataset.read(glm::value_ptr(bas), H5::PredType::NATIVE_FLOAT);

            vec3 offset = -0.5f * vec3(bas[0] + bas[1] + bas[2]);
            basis[0] = vec4(bas[0], 0.0f);
            basis[1] = vec4(bas[1], 0.0f);
            basis[2] = vec4(bas[2], 0.0f);
            basis[3] = vec4(offset, 1.0f);

        } else if (dims[0] == 4 && dims[1] == 4) {
            dataset.read(glm::value_ptr(basis), H5::PredType::NATIVE_FLOAT);
        } else {
            throw DataReaderException(
                "Could not create Basis from: " + meta.path_.toString() + "Invalid dimensions",
                IVW_CONTEXT);
        }
    }
    return basis;
}

void HDF5ToVolume::onDataChange() {
    if (inport_.hasData()) {
        const auto data = inport_.getData();

        std::vector<MetaData> metadata = util::getMetaData(data->getGroup());

        volumeMatches_.clear();
        std::copy_if(metadata.begin(), metadata.end(), std::back_inserter(volumeMatches_),
                     [](const MetaData& meta) {
                         auto dims = meta.getColumnMajorDimensions();
                         return meta.type_ == MetaData::HDFType::DataSet && dims.size() >= 3ull &&
                                std::accumulate(dims.begin(), dims.end(), 1ull,
                                                std::multiplies<size_t>()) > 50000ull;
                     });

        basisMatches_.clear();
        std::copy_if(metadata.begin(), metadata.end(), std::back_inserter(basisMatches_),
                     [](const MetaData& meta) {
                         auto dims = meta.getColumnMajorDimensions();
                         static const std::vector<size_t> basis{3, 3};
                         static const std::vector<size_t> basisAndOffset{4, 4};
                         return meta.type_ == MetaData::HDFType::DataSet &&
                                (dims == basis || dims == basisAndOffset);
                     });

        // Update Volume Selection
        std::vector<OptionPropertyStringOption> volumeOptions;
        for (const auto& meta : volumeMatches_) {
            volumeOptions.emplace_back(meta.path_, getDescription(meta), meta.path_);
        }
        volumeSelection_.replaceOptions(volumeOptions);
        volumeSelection_.setCurrentStateAsDefault();

        // Update Basis Selection

        std::vector<OptionPropertyStringOption> basisOptions;
        basisOptions.emplace_back("default", "User defined basis", "default");
        basisOptions.emplace_back("default", "User defined spacing", "default");
        for (const auto& meta : basisMatches_) {
            basisOptions.emplace_back(meta.path_, getDescription(meta), meta.path_);
        }
        basisSelection_.replaceOptions(basisOptions);
        basisSelection_.setCurrentStateAsDefault();

    } else {
        basisSelection_.clearOptions();
        volumeSelection_.clearOptions();
    }
}

std::string HDF5ToVolume::getDescription(const MetaData& meta) {
    return meta.path_.toString() +
           (meta.format_ ? (" " + std::string(meta.format_->getString())) : "") + " [" +
           joinString(meta.getColumnMajorDimensions(), ", ") + "]";
}

void HDF5ToVolume::onBasisSelecionChange() {
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
        MetaData volumeMeta = volumeMatches_[volumeSelection_.getSelectedIndex()];
        dataDimensions_.set("[" + joinString(volumeMeta.getColumnMajorDimensions(), ", ") + "]");
        selection_.update(volumeMeta);
    }
}

void HDF5ToVolume::makeVolume() {
    if (inport_.hasData()) {
        const auto data = inport_.getData();
        MetaData volumeMeta = volumeMatches_[volumeSelection_.getSelectedIndex()];

        try {
            const DataFormatBase* format = nullptr;

            switch (datatype_.getSelectedIndex()) {
                case 1:
                    format = DataFloat32::get();
                    break;
                case 2:
                    format = DataFloat64::get();
                    break;
                case 3:
                    format = DataUInt8::get();
                    break;
                case 4:
                    format = DataUInt16::get();
                    break;
                default:
                    break;
            }

            volume_ = std::shared_ptr<Volume>(
                data->getVolumeAtPathAsType(Path(data->getGroup().getObjName()) + volumeMeta.path_,
                                            selection_.getSelection(), format));

            dataRange_.set(volume_->dataMap_.dataRange);

            outport_.setData(volume_);

        } catch (const H5::GroupIException& e) {
            LogInfo(e.getDetailMsg());

            return;
        }
    }
}

HDF5ToVolume::DimSelection::DimSelection(std::string identifier, std::string displayName,
                                         InvalidationLevel level)
    : CompositeProperty(identifier, displayName, level, PropertySemantics::Default)
    , range("range", "Range", 0, 255, 0, 255, 1, 1)
    , stride("stride", "Stride", 1, 1, 255) {

    addProperty(range);
    addProperty(stride);
}

void HDF5ToVolume::DimSelection::update(int newMax) {
    range.setRangeMax(newMax);
    range.get().y = newMax;
    stride.set(std::min(stride.get(), newMax));
    stride.setMaxValue(std::max(10, newMax));
}

HDF5ToVolume::DimSelections::DimSelections(std::string identifier, std::string displayName,
                                           size_t maxRank, InvalidationLevel level)
    : CompositeProperty(identifier, displayName, level, PropertySemantics::Default)
    , adjustBasis_("adjustBasis", "Automatically adjust basis", true)
    , maxRank_(maxRank)
    , rank_(maxRank) {

    addProperty(adjustBasis_);

    char last = 'Z';
    for (size_t i = 0; i < maxRank_; ++i) {
        std::string ind = toString(static_cast<char>(last - maxRank + i + 1));
        selection_.push_back(std::make_unique<DimSelection>("dim" + ind, ind));
        addProperty(selection_[i].get(), false);
    }
}

void HDF5ToVolume::DimSelections::update(const MetaData& meta) {
    auto cmdims = meta.getColumnMajorDimensions();
    rank_ = cmdims.size();
    for (size_t i = 0; i < maxRank_ - rank_; ++i) {
        selection_[i]->setVisible(false);
    }
    for (size_t i = 0; i < rank_; ++i) {
        selection_[i + maxRank_ - rank_]->update(static_cast<int>(cmdims[i]));
    }
}

std::vector<Handle::Selection> HDF5ToVolume::DimSelections::getSelection() const {
    std::vector<Handle::Selection> selection;
    for (size_t i = maxRank_ - rank_; i < maxRank_; i++) {
        selection.emplace_back(selection_[i]->range.get().x, selection_[i]->range.get().y,
                               selection_[i]->stride.get());
    }
    return selection;
}

std::vector<Handle::Selection> HDF5ToVolume::DimSelections::getMaxSelection() const {
    std::vector<Handle::Selection> selection;
    for (size_t i = maxRank_ - rank_; i < maxRank_; i++) {
        selection.emplace_back(selection_[i]->range.getRangeMin(),
                               selection_[i]->range.getRangeMax(),
                               selection_[i]->stride.getMinValue());
    }
    return selection;
}

}  // namespace hdf5

}  // namespace inviwo
