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

#include <modules/brushingandlinking/brushingandlinkingmoduledefine.h>

#include <inviwo/core/datastructures/datasequence.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>

#include <modules/brushingandlinking/ports/brushingandlinkingports.h>

namespace inviwo {

template <typename T>
class SequenceBrush : public Processor {
public:
    SequenceBrush();

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    static constexpr std::string_view identifierSuffix() { return ".sequence.brush"; }

private:
    DataInport<DataSequence<T>> inport_;
    BrushingAndLinkingInport bnl_;
    DataOutport<DataSequence<T>> outport_;

    OptionProperty<BrushingAction> action_;
    OptionProperty<BrushingTarget> target_;
    OrdinalProperty<size_t> maxSize_;
    OrdinalProperty<size_t> minSize_;
};

template <typename T>
const ProcessorInfo& SequenceBrush<T>::getProcessorInfo() const {
    static const ProcessorInfo info{ProcessorTraits<SequenceBrush<T>>::getProcessorInfo()};
    return info;
}

template <typename T>
struct ProcessorTraits<SequenceBrush<T>> {
    static ProcessorInfo getProcessorInfo() {

        const auto name = fmt::format("{} Sequence Brush", DataTraits<T>::dataName());
        const auto cid = fmt::format("{}{}", DataTraits<T>::classIdentifier(),
                                     SequenceBrush<T>::identifierSuffix());

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

template <typename T>
SequenceBrush<T>::SequenceBrush()
    : Processor{}
    , inport_{"inport", "DataSequence to brush"_help}
    , bnl_{"bnl"}
    , outport_{"outport", "Brushed DataSequence"_help}
    , action_{"action",
              "Action",
              {BrushingAction::Filter, BrushingAction::Select, BrushingAction::Highlight},
              1}
    , target_{"target", "Target", {BrushingTarget::Row, BrushingTarget::Column}, 0}
    , maxSize_{"maxSize", "Max Size", util::ordinalCount(0uz)}
    , minSize_{"minSize", "Min Size", util::ordinalCount(0uz)} {
    addPorts(inport_, bnl_, outport_);

    addProperties(action_, target_, maxSize_, minSize_);

    bnl_.setInvalidationLevels(
        {{BrushingModifications{flags::any}, InvalidationLevel::Valid},
         {{target_.get()}, fromAction(action_.get()), InvalidationLevel::InvalidOutput}});
}

template <typename T>
void SequenceBrush<T>::process() {
    if (target_.isModified() || action_.isModified()) {
        bnl_.setInvalidationLevels(
            {{BrushingModifications{flags::any}, InvalidationLevel::Valid},
             {{target_.get()}, fromAction(action_.get()), InvalidationLevel::InvalidOutput}});
    }

    const auto sequence = inport_.getData();
    auto brushed = std::make_shared<DataSequence<T>>();

    const auto& indices = bnl_.getIndices(action_.get(), target_.get());
    for (auto i : indices) {
        if (maxSize_ != 0 && i >= maxSize_) break;
        if (i >= sequence->size()) break;
        brushed->push_back(sequence->at(i));
    }

    for (auto&& [i, data] : std::views::zip(std::views::iota(0uz), *sequence)) {
        if (brushed->size() >= minSize_.get()) break;
        if (indices.contains(i)) continue;
        brushed->push_back(sequence->at(i));
    }

    outport_.setData(brushed);
}

}  // namespace inviwo
