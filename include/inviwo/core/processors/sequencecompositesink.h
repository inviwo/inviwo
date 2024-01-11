/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
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
    virtual ~SequenceCompositeSinkBase() = default;

    static constexpr std::string_view identifierSuffix() { return ".metasequencesink"; };

    /**
     * Outport to be used by the SequenceProcessor to get data from its sub network.
     */
    virtual Outport& getSuperOutport() = 0;
    virtual void superProcessStart() = 0;
    virtual void superProcessEnd() = 0;
};

/**
 * Processor used to connect outports in a sub network inside of a SequenceProcessor to inports in
 * the network it is in (referred as the super network). The SequenceProcessor will find all
 * SinkProcessors in its sub network and add the SinkProcessors super outports to it self. Whenever
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
public:
    // static_assert(
    //     std::is_same<typename InportType::type, typename OutportSequenceType::type>::value,
    //     "InportType and OutportSequenceType must work with the same data type");
    SequenceCompositeSink();
    virtual ~SequenceCompositeSink() = default;

    virtual void process() override;

    /**
     * Outport to be used by the SequenceProcessor to get data from its sub network.
     */
    virtual Outport& getSuperOutport() override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    virtual const ProcessorInfo getProcessorInfo() const override;

    virtual void superProcessStart() override {
        sequenceData_ = std::make_shared<OutportSequenceData>();
    }
    virtual void superProcessEnd() override { superOutport_.setData(sequenceData_); }

private:
    using InportData = typename InportType::type;
    using OutportSequenceData = typename OutportSequenceType::type;

    std::shared_ptr<OutportSequenceData> sequenceData_;
    InportType inport_;
    OutportSequenceType superOutport_;  ///< To be added to SequenceProcessor, not itself
};

template <typename InportType, typename OutportSequenceType>
struct ProcessorTraits<SequenceCompositeSink<InportType, OutportSequenceType>> {
    static ProcessorInfo getProcessorInfo() {
        using intype = typename InportType::type;
        using outtype = typename InportType::type;
        static_assert(std::is_same<intype, outtype>::value, "type mismatch");
        auto name =
            util::cleanIdentifier(fmt::format("{} Meta Sink", DataTraits<intype>::dataName()), " ");
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
const ProcessorInfo SequenceCompositeSink<InportType, OutportSequenceType>::getProcessorInfo()
    const {
    return ProcessorTraits<
        SequenceCompositeSink<InportType, OutportSequenceType>>::getProcessorInfo();
}

template <typename InportType, typename OutportSequenceType>
void SequenceCompositeSink<InportType, OutportSequenceType>::process() {
    if constexpr (std::is_same_v<InportType, ImageInport>) {
        sequenceData_->push_back(std::shared_ptr<Image>{inport_.getData()->clone()});
    } else {
        sequenceData_->push_back(inport_.getData());
    }
}

template <typename InportType, typename OutportSequenceType>
SequenceCompositeSink<InportType, OutportSequenceType>::SequenceCompositeSink()
    : SequenceCompositeSinkBase()
    , sequenceData_{std::make_shared<OutportSequenceData>()}
    , inport_{"inport"}
    , superOutport_{"outport"} {

    addPort(inport_);
    addPortToGroup(&superOutport_, "default");
}

template <typename InportType, typename OutportSequenceType>
Outport& SequenceCompositeSink<InportType, OutportSequenceType>::getSuperOutport() {
    return superOutport_;
}

template <typename InportType, typename OutportSequenceType>
void SequenceCompositeSink<InportType, OutportSequenceType>::serialize(Serializer& s) const {
    SequenceCompositeSinkBase::serialize(s);
    s.serialize("SuperOutport", superOutport_);
}

template <typename InportType, typename OutportSequenceType>
void SequenceCompositeSink<InportType, OutportSequenceType>::deserialize(Deserializer& d) {
    SequenceCompositeSinkBase::deserialize(d);
    d.deserialize("SuperOutport", superOutport_);
}

}  // namespace inviwo
