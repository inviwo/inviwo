/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/processors/discardshortlines.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DiscardShortLines::processorInfo_{
    "org.inviwo.DiscardShortLines",  // Class identifier
    "Discard Short Lines",           // Display name
    "Integral Lines",                // Category
    CodeState::Stable,               // Code state
    Tags::CPU,                       // Tags
};
const ProcessorInfo DiscardShortLines::getProcessorInfo() const { return processorInfo_; }

DiscardShortLines::DiscardShortLines()
    : Processor()
    , linesIn_("linesIn")
    , linesOut_("linesOut")
    , removedLines_("removedLines")

    , minLength_("minLength", "Min Length", 0.1, 0.0, 1.0, 0.01)
    , dataMinLength_("dataMinLength", "Min Length (data)", 0, 0, std::numeric_limits<double>::max(),
                     0.01, InvalidationLevel::Valid, PropertySemantics::Text)
    , dataMaxLength_("dataMaxLength", "Max Length (data)", 1, 0, std::numeric_limits<double>::max(),
                     0.01, InvalidationLevel::Valid, PropertySemantics::Text)

{
    addPort(linesIn_);
    addPort(linesOut_);
    addPort(removedLines_);

    addProperty(minLength_);
    addProperty(dataMinLength_);
    addProperty(dataMaxLength_);

    dataMinLength_.setReadOnly(true);
    dataMaxLength_.setReadOnly(true);

    linesIn_.onChange([&]() {
        if (!linesIn_.hasData()) return;
        auto linesPtr = linesIn_.getData();
        auto &lines = *linesPtr;
        if (lines.size() == 0) return;

        double maxL = 0;
        double minL = std::numeric_limits<double>::max();
        for (const auto &line : lines) {
            auto l = line.getLength();
            maxL = std::max(maxL, l);
            minL = std::min(minL, l);
        }

        dataMinLength_.set(minL);
        dataMaxLength_.set(maxL);

        auto t = (minLength_.get() - minLength_.getMinValue()) /
                 (minLength_.getMaxValue() - minLength_.getMinValue());
        minLength_.setMinValue(minL);
        minLength_.setMaxValue(maxL);
        minLength_.set(minL + t * (maxL - minL));
    });
}

void DiscardShortLines::process() {

    auto linesData = linesIn_.getData();
    auto &lines = *linesData;

    auto outLinesData = std::make_shared<IntegralLineSet>(lines.getModelMatrix());
    auto filteredLinesData = std::make_shared<IntegralLineSet>(lines.getModelMatrix());
    auto &outLines = *outLinesData;
    auto &filteredLines = *filteredLinesData;

    for (const auto &line : lines) {
        bool keep = line.getLength() >= minLength_.get();

        if (keep) {
            outLines.push_back(line, IntegralLineSet::SetIndex::No);
        } else {
            filteredLines.push_back(line, IntegralLineSet::SetIndex::No);
        }
    }

    linesOut_.setData(outLinesData);
    removedLines_.setData(filteredLinesData);
}

}  // namespace inviwo
