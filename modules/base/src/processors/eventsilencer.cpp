/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#include <modules/base/processors/eventsilencer.h>

#include <inviwo/core/interaction/events/interactionevent.h>

namespace inviwo {

const ProcessorInfo EventSilencer::processorInfo_{
    "org.inviwo.EventSilencer",    // Class identifier
    "Event Silencer",              // Display name
    "None",                        // Category
    CodeState::Experimental,       // Code state
    Tags::CPU,                     // Tags
};
const ProcessorInfo EventSilencer::getProcessorInfo() const { return processorInfo_; }

EventSilencer::EventSilencer() : Processor()
    , imgIn_("imgIn")
    , imgOut_("imgOut")
    , meshIn_("meshIn")
    , meshOut_("meshOut")
    , polylineIn_("polylineIn")
    , polylineOut_("polylineOut")
    , volumeIn_("volumeIn")
    , volumeOut_("volumeOut") {

    imgIn_.setOptional(true);
    imgIn_.onChange([this]() { imgOut_.setData(imgIn_.getData()); });
    addPort(imgIn_);
    addPort(imgOut_);

    meshIn_.setOptional(true);
    meshIn_.onChange([this]() { meshOut_.setData(meshIn_.getData()); });
    addPort(meshIn_);
    addPort(meshOut_);

    polylineIn_.setOptional(true);
    polylineIn_.onChange([this]() { polylineOut_.setData(polylineIn_.getData()); });
    addPort(polylineIn_);
    addPort(polylineOut_);

    volumeIn_.setOptional(true);
    volumeIn_.onChange([this]() { volumeOut_.setData(volumeIn_.getData()); });
    addPort(volumeIn_);
    addPort(volumeOut_);
}

void EventSilencer::propagateEvent(Event* e, Outport* source) {
    if (dynamic_cast<InteractionEvent*>(e)) {
        return;
    }

    Processor::propagateEvent(e, source);
}

}  // namespace inviwo
