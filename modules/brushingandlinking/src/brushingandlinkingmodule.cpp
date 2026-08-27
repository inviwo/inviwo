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

#include <inviwo/core/processors/sequencecompositesource.h>

#include <modules/brushingandlinking/ports/brushingandlinkingports.h>
#include <modules/brushingandlinking/processors/brushingandlinkingprocessor.h>

#include <memory>
#include <modules/brushingandlinking/processors/propertytobrushing.h>
#include <modules/brushingandlinking/processors/sequencebrush.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/buffer/buffer.h>

namespace inviwo {
class InviwoApplication;

template <>
struct ProcessorTraits<CompositeSource<BrushingAndLinkingInport, BrushingAndLinkingOutport>> {
    static ProcessorInfo getProcessorInfo() {
        constexpr auto name = std::string_view{"BrushingAndLinking Meta Source"};
        auto id = util::appendIfNotEmpty(PortTraits<BrushingAndLinkingOutport>::classIdentifier(),
                                         CompositeSourceBase::identifierSuffix());
        return {
            id,                 // Class identifier
            std::string{name},  // Display name
            "Meta",             // Category
            CodeState::Stable,  // Code state
            "Meta",             // Tags
            "Internal processor for composites processors"_help,
            false  // Visible
        };
    }
};

// Specialize to pass on the manager
template <>
CompositeSource<BrushingAndLinkingInport, BrushingAndLinkingOutport>::CompositeSource()
    : CompositeSourceBase(), superInport_{"inport"}, outport_{"outport"} {
    addPort(outport_);
    addPortToGroup(&superInport_, "default");

    outport_.getManager().setParent(&superInport_.getManager());
}

// Specialize to do nothing
template <>
void CompositeSource<BrushingAndLinkingInport, BrushingAndLinkingOutport>::process() {}

template <>
struct ProcessorTraits<CompositeSink<BrushingAndLinkingInport, BrushingAndLinkingOutport>> {
    static ProcessorInfo getProcessorInfo() {
        constexpr auto name = std::string_view{"BrushingAndLinking Meta Sink"};
        const auto cid =
            util::appendIfNotEmpty(PortTraits<BrushingAndLinkingOutport>::classIdentifier(),
                                   CompositeSinkBase::identifierSuffix());
        return {
            cid,                // Class identifier
            std::string{name},  // Display name
            "Composite",        // Category
            CodeState::Stable,  // Code state
            "Composite",        // Tags
            "Internal processor for composites processors"_help,
            false  // Visible
        };
    }
};

// Specialize to pass on the manager
template <>
CompositeSink<BrushingAndLinkingInport, BrushingAndLinkingOutport>::CompositeSink()
    : CompositeSinkBase(), inport_{"inport"}, superOutport_{"outport"} {
    addPort(inport_);
    addPortToGroup(&superOutport_, "default");

    superOutport_.getManager().setParent(&inport_.getManager());
}

// Specialize to do nothing
template <>
void CompositeSink<BrushingAndLinkingInport, BrushingAndLinkingOutport>::process() {}

template <>
struct ProcessorTraits<
    SequenceCompositeSource<BrushingAndLinkingInport, BrushingAndLinkingOutport>> {
    static ProcessorInfo getProcessorInfo() {
        constexpr auto name = std::string_view{"BrushingAndLinking Meta Sequence Source"};
        auto id = util::appendIfNotEmpty(PortTraits<BrushingAndLinkingOutport>::classIdentifier(),
                                         SequenceCompositeSourceBase::identifierSuffix());
        return {
            id,                 // Class identifier
            std::string{name},  // Display name
            "Meta",             // Category
            CodeState::Stable,  // Code state
            "Meta",             // Tags
            "Internal processor for composites processors"_help,
            false  // Visible
        };
    }
};

// Specialize to pass on the manager
template <>
void SequenceCompositeSource<BrushingAndLinkingInport,
                             BrushingAndLinkingOutport>::createSuperInport(std::string_view id,
                                                                           bool optional) {
    superInport_ = std::make_shared<BrushingAndLinkingInport>(id);
    superInport_->setOptional(optional);
    addPortToGroup(superInport_.get(), "default");
    outport_.getManager().setParent(&superInport_->getManager());
}

// Specialize to do nothing
template <>
void SequenceCompositeSource<BrushingAndLinkingInport, BrushingAndLinkingOutport>::process() {}

template <>
size_t SequenceCompositeSource<BrushingAndLinkingInport, BrushingAndLinkingOutport>::sequenceSize()
    const {
    return 0;
}

template <>
std::shared_ptr<Processor> SequenceCompositeSource<
    BrushingAndLinkingInport, BrushingAndLinkingOutport>::createConverter() const {
    return nullptr;
}

template <>
void SequenceCompositeSource<BrushingAndLinkingInport, BrushingAndLinkingOutport>::sync(
    SequenceCompositeSourceBase* source) {

    if (source) {
        if (auto* typedSource = dynamic_cast<
                SequenceCompositeSource<BrushingAndLinkingInport, BrushingAndLinkingOutport>*>(
                source)) {
            // set the new parent before we assign the superInport_ since that is the current
            // parent.
            outport_.getManager().setParent(&typedSource->superInport_->getManager());
            superInport_ = typedSource->superInport_;
            addPortToGroup(superInport_.get(), "default");
        } else {
            throw Exception(SourceContext{}, "SequenceCompositeSinkBase of wrong type");
        }
    }
}

BrushingAndLinkingModule::BrushingAndLinkingModule(InviwoApplication* app)
    : InviwoModule(app, "BrushingAndLinking") {
    // Processors
    registerProcessor<BrushingAndLinkingProcessor>();
    registerProcessor<CompositeSource<BrushingAndLinkingInport, BrushingAndLinkingOutport>>();
    registerProcessor<CompositeSink<BrushingAndLinkingInport, BrushingAndLinkingOutport>>();

    registerProcessor<
        SequenceCompositeSource<BrushingAndLinkingInport, BrushingAndLinkingOutport>>();

    registerProcessor<PropertyToBrushing>();

    registerProcessor<SequenceBrush<Volume>>();
    registerProcessor<SequenceBrush<Layer>>();
    registerProcessor<SequenceBrush<Image>>();
    registerProcessor<SequenceBrush<Mesh>>();
    registerProcessor<SequenceBrush<BufferBase>>();

    // Ports
    registerPort<BrushingAndLinkingOutport>();
    registerPort<BrushingAndLinkingInport>();
}

}  // namespace inviwo
