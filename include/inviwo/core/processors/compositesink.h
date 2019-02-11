/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifndef IVW_COMPOSITESINK_H
#define IVW_COMPOSITESINK_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
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
 * Base class for all sink processors inside the sub network of a CompositeProcessor.
 * @see CompositeSink
 * @see CompositeProcessor
 */
class IVW_CORE_API CompositeSinkBase : public Processor {
public:
    CompositeSinkBase();
    virtual ~CompositeSinkBase() = default;

    /**
     * Outport to be used by the CompositeProcessor to get data from its sub network.
     */
    virtual Outport& getSuperOutport() = 0;
};

/**
 * Processor used to connect outports in a sub network inside of a CompositeProcessor to inports in
 * the network it is in (referred as the super network). The CompositeProcessor will find all
 * SinkProcessors in its sub network and add the SinkProcessors super outports to it self. Whenever
 * the sub network gets evaluated the CompositeSink will pass through inport data to its super
 * outport, thus making the data available to the super network. Note that the actual data will not
 * be copied since shared pointers are used.
 * @see CompositeProcessor
 * @see CompositeSource
 */
template <typename InportType, typename OutportType>
class CompositeSink : public CompositeSinkBase {
public:
    static_assert(std::is_same<typename InportType::type, typename OutportType::type>::value,
                  "InportType and OutportType must work with the same data type");
    CompositeSink();
    virtual ~CompositeSink() = default;

    virtual void process() override;

    /**
     * Outport to be used by the CompositeProcessor to get data from its sub network.
     */
    virtual Outport& getSuperOutport() override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    virtual const ProcessorInfo getProcessorInfo() const override;

private:
    InportType inport_;
    OutportType superOutport_;  ///< To be added to CompositeProcessor, not itself
};

template <typename InportType, typename OutportType>
struct ProcessorTraits<CompositeSink<InportType, OutportType>> {
    static ProcessorInfo getProcessorInfo() {
        using intype = typename InportType::type;
        using outtype = typename InportType::type;
        static_assert(std::is_same<intype, outtype>::value, "type mismatch");
        auto name = util::cleanIdentifier(DataTraits<intype>::dataName() + " Meta Sink", " ");
        auto id = util::appendIfNotEmpty(PortTraits<OutportType>::classIdentifier(), ".metasink");
        return {
            id,                 // Class identifier
            name,               // Display name
            "Composite",        // Category
            CodeState::Stable,  // Code state
            "Composite",        // Tags
            false               // Visible
        };
    }
};

template <typename InportType, typename OutportType>
const ProcessorInfo CompositeSink<InportType, OutportType>::getProcessorInfo() const {
    return ProcessorTraits<CompositeSink<InportType, OutportType>>::getProcessorInfo();
}

template <typename InportType, typename OutportType>
void CompositeSink<InportType, OutportType>::process() {
    superOutport_.setData(inport_.getData());
}

template <typename InportType, typename OutportType>
CompositeSink<InportType, OutportType>::CompositeSink()
    : CompositeSinkBase(), inport_{"inport"}, superOutport_{"outport"} {
    addPort(inport_);
    addPortToGroup(&superOutport_, "default");
}

template <typename InportType, typename OutportType>
Outport& CompositeSink<InportType, OutportType>::getSuperOutport() {
    return superOutport_;
}

template <typename InportType, typename OutportType>
void CompositeSink<InportType, OutportType>::serialize(Serializer& s) const {
    CompositeSinkBase::serialize(s);
    // Need to use pointers here, since the port will be serialized in portconnections etc and we
    // want the serialization to know they are the same object.
    auto ptr = &superOutport_;
    s.serialize("SuperOutport", ptr);
}

template <typename InportType, typename OutportType>
void CompositeSink<InportType, OutportType>::deserialize(Deserializer& d) {
    CompositeSinkBase::deserialize(d);
    // Need to use pointers here, since the port will be deserialized in portconnections etc and we
    // want the serialization to know they are the same object.
    auto ptr = &superOutport_;
    d.deserialize("SuperOutport", ptr);
}

}  // namespace inviwo

#endif  // IVW_COMPOSITESINK_H
