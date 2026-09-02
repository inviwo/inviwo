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
#include <modules/hdf5/hdf5read.h>
#include <modules/hdf5/datastructures/hdf5path.h>
#include <modules/hdf5/hdf5utils.h>
#include <inviwo/core/network/networklock.h>

#include <algorithm>
#include <numeric>

namespace inviwo::hdf5 {

const ProcessorInfo HDF5ToLayer::processorInfo_{
    "org.inviwo.hdf5.ToLayer",               // Class identifier
    "HDF5 To Layer",                         // Display name
    "Data Input",                            // Category
    CodeState::Stable,                       // Code state
    Tags::CPU | Tag{"HDF5"} | Tag{"Layer"},  // Tags
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
    , datatype_("convertType", "Convert to type", util::conversionOptions(), 0)
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
        std::vector<DataSetInfo> metadata = util::getDataSets(*data);

        layerMatches_.clear();
        std::ranges::copy_if(
            metadata, std::back_inserter(layerMatches_), [](const DataSetInfo& meta) {
                auto dims = meta.getColumnMajorDimensions();
                return dims.size() >= 2ull &&
                       std::ranges::fold_left(dims, size_t{1}, std::multiplies{}) > 0ull;
            });

        std::vector<OptionPropertyStringOption> layerOptions;
        for (const auto& meta : layerMatches_) {
            const auto path = meta.path_.toString();
            layerOptions.emplace_back(path, util::dataSetDescription(meta), path);
        }
        layerSelection_.replaceOptions(layerOptions);
        layerSelection_.setCurrentStateAsDefault();
    } else {
        layerSelection_.clearOptions();
    }

    onSelectionChange();
}

void HDF5ToLayer::onSelectionChange() {
    dirty_ = true;
    if (!layerMatches_.empty()) {
        constDataSetInfo layerMeta = layerMatches_[layerSelection_.getSelectedIndex()];
        selection_.update(layerMeta);
    }
}

void HDF5ToLayer::makeLayer() {
    if (inport_.hasData()) {
        const auto data = inport_.getData();
        const DataSetInfo layerMeta = layerMatches_[layerSelection_.getSelectedIndex()];

        layer_ = getLayerAtPathAsType(*data + layerMeta.path_, selection_.getSelection(),
                                      util::conversionFormat(datatype_.getSelectedIndex()));
    }
}

void HDF5ToLayer::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    deserialized_ = true;
}

}  // namespace inviwo::hdf5
