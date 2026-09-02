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
    , evaluate_("evaluate", "Load", [this]() { dirty_ = true; })
    , information_("Information", "Data information")
    , outputGroup_("outputGroup", "Operations", InvalidationLevel::Valid)
    , datatype_("convertType", "Convert to type", util::conversionOptions(), 0)
    , selection_("selection", "Selection", 4)
    , dirty_(false) {

    addPort(inport_);
    addPort(outport_);

    layerSelection_.onChange([this]() { onSelectionChange(); });
    layerSelection_.setSerializationMode(PropertySerializationMode::All);

    automaticEvaluation_.onChange([this]() { evaluate_.setReadOnly(automaticEvaluation_); });

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
    const auto data = inport_.getData();
    if (inport_.isChanged()) {

        std::vector<DataSetInfo> metadata = util::getDataSets(*data);
        layerMatches_.assign_range(metadata | std::views::filter([](const DataSetInfo& info) {
                                       return info.dimensions.size() >= 2ull &&
                                              std::ranges::fold_left(info.dimensions, size_t{1},
                                                                     std::multiplies{}) > 100ull;
                                   }));

        std::vector<OptionPropertyStringOption> layerOptions;
        for (const auto& info : layerMatches_) {
            layerOptions.emplace_back(info.path.toString(), util::dataSetDescription(info),
                                      info.path.toString());
        }
        layerSelection_.replaceOptions(layerOptions);
        layerSelection_.setCurrentStateAsDefault();
    }

    onSelectionChange();

    if (dirty_) {
        dirty_ = false;
        const DataSetInfo layerMeta = layerMatches_[layerSelection_.getSelectedIndex()];

        layer_ = getLayerAtPathAsType(*data + layerMeta.path, selection_.getSelection(),
                                      util::conversionFormat(datatype_.getSelectedIndex()));
        information_.updateForNewLayer(*layer_, deserialized_ ? inviwo::util::OverwriteState::Yes
                                                              : inviwo::util::OverwriteState::No);
        deserialized_ = false;
    }

    if (layer_) {
        information_.updateLayer(*layer_);
        outport_.setData(layer_);
    }

} catch (H5::Exception& e) {
    throw Exception(SourceContext{}, "Error reading HDF5 data: {}", e.getDetailMsg());
}

void HDF5ToLayer::onSelectionChange() {
    dirty_ = true;
    if (!layerMatches_.empty()) {
        const DataSetInfo layerMeta = layerMatches_[layerSelection_.getSelectedIndex()];
        selection_.update(layerMeta);
    }
}

void HDF5ToLayer::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    deserialized_ = true;
}

}  // namespace inviwo::hdf5
