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

#include <modules/hdf5/processors/hdf5buffersource.h>
#include <modules/hdf5/datastructures/hdf5handle.h>
#include <modules/hdf5/hdf5read.h>
#include <modules/hdf5/datastructures/hdf5path.h>
#include <modules/hdf5/hdf5utils.h>
#include <inviwo/core/network/networklock.h>

#include <algorithm>
#include <numeric>

namespace inviwo::hdf5 {

const ProcessorInfo HDF5ToBuffer::processorInfo_{
    "org.inviwo.hdf5.ToBuffer",  // Class identifier
    "HDF5 To Buffer",            // Display name
    "Data Input",                // Category
    CodeState::Stable,           // Code state
    Tags::CPU | Tag{"HDF5"},     // Tags
    "Load a buffer from an HDF5 file handle."_help,
};
const ProcessorInfo& HDF5ToBuffer::getProcessorInfo() const { return processorInfo_; }

HDF5ToBuffer::HDF5ToBuffer()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , bufferSelection_("bufferSelection", "Dataset")
    , outputGroup_("outputGroup", "Operations")
    , datatype_("convertType", "Convert to type", util::conversionOptions(), 0)
    , selection_("selection", "Selection", 4) {

    addPort(inport_);
    addPort(outport_);

    bufferSelection_.onChange([this]() { onSelectionChange(); });
    bufferSelection_.setSerializationMode(PropertySerializationMode::All);

    outputGroup_.addProperties(datatype_, selection_);
    addProperties(bufferSelection_, outputGroup_);
}

HDF5ToBuffer::~HDF5ToBuffer() = default;

void HDF5ToBuffer::process() try {
    const auto data = inport_.getData();

    if (inport_.isChanged()) {
        bufferMatches_ = util::getDataSets(*data);

        std::vector<OptionPropertyStringOption> bufferOptions;
        for (const auto& info : bufferMatches_) {
            bufferOptions.emplace_back(info.path.toString(), util::dataSetDescription(info),
                                       info.path.toString());
        }
        bufferSelection_.replaceOptions(bufferOptions);
        bufferSelection_.setCurrentStateAsDefault();
    }

    onSelectionChange();

    const DataSetInfo bufferMeta = bufferMatches_[bufferSelection_.getSelectedIndex()];

    auto buffer = getBufferAtPathAsType(*data + bufferMeta.path, selection_.getSelection(),
                                        util::conversionFormat(datatype_.getSelectedIndex()));

    outport_.setData(buffer);

} catch (H5::Exception& e) {
    throw Exception(SourceContext{}, "Error reading HDF5 data: {}", e.getDetailMsg());
}

void HDF5ToBuffer::onSelectionChange() {
    if (!bufferMatches_.empty()) {
        const DataSetInfo bufferMeta = bufferMatches_[bufferSelection_.getSelectedIndex()];
        selection_.update(bufferMeta);
    }
}

}  // namespace inviwo::hdf5
