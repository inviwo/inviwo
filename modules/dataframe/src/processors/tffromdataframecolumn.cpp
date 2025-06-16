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
#include <inviwo/core/util/glmvec.h>
#include <modules/base/algorithm/tfconstruction.h>

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
    R"(Construct a transfer function with a peak at each selected column row value.
        The heights and width can be controlled with @p alpha and @p delta.
        The @p shift parameter can be used to offset the positions.)"_unindentHelp,
};

const ProcessorInfo& TFFromDataFrameColumn::getProcessorInfo() const { return processorInfo_; }

TFFromDataFrameColumn::TFFromDataFrameColumn()
    : Processor{}
    , dataFrame_{"dataFrame", ""_help}
    , bnl_{"brushingAndLinking"}
    , column_{"column", "Column"}
    , lock_{"lock", "Lock", false}
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
    addProperties(column_, lock_, tf_, alpha_, delta_, shift_, range_);

    tf_.setInvalidationLevel(InvalidationLevel::Valid);
}

void TFFromDataFrameColumn::process() {
    if (!lock_.get()) {
        auto df = dataFrame_.getData();
        column_.setOptions(*df);

        auto col = df->getColumn(column_.getSelectedValue());

        const auto range = col->getRange();
        range_.set(range);

        auto newPos =
            bnl_.getSelectedIndices() | std::views::transform([&](auto i) {
                return TFPrimitiveData{col->getAsDouble(i), glm::vec4{1, 1, 1, alpha_.get()}};
            });
        pos_.assign(std::begin(newPos), std::end(newPos));
    }

    auto data = util::tfSawTooth({
        .points = pos_,
        .range = range_.get(),
        .delta = delta_.get(),
        .shift = shift_.get(),
    });

    tf_.get().set(data);
}

}  // namespace inviwo
