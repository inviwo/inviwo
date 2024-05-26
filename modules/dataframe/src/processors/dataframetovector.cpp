/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <inviwo/dataframe/processors/dataframetovector.h>

#include <type_traits>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DataFrameToVector::processorInfo_{
    "org.inviwo.DataFrameToVector",  // Class identifier
    "Data Frame To Vector",          // Display name
    "Undefined",                     // Category
    CodeState::Experimental,         // Code state
    Tags::None,                      // Tags
    R"(<Explanation of how to use the processor.>)"_unindentHelp};

const ProcessorInfo DataFrameToVector::getProcessorInfo() const { return processorInfo_; }

DataFrameToVector::DataFrameToVector()
    : Processor{}
    , dataFrame_{"dataFrame", ""_help}
    , uintOutport_{"uintOutport", "<description of the generated outport data>"_help}
    , floatOutport_{"floatOutport", "<description of the generated outport data>"_help}
    , stringOutport_{"stringOutport", "<description of the generated outport data>"_help}
    , selectedColumn_("selectedColumn", "Selected Column", dataFrame_) {

    addPorts(dataFrame_, uintOutport_, floatOutport_, stringOutport_);
    addProperties(selectedColumn_);
}

void DataFrameToVector::process() {
    const auto col = dataFrame_.getData()->getColumn(selectedColumn_.getSelectedValue());
    if (const auto* cat = dynamic_cast<const CategoricalColumn*>(col.get())) {
        stringOutport_.setData(
            std::make_shared<std::vector<std::string>>(cat->values().begin(), cat->values().end()));
        return;
    }

    col->getBuffer()->getRepresentation<BufferRAM>()->dispatch<void, dispatching::filter::Scalars>(
        [&]<typename T, BufferTarget Target>(const BufferRAMPrecision<T, Target>* buf) {
            if constexpr (std::is_same_v<T, uint32_t>) {
                uintOutport_.setData(std::make_shared<std::vector<T>>(buf->getDataContainer()));
            } else if constexpr (std::is_same_v<T, float>) {
                floatOutport_.setData(std::make_shared<std::vector<T>>(buf->getDataContainer()));
            } else {
                throw Exception("Column type not supported");
            }
        });
}

}  // namespace inviwo
