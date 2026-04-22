/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2026 Inviwo Foundation
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

#include <modules/brushingandlinking/brushingandlinkingmodule.h>

#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/processors/processortraits.h>
#include <inviwo/core/processors/compositesource.h>
#include <inviwo/core/processors/compositesink.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>
#include <modules/brushingandlinking/processors/brushingandlinkingprocessor.h>

#include <memory>
#include <modules/brushingandlinking/processors/propertytobrushing.h>

namespace inviwo {
class InviwoApplication;

template <>
struct ProcessorTraits<CompositeSource<BrushingAndLinkingInport, BrushingAndLinkingOutport>> {
    static ProcessorInfo getProcessorInfo() {
        using intype = typename BrushingAndLinkingInport::type;
        using outtype = typename BrushingAndLinkingInport::type;
        static_assert(std::is_same<intype, outtype>::value, "type mismatch");
        auto name = fmt::format("{} Meta Source", "BrushingAndLinking");
        auto id = util::appendIfNotEmpty(PortTraits<BrushingAndLinkingOutport>::classIdentifier(),
                                         CompositeSourceBase::identifierSuffix());
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

template <>
struct ProcessorTraits<CompositeSink<BrushingAndLinkingInport, BrushingAndLinkingOutport>> {
    static ProcessorInfo getProcessorInfo() {
        using intype = typename BrushingAndLinkingInport::type;
        using outtype = typename BrushingAndLinkingInport::type;
        static_assert(std::is_same<intype, outtype>::value, "type mismatch");
        auto name = fmt::format("{} Meta Sink", "BrushingAndLinking");
        auto cid = util::appendIfNotEmpty(PortTraits<BrushingAndLinkingOutport>::classIdentifier(),
                                          CompositeSinkBase::identifierSuffix());
        return {
            cid,                // Class identifier
            name,               // Display name
            "Composite",        // Category
            CodeState::Stable,  // Code state
            "Composite",        // Tags
            "Internal processor for composites processors"_help,
            false  // Visible
        };
    }
};

template <>
const ProcessorInfo&
CompositeSource<BrushingAndLinkingInport, BrushingAndLinkingOutport>::getProcessorInfo() const {
    static const ProcessorInfo info{ProcessorTraits<
        CompositeSource<BrushingAndLinkingInport, BrushingAndLinkingOutport>>::getProcessorInfo()};
    return info;
}

template <>
CompositeSource<BrushingAndLinkingInport, BrushingAndLinkingOutport>::CompositeSource()
    : CompositeSourceBase(), superInport_{"inport"}, outport_{"outport"} {
    addPort(outport_);
    addPortToGroup(&superInport_, "default");

    outport_.getManager().setParent(&superInport_.getManager());
}

template <>
void CompositeSource<BrushingAndLinkingInport, BrushingAndLinkingOutport>::process() {}

template <>
Inport& CompositeSource<BrushingAndLinkingInport, BrushingAndLinkingOutport>::getSuperInport() {
    return superInport_;
}

template <>
void CompositeSource<BrushingAndLinkingInport, BrushingAndLinkingOutport>::serialize(
    Serializer& s) const {
    CompositeSourceBase::serialize(s);
    s.serialize("SuperInport", superInport_);
}

template <>
void CompositeSource<BrushingAndLinkingInport, BrushingAndLinkingOutport>::deserialize(
    Deserializer& d) {
    CompositeSourceBase::deserialize(d);
    d.deserialize("SuperInport", superInport_);
}

template <>
void CompositeSource<BrushingAndLinkingInport, BrushingAndLinkingOutport>::propagateEvent(
    Event* event, Outport* source) {
    if (event->hasVisitedProcessor(this)) return;
    event->markAsVisited(this);
    invokeEvent(event);
    if (event->hasBeenUsed()) return;
    if (event->shouldPropagateTo(&superInport_, this, source)) {
        superInport_.propagateEvent(event);
    }
}

template <>
const ProcessorInfo&
CompositeSink<BrushingAndLinkingInport, BrushingAndLinkingOutport>::getProcessorInfo() const {
    static const ProcessorInfo info{ProcessorTraits<
        CompositeSink<BrushingAndLinkingInport, BrushingAndLinkingOutport>>::getProcessorInfo()};

    return info;
}

template <>
CompositeSink<BrushingAndLinkingInport, BrushingAndLinkingOutport>::CompositeSink()
    : CompositeSinkBase(), inport_{"inport"}, superOutport_{"outport"} {
    addPort(inport_);
    addPortToGroup(&superOutport_, "default");

    superOutport_.getManager().setParent(&inport_.getManager());
}

template <>
void CompositeSink<BrushingAndLinkingInport, BrushingAndLinkingOutport>::process() {}

template <>
Outport& CompositeSink<BrushingAndLinkingInport, BrushingAndLinkingOutport>::getSuperOutport() {
    return superOutport_;
}

template <>
void CompositeSink<BrushingAndLinkingInport, BrushingAndLinkingOutport>::serialize(
    Serializer& s) const {
    CompositeSinkBase::serialize(s);
    s.serialize("SuperOutport", superOutport_);
}

template <>
void CompositeSink<BrushingAndLinkingInport, BrushingAndLinkingOutport>::deserialize(
    Deserializer& d) {
    CompositeSinkBase::deserialize(d);
    d.deserialize("SuperOutport", superOutport_);
}

BrushingAndLinkingModule::BrushingAndLinkingModule(InviwoApplication* app)
    : InviwoModule(app, "BrushingAndLinking") {
    // Processors
    registerProcessor<BrushingAndLinkingProcessor>();
    registerProcessor<CompositeSource<BrushingAndLinkingInport, BrushingAndLinkingOutport>>();
    registerProcessor<PropertyToBrushing>();

    // Ports
    registerPort<BrushingAndLinkingOutport>();
    registerPort<BrushingAndLinkingInport>();
}

}  // namespace inviwo
