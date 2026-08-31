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

#include <modules/hdf5/processors/hdf5buffersource.h>
#include <modules/hdf5/datastructures/hdf5handle.h>
#include <modules/hdf5/hdf5read.h>
#include <modules/hdf5/datastructures/hdf5path.h>
#include <modules/hdf5/hdf5utils.h>
#include <inviwo/core/network/networklock.h>

#include <algorithm>
#include <numeric>

namespace inviwo {

namespace hdf5 {

const ProcessorInfo HDF5ToBuffer::processorInfo_{
    "org.inviwo.hdf5.ToBuffer",  // Class identifier
    "HDF5 To Buffer",            // Display name
    "Data Input",                // Category
    CodeState::Stable,           // Code state
    Tags::None,                  // Tags
    "Load a buffer from an HDF5 file handle."_help,
};
const ProcessorInfo& HDF5ToBuffer::getProcessorInfo() const { return processorInfo_; }

HDF5ToBuffer::HDF5ToBuffer()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , bufferSelection_("bufferSelection", "Dataset")
    , automaticEvaluation_("automaticEvaluation", "Automatic loading", true,
                           InvalidationLevel::Valid)
    , evaluate_("evaluate", "Load")
    , outputGroup_("outputGroup", "Operations", InvalidationLevel::Valid)
    , datatype_("convertType", "Convert to type", util::conversionOptions(), 0)
    , selection_("selection", "Selection", 4)
    , dirty_(false) {

    addPort(inport_);
    addPort(outport_);
    inport_.onChange([this]() { onDataChange(); });

    bufferSelection_.onChange([this]() { onSelectionChange(); });
    bufferSelection_.setSerializationMode(PropertySerializationMode::All);

    automaticEvaluation_.onChange([this]() { evaluate_.setReadOnly(automaticEvaluation_); });

    evaluate_.onChange([this]() { dirty_ = true; });

    outputGroup_.addProperties(datatype_, selection_);
    outputGroup_.onChange([this]() {
        if (automaticEvaluation_) {
            dirty_ = true;
            this->invalidate(InvalidationLevel::InvalidOutput);
        }
    });

    addProperties(bufferSelection_, automaticEvaluation_, evaluate_, outputGroup_);
}

HDF5ToBuffer::~HDF5ToBuffer() = default;

void HDF5ToBuffer::process() try {
    if (dirty_) {
        dirty_ = false;
        makeBuffer();
        deserialized_ = false;
    }

    if (buffer_) {
        outport_.setData(buffer_);
    }
} catch (H5::Exception& e) {
    throw Exception(SourceContext{}, "Error reading HDF5 data: {}", e.getDetailMsg());
}

void HDF5ToBuffer::onDataChange() {
    if (inport_.hasData()) {
        const auto data = inport_.getData();
        std::vector<DataSetInfo> metadata = util::getDataSets(*data);

        bufferMatches_.clear();
        std::ranges::copy_if(
            metadata, std::back_inserter(bufferMatches_), [](const DataSetInfo& meta) {
                auto dims = meta.getColumnMajorDimensions();
                return !dims.empty() &&
                       std::ranges::fold_left(dims, size_t{1}, std::multiplies{}) > 0ull;
            });

        std::vector<OptionPropertyStringOption> bufferOptions;
        for (const auto& meta : bufferMatches_) {
            const auto path = meta.path_.toString();
            bufferOptions.emplace_back(path, util::dataSetDescription(meta), path);
        }
        bufferSelection_.replaceOptions(bufferOptions);
        bufferSelection_.setCurrentStateAsDefault();
    } else {
        bufferSelection_.clearOptions();
    }

    onSelectionChange();
}

void HDF5ToBuffer::onSelectionChange() {
    dirty_ = true;
    if (!bufferMatches_.empty()) {
        DataSetInfo bufferMeta = bufferMatches_[bufferSelection_.getSelectedIndex()];
        selection_.update(bufferMeta);
    }
}

void HDF5ToBuffer::makeBuffer() {
    if (inport_.hasData()) {
        const auto data = inport_.getData();
        DataSetInfo bufferMeta = bufferMatches_[bufferSelection_.getSelectedIndex()];

        buffer_ = getBufferAtPathAsType(*data + bufferMeta.path_, selection_.getSelection(),
                                        util::conversionFormat(datatype_.getSelectedIndex()));
    }
}

void HDF5ToBuffer::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    deserialized_ = true;
}

}  // namespace hdf5

}  // namespace inviwo
