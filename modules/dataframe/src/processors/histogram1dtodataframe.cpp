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

#include <inviwo/dataframe/processors/histogram1dtodataframe.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo Histogram1DToDataFrame::processorInfo_{
    "org.inviwo.Histogram1DToDataFrame",  // Class identifier
    "Histogram1D To Data Frame",          // Display name
    "Data Creation",                      // Category
    CodeState::Experimental,              // Code state
    Tags::CPU,                            // Tags
    R"(Convert a histograms into a DataFrame)"_unindentHelp,
};

const ProcessorInfo& Histogram1DToDataFrame::getProcessorInfo() const { return processorInfo_; }

Histogram1DToDataFrame::Histogram1DToDataFrame()
    : Processor{}, inport_{"inport", ""_help}, outport_{"outport", ""_help} {

    addPorts(inport_, outport_);
}

void Histogram1DToDataFrame::process() {

    auto df = std::make_shared<DataFrame>();

    for (auto histogram : inport_) {
        df->addColumn(
            fmt::format("{} Data Range", histogram->name),
            std::views::iota(0uz, histogram->counts.size()) |
                std::views::transform([&](auto index) {
                    return static_cast<float>(
                        histogram->dataMap.dataRange.x +
                        static_cast<double>(index) / static_cast<double>(histogram->counts.size()) *
                            (histogram->dataMap.dataRange.y - histogram->dataMap.dataRange.x));
                }) |
                std::ranges::to<std::vector>());
        df->addColumn(
            fmt::format("{} {}", histogram->name, histogram->dataMap.valueAxis.name),
            std::views::iota(0uz, histogram->counts.size()) |
                std::views::transform([&](auto index) {
                    return static_cast<float>(
                        histogram->dataMap.valueRange.x +
                        static_cast<double>(index) / static_cast<double>(histogram->counts.size()) *
                            (histogram->dataMap.valueRange.y - histogram->dataMap.valueRange.x));
                }) |
                std::ranges::to<std::vector>(),
            histogram->dataMap.valueAxis.unit);
        df->addColumn(fmt::format("{} Counts", histogram->name),
                      histogram->counts | std::views::transform([](auto count) {
                          return static_cast<float>(count);
                      }) | std::ranges::to<std::vector>(),
                      Unit{}, dvec2{0.0, static_cast<double>(histogram->maxCount)});
        df->addColumn(fmt::format("{} Normalized Counts", histogram->name),
                      histogram->counts | std::views::transform([&](auto count) {
                          return static_cast<float>(static_cast<double>(count) /
                                                    static_cast<double>(histogram->maxCount));
                      }) | std::ranges::to<std::vector>(),
                      Unit{}, dvec2{0.0, 1.0});
    }

    df->updateIndexBuffer();

    outport_.setData(df);
}

}  // namespace inviwo
