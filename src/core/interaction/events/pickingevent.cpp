/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/interaction/pickingaction.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/touchevent.h>

namespace inviwo {

PickingEvent::PickingEvent(const PickingAction* pickingAction, PickingState state, Event* event,
                           dvec3 pressNDC, dvec3 previousNDC, size_t pickedId)
    : Event()
    , pickingAction_(pickingAction)
    , state_(state)
    , event_(event)
    , pressNDC_(pressNDC)
    , previousNDC_(previousNDC)
    , pickedId_(pickedId) {}


PickingEvent* PickingEvent::clone() const {
    return new PickingEvent(*this);
};

uint64_t PickingEvent::hash() const {
    return chash();
}


size_t PickingEvent::getPickedId() const {
    return pickedId_;
}

dvec2 PickingEvent::getPosition() const {
    if(event_) {
        switch (event_->hash()) {
            case MouseEvent::chash():
                return static_cast<MouseEvent*>(event_)->posNormalized();
            case TouchEvent::chash():
                return static_cast<TouchEvent*>(event_)->getCenterPointNormalized();
        }  
    } 
    return dvec2(0.0f);
}

double PickingEvent::getDepth() const {
    if (event_) {
        switch (event_->hash()) {
            case MouseEvent::chash():
                return static_cast<MouseEvent*>(event_)->depth();
            case TouchEvent::chash():
                return static_cast<TouchEvent*>(event_)->getTouchPoints().front().getDepth();
        }
    }
    return 0.0f;
}

dvec2 PickingEvent::getPreviousPosition() const {
    return dvec2(0.5) * (previousNDC_.xy() + dvec2(1.0));
}


double PickingEvent::getPreviousDepth() const {
    return previousNDC_.z;
}


dvec2 PickingEvent::getPressPosition() const {
    return dvec2(0.5) * (pressNDC_.xy() + dvec2(1.0));
}


double PickingEvent::getPressDepth() const {
    return pressNDC_.z;
}

dvec2 PickingEvent::getDeltaPosition() const {
    return getPosition() - getPreviousPosition();
}

double PickingEvent::getDeltaDepth() const {
    return getDepth() - previousNDC_.z;
}

dvec2 PickingEvent::getDeltaPressPosition() const {
    return getPosition() - getPressPosition();
}

double PickingEvent::getDeltaPressDepth() const {
    return getDepth() - pressNDC_.z;
}

dvec3 PickingEvent::getNDC() const {
    if (event_) {
        switch (event_->hash()) {
            case MouseEvent::chash():
                return static_cast<MouseEvent*>(event_)->ndc();
            //case TouchEvent::chash():
            //    return static_cast<TouchEvent*>(event_)->getCenterPointNormalized();
        }
    }
    return dvec3(0.0f);
}

dvec3 PickingEvent::getPreviousNDC() const {
    return previousNDC_;
}

dvec3 PickingEvent::getPressNDC() const {
    return pressNDC_;
}

PickingState PickingEvent::getState() const {
    return state_;
}

void PickingEvent::invoke(Processor* p) const {
    if (p == pickingAction_->getProcessor() && pickingAction_->isEnabled()) {
        (*pickingAction_)(this);
    }
}


} // namespace

