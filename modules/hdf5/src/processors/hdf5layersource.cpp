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

#include <modules/hdf5/processors/hdf5layersource.h>
#include <modules/hdf5/datastructures/hdf5handle.h>
#include <modules/hdf5/datastructures/hdf5path.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/zip.h>
#include <numeric>

namespace inviwo {

namespace hdf5 {

const ProcessorInfo HDF5ToLayer::processorInfo_{
    "org.inviwo.hdf5.ToLayer",  // Class identifier
    "HDF5 To Layer",            // Display name
    "Data Input",               // Category
    CodeState::Stable,          // Code state
    Tags::None,                 // Tags
    "Load a layer from an HDF5 file handle."_help,
};
const ProcessorInfo& HDF5ToLayer::getProcessorInfo() const { return processorInfo_; }

HDF5ToLayer::HDF5ToLayer()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , layerSelection_("layerSelection", "Layer")
    , automaticEvaluation_("automaticEvaluation", "Automatic loading", true,
                           InvalidationLevel::Valid)
    , evaluate_("evaluate", "Load")
    , information_("Information", "Data information")
    , outputGroup_("outputGroup", "Operations", InvalidationLevel::Valid)
    , datatype_("convertType", "Convert to type",
                {{"none", "No conversion", 0},
                 {"float", "Float", 0},
                 {"double", "Double", 1},
                 {"uchar", "Unsigned Char", 2},
                 {"ushort", "Unsigned Short", 3}},
                0)
    , selection_("selection", "Selection", 4)
    , dirty_(false) {

    addPort(inport_);
    addPort(outport_);
    inport_.onChange([this]() { onDataChange(); });

    layerSelection_.onChange([this]() { onSelectionChange(); });
    layerSelection_.setSerializationMode(PropertySerializationMode::All);

    automaticEvaluation_.onChange([this]() { evaluate_.setReadOnly(automaticEvaluation_); });

    evaluate_.onChange([this]() { dirty_ = true; });

    outputGroup_.addProperties(datatype_, selection_);
    outputGroup_.onChange([this]() {
        if (automaticEvaluation_) {
            dirty_ = true;
            this->invalidate(InvalidationLevel::InvalidOutput);
        }
    });

    addProperties(layerSelection_, automaticEvaluation_, evaluate_, information_, outputGroup_);
}

HDF5ToLayer::~HDF5ToLayer() = default;

void HDF5ToLayer::process() try {
    if (dirty_) {
        dirty_ = false;
        makeLayer();
        deserialized_ = false;
    }

    if (layer_) {
        information_.updateForNewLayer(*layer_, deserialized_ ? inviwo::util::OverwriteState::Yes
                                                              : inviwo::util::OverwriteState::No);
        information_.updateLayer(*layer_);
        outport_.setData(layer_);
    }
} catch (H5::Exception& e) {
    throw Exception(SourceContext{}, "Error reading HDF5 data: {}", e.getDetailMsg());
}

void HDF5ToLayer::onDataChange() {
    if (inport_.hasData()) {
        const auto data = inport_.getData();
        std::vector<MetaData> metadata = util::getMetaData(data->getGroup());

        layerMatches_.clear();
        std::copy_if(metadata.begin(), metadata.end(), std::back_inserter(layerMatches_),
                     [](const MetaData& meta) {
                         auto dims = meta.getColumnMajorDimensions();
                         return meta.type_ == MetaData::HDFType::DataSet && dims.size() >= 2ull &&
                                std::accumulate(dims.begin(), dims.end(), 1ull,
                                                std::multiplies<size_t>()) > 0ull;
                     });

        std::vector<OptionPropertyStringOption> layerOptions;
        for (const auto& meta : layerMatches_) {
            const auto path = meta.path_.toString();
            layerOptions.emplace_back(path, getDescription(meta), path);
        }
        layerSelection_.replaceOptions(layerOptions);
        layerSelection_.setCurrentStateAsDefault();
    } else {
        layerSelection_.clearOptions();
    }

    onSelectionChange();
}

std::string HDF5ToLayer::getDescription(const MetaData& meta) {
    return meta.path_.toString() +
           (meta.format_ ? (" " + std::string(meta.format_->getString())) : "") + " [" +
           joinString(meta.getColumnMajorDimensions(), ", ") + "]";
}

void HDF5ToLayer::onSelectionChange() {
    dirty_ = true;
    if (!layerMatches_.empty()) {
        MetaData layerMeta = layerMatches_[layerSelection_.getSelectedIndex()];
        selection_.update(layerMeta);
    }
}

void HDF5ToLayer::makeLayer() {
    if (inport_.hasData()) {
        const auto data = inport_.getData();
        MetaData layerMeta = layerMatches_[layerSelection_.getSelectedIndex()];

        auto format = [&]() -> const DataFormatBase* {
            switch (datatype_.getSelectedIndex()) {
                case 1:
                    return DataFloat32::get();
                case 2:
                    return DataFloat64::get();
                case 3:
                    return DataUInt8::get();
                case 4:
                    return DataUInt16::get();
                default:
                    return nullptr;
            }
        }();

        layer_ = data->getLayerAtPathAsType(
            Path(data->getGroup().getObjName()) + layerMeta.path_, selection_.getSelection(),
            format);
    }
}

void HDF5ToLayer::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    deserialized_ = true;
}

// DimSelection -----------------------------------------------------------------------

HDF5ToLayer::DimSelection::DimSelection(const std::string& identifier,
                                        const std::string& displayName, InvalidationLevel level)
    : CompositeProperty(identifier, displayName, level, PropertySemantics::Default)
    , range("range", "Range", 0, 255, 0, 255, 1, 1)
    , stride("stride", "Stride", 1, 1, 255) {

    addProperty(range);
    addProperty(stride);
}

void HDF5ToLayer::DimSelection::update(int newMax) {
    range.setRangeMax(newMax);
    range.setEnd(std::min(range.getEnd(), newMax));
    stride.set(std::min(stride.get(), newMax));
    stride.setMaxValue(std::max(10, newMax));
}

// DimSelections ----------------------------------------------------------------------

HDF5ToLayer::DimSelections::DimSelections(const std::string& identifier,
                                          const std::string& displayName, size_t maxRank,
                                          InvalidationLevel level)
    : CompositeProperty(identifier, displayName, level, PropertySemantics::Default)
    , maxRank_(maxRank)
    , rank_(maxRank) {

    char last = 'Z';
    for (size_t i = 0; i < maxRank_; ++i) {
        const auto ind = fmt::to_string(static_cast<char>(last - maxRank + i + 1));
        selection_.push_back(std::make_unique<DimSelection>("dim" + ind, ind));
        addProperty(selection_[i].get(), false);
    }
}

void HDF5ToLayer::DimSelections::update(const MetaData& meta) {
    const NetworkLock lock{this};
    auto cmdims = meta.getColumnMajorDimensions();
    rank_ = cmdims.size();

    for (auto&& [index, selection] : inviwo::util::enumerate(selection_)) {
        selection->setVisible(index >= maxRank_ - rank_);
    }

    for (size_t i = 0; i < rank_; ++i) {
        selection_[i + maxRank_ - rank_]->update(static_cast<int>(cmdims[i]));
    }
}

std::vector<Handle::Selection> HDF5ToLayer::DimSelections::getSelection() const {
    std::vector<Handle::Selection> selection;
    for (size_t i = maxRank_ - rank_; i < maxRank_; i++) {
        selection.emplace_back(selection_[i]->range.get().x, selection_[i]->range.get().y,
                               selection_[i]->stride.get());
    }
    return selection;
}

std::vector<Handle::Selection> HDF5ToLayer::DimSelections::getMaxSelection() const {
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
