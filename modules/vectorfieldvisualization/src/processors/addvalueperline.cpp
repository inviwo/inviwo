/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/processors/addvalueperline.h>
#include <inviwo/core/util/zip.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo AddValuePerLine::processorInfo_{
    "org.inviwo.AddValuePerLine",  // Class identifier
    "Add Value Per Line",          // Display name
    "Undefined",                   // Category
    CodeState::Experimental,       // Code state
    Tags::None,                    // Tags
};
const ProcessorInfo AddValuePerLine::getProcessorInfo() const { return processorInfo_; }

AddValuePerLine::AddValuePerLine()
    : Processor()
    , linesIn_("linesIn")
    , linesOut_("linesOut")
    , dataIn_("dataIn")
    , valueName_("valueName_", "Value name", "meta") {

    addPorts(linesIn_, dataIn_, linesOut_);
    addProperty(valueName_);
}

void AddValuePerLine::process() {
    if (!linesIn_.hasData() || !dataIn_.hasData()) {
        linesOut_.clear();
        return;
    }

    auto column = dataIn_.getData()->getColumn(1);
    size_t dataSize = column->getSize();
    size_t lineSize = linesIn_.getData()->size();
    if (dataSize != lineSize) {
        LogError(fmt::format("Got {} data points, but {} lines. Aborting.", dataSize, lineSize));
        linesOut_.clear();
        return;
    }

    auto lineSet = std::make_shared<IntegralLineSet>(mat4(1.0));
    // for (auto&& sample : util::zip(linesIn_.getData()->getVector(), column->get)) {
    for (size_t l = 0; l < lineSize; ++l) {
        double value = column->getAsDouble(l);
        IntegralLine line = linesIn_.getData()->at(l);

        auto& valueData = line.getMetaData<double>(valueName_.get(), true);
        valueData.resize(line.getPositions().size());
        std::fill(valueData.begin(), valueData.end(), value);

        lineSet->push_back(std::move(line), lineSet->size());
        // std::cout << "Value: " << value << std::endl;
    }
    linesOut_.setData(lineSet);
}

}  // namespace inviwo
