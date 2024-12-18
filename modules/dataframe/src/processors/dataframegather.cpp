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

#include <inviwo/dataframe/processors/dataframegather.h>

#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/bitset.h>
#include <inviwo/dataframe/datastructures/column.h>
#include <inviwo/dataframe/util/dataframeutil.h>

#include <ranges>
#include <unordered_set>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DataFrameGather::processorInfo_{
    "org.inviwo.DataFrameGather",  // Class identifier
    "DataFrame Gather",            // Display name
    "DataFrame",                   // Category
    CodeState::Experimental,       // Code state
    Tags::CPU | Tag{"DataFrame"},  // Tags
    R"(Aggregates rows based on unique entries in the selected column. Brushing &
      Linking indices are translated between row indices of the input DataFrame
      and the resulting aggregated DataFrame.

      Columns containing floating point values will not be aggregated.
    )"_unindentHelp,
};

const ProcessorInfo& DataFrameGather::getProcessorInfo() const { return processorInfo_; }

DataFrameGather::DataFrameGather()
    : Processor{}
    , inport_{"inport", "Input DataFrame"_help}
    , bnlInport_{"brushingInport", "Brushing & Linking connected to input DataFrame"_help}
    , outport_{"outport", "Aggregated DataFrame resulting from gathering unique rows"_help}
    , bnlOutport_{"brushingOutport",
                  "Brushing & Linking port using indices connected to the resulting DataFrame"_help}
    , gatherColumn_{"gatherColumn", "Selected Column", inport_,
                    ColumnOptionProperty::AddNoneOption::No, 1}
    , propagationAction_{"propagationAction", "Propagate Selection as",
                         OptionPropertyState<BrushingAction>{
                             .options = {{"selection", "Selection", BrushingAction::Select},
                                         {"filtering", "Filtering", BrushingAction::Filter}},
                             .help = "Propagate Brushing & Linking selections in the result "
                                     "DataFrame as "
                                     "selection or filter events"_help}
                             .setSelectedValue(BrushingAction::Filter)}
    , maxBrushingIndex_{0u} {

    addPorts(inport_, bnlInport_, outport_, bnlOutport_);
    addProperties(gatherColumn_, propagationAction_);

    bnlOutport_.getManager().onBrush([this](BrushingAction action, BrushingTarget target,
                                            const BitSet& indices, std::string_view) {
        // transform indices back to gathered row indices of the input DataFrame
        BitSet rows;
        for (const std::uint32_t& row : indices) {
            rows |= gatheredRowToRowIndices_[row];
        }
        if (action == BrushingAction::Select) {
            action = propagationAction_.get();
        }
        if (action == BrushingAction::Filter && !rows.empty()) {
            // invert bitsets used for filtering
            rows.flipRange(0, maxBrushingIndex_);
        }

        bnlInport_.brush(action, target, rows, getIdentifier());
    });
}

void DataFrameGather::process() {
    if (inport_.isChanged() || gatherColumn_.isModified() || !dataFrame_) {
        auto srcDataFrame = inport_.getData();
        auto column = srcDataFrame->getColumn(gatherColumn_.get());

        matchingGatheredRow_.clear();
        gatheredRowToRowIndices_.clear();

        auto rowSelection =
            column->getBuffer()
                ->getRepresentation<BufferRAM>()
                ->dispatch<std::vector<std::uint32_t>>([&]<typename T>(T* typedBuf) {
                    using ValueType = util::PrecisionValueType<T>;
                    if constexpr (std::is_floating_point_v<ValueType>) {
                        // ignore floating point types, select all rows
                        auto iota = std::views::iota(
                            std::uint32_t{0u}, static_cast<std::uint32_t>(typedBuf->getSize()));
                        return std::vector<std::uint32_t>(iota.begin(), iota.end());
                    } else {
                        auto& data = typedBuf->getDataContainer();
                        std::vector<std::uint32_t> selectedRows;
                        std::unordered_map<ValueType, std::uint32_t> gatheredValueToRow;
                        for (std::uint32_t row = 0; row < static_cast<std::uint32_t>(data.size());
                             ++row) {
                            auto&& [it, successful] =
                                gatheredValueToRow.emplace(data[row], gatheredValueToRow.size());
                            if (successful) {
                                selectedRows.push_back(row);
                            }
                            // TODO: need to translate index column[row] to row
                            matchingGatheredRow_.push_back(it->second);
                            // TODO: use content of index column[row] instead of row?
                            gatheredRowToRowIndices_[it->second].add(row);
                        }
                        return selectedRows;
                    }
                });

        maxBrushingIndex_ =
            static_cast<std::uint32_t>(srcDataFrame->getIndexColumn()->getDataRange().y);

        dataFrame_ = std::make_shared<DataFrame>();
        dataFrame_->addColumn(std::shared_ptr<Column>(column->clone(rowSelection)));
        dataFrame_->updateIndexBuffer();
    }

    // if (bnlInport_.isConnected() && bnlInport_.getManager().isModified()) {
    //     auto& bnlSource = bnlInport_.getManager();
    //     if (bnlSource.isHighlightModified()) {
    //         BitSet highlightedGatheredRows;
    //         for (auto index : bnlSource.getHighlightedIndices()) {
    //             highlightedGatheredRows.add(matchingGatheredRow_[index]);
    //         }
    //         bnlOutport_.getManager().highlight(highlightedGatheredRows, BrushingTarget::Row);
    //     }
    // }

    outport_.setData(dataFrame_);
}

}  // namespace inviwo
