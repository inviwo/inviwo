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

#include <inviwo/dataframe/processors/pathsequencetodataframe.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo PathSequenceToDataFrame::processorInfo_{
    "org.inviwo.PathSequenceToDataFrame",  // Class identifier
    "Path Sequence To Data Frame",         // Display name
    "DataFrame",                           // Category
    CodeState::Stable,                     // Code state
    Tags::CPU,                             // Tags
    R"(Convert a PathSeqeunce to a DataFrame)"_unindentHelp,
};

const ProcessorInfo& PathSequenceToDataFrame::getProcessorInfo() const { return processorInfo_; }

PathSequenceToDataFrame::PathSequenceToDataFrame()
    : Processor{}
    , inport_{"inport"}
    , outport_{"outport"}
    , bnlInport_{"bnlInport"}
    , bnlOutport_{"bnlOutport"}
    , selection_{"selection"} {

    addPorts(inport_, outport_, bnlInport_, bnlOutport_, selection_);

    bnlOutport_.getManager().setParent(&bnlInport_.getManager());
    bnlOutport_.getManager().onBrush([this](BrushingAction action, const BrushingTarget& target,
                                            const BitSet& indices,
                                            [[maybe_unused]] std::string_view source) {
        if (action == BrushingAction::Select && target == BrushingTarget::Row) {
            auto sel = std::make_shared<DataSequence<std::filesystem::path>>(
                indices | std::views::filter([this](auto i) { return i < paths_->size(); }) |
                std::views::transform([this](auto i) { return (*paths_)[i]; }));
            selection_.setData(sel);
        }
    });
}

void PathSequenceToDataFrame::process() {
    paths_ = inport_.getData();
    auto df = std::make_shared<DataFrame>();

    df->addCategoricalColumn("Name", *paths_ | std::views::transform([](const auto& p) {
                                 return p->filename().generic_string();
                             }) | std::ranges::to<std::vector>());

    df->addColumn("Size", *paths_ | std::views::transform([](const auto& p) {
                      return static_cast<int>(std::filesystem::file_size(*p));
                  }) | std::ranges::to<std::vector>());

    df->addCategoricalColumn("Modified", *paths_ | std::views::transform([](const auto& p) {
                                 return fmt::format("{}", std::filesystem::last_write_time(*p));
                             }) | std::ranges::to<std::vector>());

    df->addCategoricalColumn("Directory", *paths_ | std::views::transform([](const auto& p) {
                                 return p->parent_path().generic_string();
                             }) | std::ranges::to<std::vector>());

    df->updateIndexBuffer();

    outport_.setData(df);

    auto sel = std::make_shared<DataSequence<std::filesystem::path>>(
        bnlInport_.getIndices(BrushingAction::Select, BrushingTarget::Row) |
        std::views::filter([this](auto i) { return i < paths_->size(); }) |
        std::views::transform([this](auto i) { return (*paths_)[i]; }));
    selection_.setData(sel);
}

}  // namespace inviwo
