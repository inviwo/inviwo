/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2021 Inviwo Foundation
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

#include <modules/brushingandlinking/processors/brushingandlinkingprocessor.h>

#include <inviwo/core/datastructures/bitset.h>
#include <modules/brushingandlinking/brushingandlinkingmanager.h>
#include <modules/brushingandlinking/datastructures/brushingaction.h>

#include <fmt/format.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo BrushingAndLinkingProcessor::processorInfo_{
    "org.inviwo.BrushingAndLinkingProcessor",  // Class identifier
    "Brushing And Linking Processor",          // Display name
    "Brushing And Linking",                    // Category
    CodeState::Stable,                         // Code state
    "Brushing, Linking",                       // Tags
};
const ProcessorInfo BrushingAndLinkingProcessor::getProcessorInfo() const { return processorInfo_; }

BrushingAndLinkingProcessor::BrushingAndLinkingProcessor()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , clearSelection_("clearSelection", "Clear Selection",
                      [&]() { outport_.getManager().clearSelected(); })
    , clearHighlight_("clearHighlight", "Clear Highlighting",
                      [&]() { outport_.getManager().clearHighlighted(); })
    , clearCols_("clearCols", "Clear Columns",
                 [&]() { outport_.getManager().clearSelected(BrushingTarget::Column); })
    , clearAll_("clearAll", "Clear Selections and Highlights",
                [&]() {
                    outport_.getManager().clearSelected();
                    outport_.getManager().clearHighlighted();
                    outport_.getManager().clearSelected(BrushingTarget::Column);
                })
    , logging_("logging", "Logging", false, InvalidationLevel::Valid)
    , logFilterActions_("filterActions", "Filter Actions", true, InvalidationLevel::Valid)
    , logSelectActions_("selectActions", "Select Actions", true, InvalidationLevel::Valid)
    , logHighlightActions_("highlightActions", "Highlight Actions", true, InvalidationLevel::Valid)
    , maxVisibleIndices_("maxVisibleIndices", "Max Visible Indices", 50, 1, 1000, 1,
                         InvalidationLevel::Valid, PropertySemantics::Text) {

    inport_.setOptional(true);
    addPorts(inport_, outport_);

    addProperties(clearSelection_, clearHighlight_, clearCols_, clearAll_, logging_);
    logging_.addProperties(logFilterActions_, logSelectActions_, logHighlightActions_,
                           maxVisibleIndices_);

    outport_.getManager().setParent(&inport_.getManager());
    outport_.getManager().onBrush([this](BrushingAction action, BrushingTarget target,
                                         const BitSet& indices, std::string_view source) {
        if (!logging_.isChecked()) return;
        if ((action == BrushingAction::Filter) && !logFilterActions_) return;
        if ((action == BrushingAction::Select) && !logSelectActions_) return;
        if ((action == BrushingAction::Highlight) && !logHighlightActions_) return;

        const auto maxIndices = static_cast<size_t>(maxVisibleIndices_);
        const auto str = fmt::format(
            "{}{}",
            fmt::join(indices.begin(),
                      std::next(indices.begin(), std::min(indices.size(), maxIndices)), ", "),
            indices.size() > maxIndices ? "..." : "");

        LogProcessorInfo(fmt::format(
            "{:<20} action: {:<13}, target: {}\nsource: {}\nindices: [{}] ({})", getDisplayName(),
            action, target.getString(), source, str, indices.cardinality()));
    });
}

void BrushingAndLinkingProcessor::process() {}

}  // namespace inviwo
