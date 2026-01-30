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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/datastructures/datasequence.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/eventproperty.h>

namespace inviwo {
/**
 * Template for processors that want to select an element from an
 * input vector and set it as output.
 */
template <typename T, typename OutportType = DataOutport<T>>
class SequenceSelect : public Processor {
public:
    SequenceSelect();
    SequenceSelect(const SequenceSelect&) = delete;
    SequenceSelect& operator=(const SequenceSelect&) = delete;
    SequenceSelect(SequenceSelect&&) = delete;
    SequenceSelect& operator=(SequenceSelect&&) = delete;
    virtual ~SequenceSelect() = default;

    virtual const ProcessorInfo& getProcessorInfo() const override;

    void process() override;

    static constexpr std::string_view identifierSuffix() { return ".sequence.select"; }

protected:
    DataInport<DataSequence<T>> inport_;
    OutportType outport_;
    IntSizeTProperty index_;

    EventProperty next_;
    EventProperty prev_;
};

template <typename T, typename OutportType>
SequenceSelect<T, OutportType>::SequenceSelect()
    : Processor()
    , inport_{"inport", "Sequence of data to select from"_help}
    , outport_{"outport", "The selected item"_help}
    , index_{"index", "Index", util::ordinalCount(0uz)}
    , next_{"next", "Next", [this](Event*) { index_.set(index_.get() + 1uz); }, IvwKey::Up,
            KeyState::Press}
    , prev_{"prev", "Prev", [this](Event*) { index_.set(std::max(index_.get(), 1uz) - 1uz); },
            IvwKey::Down, KeyState::Press} {

    addPorts(inport_, outport_);
    addProperties(index_, next_, prev_);

    inport_.onChange([this]() {
        if (auto data = inport_.getData()) {
            index_.setMaxValue(std::max(data->size(), 1uz) - 1uz);
        }
    });
}

template <typename T, typename OutportType>
void SequenceSelect<T, OutportType>::process() {
    if (!inport_.isReady()) return;

    if (auto data = inport_.getData()) {
        if (data->size() == 0) {
            outport_.detachData();
            return;
        }
        size_t index = std::clamp(index_.get(), 0uz, data->size() - 1);
        outport_.setData((*data)[index]);
    } else {
        outport_.detachData();
    }
}

template <typename T, typename OutportType>
struct ProcessorTraits<SequenceSelect<T, OutportType>> {
    static ProcessorInfo getProcessorInfo() {

        const auto name = fmt::format("{} Sequence Select", DataTraits<T>::dataName());
        const auto cid = fmt::format("{}{}", DataTraits<T>::classIdentifier(),
                                     SequenceSelect<T, OutportType>::identifierSuffix());

        const auto doc =
            fmt::format("Select a specific {0} out of a sequence", DataTraits<T>::dataName());

        return {
            cid,                // Class identifier
            name,               // Display name
            "Data Selector",    // Category
            CodeState::Stable,  // Code state
            Tags::CPU,          // Tags
            Document{doc},
            true  // Visible
        };
    }
};

template <typename T, typename OutportType>
const ProcessorInfo& SequenceSelect<T, OutportType>::getProcessorInfo() const {
    static const ProcessorInfo info{
        ProcessorTraits<SequenceSelect<T, OutportType>>::getProcessorInfo()};
    return info;
}

}  // namespace inviwo
