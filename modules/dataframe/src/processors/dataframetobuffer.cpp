/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <inviwo/dataframe/processors/dataframetobuffer.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DataFrameToBuffer::processorInfo_{
    "org.inviwo.DataFrameToBuffer",  // Class identifier
    "Data Frame To Buffer",          // Display name
    "DataFrame",                     // Category
    CodeState::Stable,               // Code state
    Tags::CPU,                       // Tags
    R"(Extract a column from a dataframe)"_unindentHelp};

const ProcessorInfo& DataFrameToBuffer::getProcessorInfo() const { return processorInfo_; }

DataFrameToBuffer::DataFrameToBuffer()
    : Processor{}
    , dataFrame_{"dataFrame", ""_help}
    , outport_{"outport", "column buffer"_help}
    , selectedColumn_{"selectedColumn", "Selected Column", dataFrame_} {

    addPorts(dataFrame_, outport_);
    addProperties(selectedColumn_);
}

void DataFrameToBuffer::process() {
    const auto col = dataFrame_.getData()->getColumn(selectedColumn_.getSelectedValue());
    outport_.setData(col->getBuffer());
}

}  // namespace inviwo
