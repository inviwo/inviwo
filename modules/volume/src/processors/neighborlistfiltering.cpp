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

#include <inviwo/volume/processors/neighborlistfiltering.h>

#include <ranges>
#include <map>
#include <set>
#include <vector>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo NeighborListFiltering::processorInfo_{
    "org.inviwo.NeighborListFiltering",  // Class identifier
    "Neighbor List Filtering",           // Display name
    "Undefined",                         // Category
    CodeState::Stable,                   // Code state
    Tags::CPU,                           // Tags
    R"(Takes a list of neighbor pairs and a center selection,
     then grows that selection by range steps. Finally it filters
     out any index not in the selection)"_unindentHelp,
};

const ProcessorInfo& NeighborListFiltering::getProcessorInfo() const { return processorInfo_; }

NeighborListFiltering::NeighborListFiltering()
    : Processor{}
    , inport_{"inport"}
    , bnl_{"brushingAndLinking"}
    , enable_{"enable", "Enable", true}
    , pairFirst_{"pairFirst", "Pair First", inport_, ColumnOptionProperty::AddNoneOption::No, 1}
    , pairSecond_{"pairSecond", "Pair Second", inport_, ColumnOptionProperty::AddNoneOption::No, 2}
    , center_{"center", "Center"}
    , range_{"range", "Range"} {

    addPorts(inport_, bnl_);
    addProperties(enable_, pairFirst_, pairSecond_, center_, range_);

    bnl_.setInvalidationLevels({});
}

void NeighborListFiltering::process() {
    if (!enable_) {
        bnl_.filter(getIdentifier(), BitSet{});
        return;
    }

    auto df = inport_.getData();

    df->getColumn(pairFirst_)
        ->getRAMRepresentation()
        ->dispatch<void, dispatching::filter::UnsignedIntegerScalars>([&](auto br) {
            using ValueType = util::PrecisionValueType<decltype(br)>;

            const auto& iCol = br->getDataContainer();
            const auto& jCol = df->getColumn(pairSecond_)->getContainer<ValueType>();

            std::map<ValueType, std::vector<ValueType>> neighbors;

            for (auto [i, j] : std::views::zip(iCol, jCol)) {
                neighbors[i].push_back(j);
                neighbors[j].push_back(i);
            }

            std::set<ValueType> selected;
            selected.emplace(static_cast<ValueType>(center_.get()));

            std::set<ValueType> tmp;

            for (size_t r = 0; r < range_.get(); ++r) {
                tmp = selected;
                for (auto i : tmp) {
                    selected.insert_range(neighbors[i]);
                }
            }

            const auto min = static_cast<std::uint32_t>(neighbors.begin()->first);
            const auto max = static_cast<std::uint32_t>(neighbors.rbegin()->first);

            auto vec = std::views::iota(min, max + 1) |
                       std::views::filter([&](auto i) { return !selected.contains(i); });

            bnl_.filter(getIdentifier(), BitSet{std::begin(vec), std::end(vec)});
        });
}

}  // namespace inviwo
