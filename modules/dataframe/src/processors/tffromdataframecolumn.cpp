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

#include <inviwo/dataframe/processors/tffromdataframecolumn.h>

#include <algorithm>
#include <ranges>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TFFromDataFrameColumn::processorInfo_{
    "org.inviwo.TFFromDataFrameColumn",  // Class identifier
    "TF From DataFrame Column",          // Display name
    "DataFrame",                         // Category
    CodeState::Stable,                   // Code state
    Tags::CPU | Tag{"TF"},               // Tags
    R"(<Explanation of how to use the processor.>)"_unindentHelp,
};

const ProcessorInfo& TFFromDataFrameColumn::getProcessorInfo() const { return processorInfo_; }

TFFromDataFrameColumn::TFFromDataFrameColumn()
    : Processor{}
    , dataFrame_{"dataFrame", ""_help}
    , bnl_{"brushingAndLinking"}
    , column_{"column", "Column"}
    , tf_{"tf", "Transferfunction"}
    , alpha_{"alpha", "Alpha", util::ordinalLength(1.0, 1.0).setInc(0.001)}
    , delta_{"delta", "Delta", util::ordinalScale(0.01, 1.0).setInc(0.001)}
    , shift_{"shift", "Iso shift", util::ordinalSymmetricVector(0.0, 1.0).setInc(0.001)}
    , range_{"range",
             "Range",
             "Range of input data, before any transforms"_help,
             0.0,
             0.0,
             std::numeric_limits<double>::lowest(),
             std::numeric_limits<double>::max(),
             0.001,
             0.0,
             InvalidationLevel::Valid,
             PropertySemantics::Text} {

    addPorts(dataFrame_, bnl_);
    addProperties(column_, tf_, alpha_, delta_, shift_, range_);
}

void TFFromDataFrameColumn::process() {
    auto df = dataFrame_.getData();
    column_.setOptions(*df);

    auto col = df->getColumn(column_.getSelectedValue());

    const auto range = col->getRange();
    range_.set(range);
    const auto normalize = [&](double pos) { return (pos - range.x) / (range.y - range.x); };
    static constexpr auto clamp = [](double pos) { return std::clamp(pos, 0.0, 1.0); };

    auto shift = [s = shift_.get()](double pos) { return pos + s; };

    auto points = bnl_.getSelectedIndices() |
                  std::views::transform([&](auto i) { return col->getAsDouble(i); }) |
                  std::views::transform(shift) | std::views::transform(normalize) |
                  std::views::transform(clamp) | std::ranges::to<std::vector>();

    std::ranges::sort(points);
    auto dups = std::ranges::unique(points);
    points.erase(dups.begin(), dups.end());

    if (points.empty()) {
        tf_.get().clear();
    } else {
        std::vector<TFPrimitiveData> data;
        const auto delta = delta_.get();
        const auto low = vec4{0.0};
        const auto high = vec4{1.0, 1.0, 1.0, alpha_.get()};

        if (points.front() > 0.0) {
            const auto p0 = points.front() - delta;
            data.emplace_back(clamp(p0), glm::mix(low, high, (clamp(p0) - p0) / delta));
        }
        for (auto&& [a, b] : std::views::zip(points, points | std::views::drop(1))) {
            data.emplace_back(a, high);
            if (b - a > 2 * delta) {
                data.emplace_back(a + delta, low);
                data.emplace_back(b - delta, low);
            } else if (b - a > delta) {
                data.emplace_back((b + a) / 2.0, glm::mix(high, low, (b - a - delta) / delta));
            }
        }
        data.emplace_back(points.back(), high);

        if (points.back() < 1.0) {
            const auto pn = points.back() + delta;
            data.emplace_back(clamp(pn), glm::mix(low, high, (pn - clamp(pn)) / delta));
        }

        tf_.get().set(data);
    }
}

}  // namespace inviwo
