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
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processortraits.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/ports/porttraits.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/utilities.h>

namespace inviwo {

/**
 * Base class for all source processors inside the sub network of a SequenceProcessor.
 * @see SequenceCompositeSource
 * @see SequenceProcessor
 */
class IVW_CORE_API SequenceCompositeSourceBase : public Processor {
public:
    SequenceCompositeSourceBase();
    virtual ~SequenceCompositeSourceBase() = default;

    static constexpr std::string_view identifierSuffix() { return ".metasequencesource"; };

    /**
     * Inport to be used by the SequenceProcessor to put data into its sub network.
     */
    virtual Inport& getSuperInport() = 0;

    virtual size_t sequenceSize() const = 0;
    virtual void setSequenceIndex(size_t) = 0;
};

/**
 * Processor used to connect inports in a sub network inside of a SequenceProcessor to outports in
 * the network it is in (referred as the super network). The SequenceProcessor will find all
 * SourceProcessors in its sub network and add the SourceProcessors super inports to it self.
 * Whenever the sub network gets evaluated the SourceProcessors will pass through super inport data
 * to its outport, thus making the data available to the sub network. Note that the actual data will
 * not be copied since shared pointers are used.
 *
 * The inport type should be a sequence version of the outport type.
 *
 * @see SequenceProcessor
 * @see SequenceCompositeSink
 */
template <typename InportSequenceType, typename OutportType>
class SequenceCompositeSource : public SequenceCompositeSourceBase {
public:
    // static_assert(
    //     std::is_same<typename InportSequenceType::type, typename OutportType::type>::value,
    //     "InportSequenceType and OutportType must work with the same data type");
    SequenceCompositeSource();
    virtual ~SequenceCompositeSource() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;

    /**
     * Inport to be used by the SequenceProcessor to put data into its sub network.
     */
    virtual Inport& getSuperInport() override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    virtual void propagateEvent(Event* event, Outport* source) override;

    virtual size_t sequenceSize() const override {
        if (superInport_.hasData()) {
            return superInport_.getData()->size();
        } else {
            return 0;
        }
    }
    virtual void setSequenceIndex(size_t index) override {
        if (index != sequenceIndex_) {
            sequenceIndex_ = index;
            invalidate(InvalidationLevel::InvalidOutput);
        }
    }

private:
    InportSequenceType superInport_;  ///< To be added to SequenceProcessor, not itself
    OutportType outport_;
    size_t sequenceIndex_ = 0;
};

template <typename InportSequenceType, typename OutportType>
struct ProcessorTraits<SequenceCompositeSource<InportSequenceType, OutportType>> {
    static ProcessorInfo getProcessorInfo() {
        using intype = typename InportSequenceType::type;
        using outtype = typename InportSequenceType::type;
        static_assert(std::is_same<intype, outtype>::value, "type mismatch");
        auto name = util::cleanIdentifier(
            fmt::format("{} Meta Source", DataTraits<intype>::dataName()), " ");
        auto id = util::appendIfNotEmpty(PortTraits<OutportType>::classIdentifier(),
                                         SequenceCompositeSourceBase::identifierSuffix());
        return {
            id,                 // Class identifier
            name,               // Display name
            "Meta",             // Category
            CodeState::Stable,  // Code state
            "Meta",             // Tags
            "Internal processor for composites processors"_help,
            false  // Visible
        };
    }
};

template <typename InportSequenceType, typename OutportType>
const ProcessorInfo SequenceCompositeSource<InportSequenceType, OutportType>::getProcessorInfo()
    const {
    return ProcessorTraits<
        SequenceCompositeSource<InportSequenceType, OutportType>>::getProcessorInfo();
}

template <typename InportSequenceType, typename OutportType>
SequenceCompositeSource<InportSequenceType, OutportType>::SequenceCompositeSource()
    : SequenceCompositeSourceBase(), superInport_{"inport"}, outport_{"outport"} {
    addPort(outport_);
    addPortToGroup(&superInport_, "default");
}

template <typename InportSequenceType, typename OutportType>
void SequenceCompositeSource<InportSequenceType, OutportType>::process() {
    auto data = superInport_.getData();
    if (sequenceIndex_ < data->size()) {
        outport_.setData((*data)[sequenceIndex_]);
    } else {
        outport_.detachData();
    }
}

template <typename InportSequenceType, typename OutportType>
Inport& SequenceCompositeSource<InportSequenceType, OutportType>::getSuperInport() {
    return superInport_;
}

template <typename InportSequenceType, typename OutportType>
void SequenceCompositeSource<InportSequenceType, OutportType>::serialize(Serializer& s) const {
    SequenceCompositeSourceBase::serialize(s);
    s.serialize("SuperInport", superInport_);
}

template <typename InportSequenceType, typename OutportType>
void SequenceCompositeSource<InportSequenceType, OutportType>::deserialize(Deserializer& d) {
    SequenceCompositeSourceBase::deserialize(d);
    d.deserialize("SuperInport", superInport_);
}

template <typename InportSequenceType, typename OutportType>
void SequenceCompositeSource<InportSequenceType, OutportType>::propagateEvent(Event* event,
                                                                              Outport* source) {
    if (event->hasVisitedProcessor(this)) return;
    event->markAsVisited(this);
    invokeEvent(event);
    if (event->hasBeenUsed()) return;
    if (event->shouldPropagateTo(&superInport_, this, source)) {
        superInport_.propagateEvent(event);
    }
}

}  // namespace inviwo
