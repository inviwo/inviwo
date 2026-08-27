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
#include <inviwo/core/datastructures/datatraits.h>
#include <inviwo/core/datastructures/datasequence.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processortraits.h>
#include <inviwo/core/processors/sequenceselect.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/ports/porttraits.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/ports/imageport.h>

namespace inviwo {

/**
 * Base class for all sink processors inside the sub network of a SequenceProcessor.
 * @see SequenceCompositeSink
 * @see SequenceProcessor
 */
class IVW_CORE_API SequenceCompositeSinkBase : public Processor {
public:
    SequenceCompositeSinkBase();
    SequenceCompositeSinkBase(const SequenceCompositeSinkBase&) = delete;
    SequenceCompositeSinkBase(SequenceCompositeSinkBase&&) = delete;
    SequenceCompositeSinkBase& operator=(const SequenceCompositeSinkBase&) = delete;
    SequenceCompositeSinkBase& operator=(SequenceCompositeSinkBase&&) = delete;
    virtual ~SequenceCompositeSinkBase() = default;

    static constexpr std::string_view identifierSuffix() { return ".metasequencesink"; };

    /**
     * Outport to be used by the SequenceProcessor to get data from its sub network.
     */
    virtual Outport& getSuperOutport() = 0;

    virtual void sync(SequenceCompositeSinkBase* source) = 0;

    virtual void newData(size_t size) = 0;
    virtual void setSequenceIndex(size_t) = 0;

    virtual void superProcessEnd() = 0;

    virtual std::shared_ptr<Processor> createConverter() const = 0;
};

/**
 * Processor used to connect outports in a sub network inside of a SequenceProcessor to inports in
 * the network it is in (referred as the super network). The SequenceProcessor will find all
 * SinkProcessors in its sub network and add the SinkProcessors' super outports to itself. Whenever
 * the sub network gets evaluated the SequenceCompositeSink will pass through inport data to its
 * super outport, thus making the data available to the super network. Note that the actual data
 * will not be copied since shared pointers are used.
 *
 * The Outport type should be a sequence version of the inport type.
 *
 * @see SequenceProcessor
 * @see SequenceCompositeSource
 */
template <typename InportType, typename OutportSequenceType>
class SequenceCompositeSink : public SequenceCompositeSinkBase {
    using InportData = typename InportType::type;
    using OutportSequenceData = typename OutportSequenceType::type;
    static_assert(std::is_same_v<InportData, typename OutportSequenceData::type>,
                  "InportType and OutportSequenceType must work with the same data type");

public:
    SequenceCompositeSink();
    SequenceCompositeSink(const SequenceCompositeSink&) = delete;
    SequenceCompositeSink(SequenceCompositeSink&&) = delete;
    SequenceCompositeSink& operator=(const SequenceCompositeSink&) = delete;
    SequenceCompositeSink& operator=(SequenceCompositeSink&&) = delete;
    virtual ~SequenceCompositeSink() = default;

    virtual void process() override;

    /**
     * Outport to be used by the SequenceProcessor to get data from its sub network.
     */
    virtual Outport& getSuperOutport() override;

    virtual void sync(SequenceCompositeSinkBase* source) override;

    virtual void newData(size_t size) override;

    virtual const ProcessorInfo& getProcessorInfo() const override;

    virtual void superProcessEnd() override;

    virtual void setSequenceIndex(size_t index) override { sequenceIndex_ = index; }

    virtual std::shared_ptr<Processor> createConverter() const override {
        return std::make_shared<SequenceSelect<InportData>>();
    }

    static std::string superPortIdentifier(Port* port) {
        auto portIdentifier = port->getIdentifier();
        // Make first letter uppercase for readability when combined with processor
        // display name
        if (!portIdentifier.empty()) {
            portIdentifier.front() = static_cast<char>(std::toupper(portIdentifier.front()));
        }
        return util::stripIdentifier(
            fmt::format("{}{}", port->getProcessor()->getDisplayName(), portIdentifier));
    }

private:
    std::shared_ptr<std::vector<std::shared_ptr<const typename OutportSequenceData::type>>> data_;
    InportType inport_;

    // To be added to SequenceProcessor, not itself
    std::shared_ptr<OutportSequenceType> superOutport_;
    size_t sequenceIndex_ = 0;
};

template <typename InportType, typename OutportSequenceType>
struct ProcessorTraits<SequenceCompositeSink<InportType, OutportSequenceType>> {
    static ProcessorInfo getProcessorInfo() {
        using inType = typename InportType::type;
        using sequenceType = typename OutportSequenceType::type;
        using outType = typename sequenceType::type;
        static_assert(std::is_same_v<inType, outType>, "type mismatch");
        auto name = fmt::format("{} Meta Sink", DataTraits<inType>::dataName());
        auto id = util::appendIfNotEmpty(PortTraits<InportType>::classIdentifier(),
                                         SequenceCompositeSinkBase::identifierSuffix());
        return {
            id,                 // Class identifier
            name,               // Display name
            "Composite",        // Category
            CodeState::Stable,  // Code state
            "Composite",        // Tags
            "Internal processor for sequence processors"_help,
            false  // Visible
        };
    }
};

template <typename InportType, typename OutportSequenceType>
const ProcessorInfo& SequenceCompositeSink<InportType, OutportSequenceType>::getProcessorInfo()
    const {
    static const ProcessorInfo info{ProcessorTraits<
        SequenceCompositeSink<InportType, OutportSequenceType>>::getProcessorInfo()};
    return info;
}

template <typename InportType, typename OutportSequenceType>
SequenceCompositeSink<InportType, OutportSequenceType>::SequenceCompositeSink()
    : SequenceCompositeSinkBase(), data_{nullptr}, inport_{"inport"}, superOutport_{nullptr} {

    addPort(inport_);
}

template <typename InportType, typename OutportSequenceType>
void SequenceCompositeSink<InportType, OutportSequenceType>::process() {
    if constexpr (std::is_same_v<InportType, ImageInport>) {
        data_->at(sequenceIndex_) = std::shared_ptr<Image>{inport_.getData()->clone()};
    } else {
        data_->at(sequenceIndex_) = inport_.getData();
    }
}

template <typename InportType, typename OutportSequenceType>
Outport& SequenceCompositeSink<InportType, OutportSequenceType>::getSuperOutport() {
    return *superOutport_;
}

template <typename InportType, typename OutportSequenceType>
void SequenceCompositeSink<InportType, OutportSequenceType>::sync(
    SequenceCompositeSinkBase* source) {

    if (source) {
        if (auto* typedSource =
                dynamic_cast<SequenceCompositeSink<InportType, OutportSequenceType>*>(source)) {
            superOutport_ = typedSource->superOutport_;
            addPortToGroup(superOutport_.get(), "default");
            data_ = typedSource->data_;
        } else {
            throw Exception(SourceContext{}, "SequenceCompositeSinkBase of wrong type");
        }
    } else {
        auto* outport = inport_.getConnectedOutport();
        superOutport_ = std::make_shared<OutportSequenceType>(superPortIdentifier(outport));
        addPortToGroup(superOutport_.get(), "default");
        data_ = std::make_shared<
            std::vector<std::shared_ptr<const typename OutportSequenceData::type>>>();
    }
}

template <typename InportType, typename OutportSequenceType>
void SequenceCompositeSink<InportType, OutportSequenceType>::newData(size_t size) {
    data_->resize(size);
}

template <typename InportType, typename OutportSequenceType>
void SequenceCompositeSink<InportType, OutportSequenceType>::superProcessEnd() {
    // only update the output once all sequence slots are filled
    if (std::ranges::all_of(*data_, [](auto& item) { return item != nullptr; })) {
        superOutport_->setData(std::make_shared<OutportSequenceData>(std::from_range, *data_));
    }
}

}  // namespace inviwo
