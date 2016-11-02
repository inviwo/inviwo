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
#include <inviwo/core/interaction/events/wheelevent.h>
#include <inviwo/core/interaction/events/gestureevent.h>
#include <inviwo/core/interaction/events/touchevent.h>

namespace inviwo {

PickingEvent::PickingEvent(const PickingAction* pickingAction, PickingState state, Event* event,
                           dvec3 pressNDC, dvec3 previousNDC, size_t pickedId)
    : Event()
    , pickingAction_(pickingAction)
    , state_(state)
    , event_{event, [](Event* e) {}}
    , ownsEvent_(false)
    , pressedNDC_(pressNDC)
    , previousNDC_(previousNDC)
    , pickedId_(pickedId) {}

PickingEvent::PickingEvent(const PickingAction* pickingAction, PickingState state,
                           std::unique_ptr<Event> event, dvec3 pressNDC, dvec3 previousNDC,
                           size_t pickedId)
    : Event()
    , pickingAction_(pickingAction)
    , state_(state)
    , event_{event.release(), [](Event* e) { delete e; }}
    , ownsEvent_(false)
    , pressedNDC_(pressNDC)
    , previousNDC_(previousNDC)
    , pickedId_(pickedId) {}

PickingEvent::PickingEvent(const PickingEvent& rhs)
    : pickingAction_(rhs.pickingAction_)
    , state_(rhs.state_)
    , event_(rhs.event_->clone(), [](Event* event) {delete event;})
    , ownsEvent_(true)
    , pressedNDC_(rhs.pressedNDC_) 
    , previousNDC_(rhs.previousNDC_)
    , pickedId_(rhs.pickedId_) {}

PickingEvent& PickingEvent::operator=(const PickingEvent& that) {
    if (this != &that) {
        pickingAction_ = that.pickingAction_;
        state_ = that.state_;
        event_ = EventPtr(that.event_->clone(), [](Event* event) {delete event;});
        ownsEvent_ = true;
        pressedNDC_ = that.pressedNDC_;
        previousNDC_ = that.previousNDC_;
        pickedId_ = that.pickedId_;
    }
    return *this;
}

PickingEvent::~PickingEvent() = default;

PickingEvent* PickingEvent::clone() const {
    return new PickingEvent(*this);
};

uint64_t PickingEvent::hash() const {
    return chash();
}


Event* PickingEvent::getEvent() const {
    return event_.get();
}

size_t PickingEvent::getPickedId() const {
    return pickedId_;
}

dvec2 PickingEvent::getPosition() const {
    if(event_) {
        switch (event_->hash()) {
            case MouseEvent::chash():
                return static_cast<MouseEvent*>(event_.get())->posNormalized();
            case WheelEvent::chash():
                return static_cast<WheelEvent*>(event_.get())->posNormalized();
            case GestureEvent::chash():
                return static_cast<GestureEvent*>(event_.get())->screenPosNormalized();
            case TouchEvent::chash():
                return static_cast<TouchEvent*>(event_.get())->centerPointNormalized();
        }  
    } 
    return dvec2(0.0f);
}

double PickingEvent::getDepth() const {
    if (event_) {
        switch (event_->hash()) {
            case MouseEvent::chash():
                return static_cast<MouseEvent*>(event_.get())->depth();
            case WheelEvent::chash():
                return static_cast<WheelEvent*>(event_.get())->depth();
            case GestureEvent::chash():
                return static_cast<GestureEvent*>(event_.get())->depth();
            case TouchEvent::chash():
                return static_cast<TouchEvent*>(event_.get())->averageDepth();
        }
    }
    return 1.0f;
}

uvec2 PickingEvent::getCanvasSize() const {
    if (event_) {
        switch (event_->hash()) {
            case MouseEvent::chash():
                return static_cast<MouseEvent*>(event_.get())->canvasSize();
            case WheelEvent::chash():
                return static_cast<WheelEvent*>(event_.get())->canvasSize();
            case GestureEvent::chash():
                return static_cast<GestureEvent*>(event_.get())->canvasSize();
            case TouchEvent::chash():
                return static_cast<TouchEvent*>(event_.get())->canvasSize();
        }
    }
    return uvec2(0);
}

dvec2 PickingEvent::getPreviousPosition() const {
    return dvec2(0.5) * (previousNDC_.xy() + dvec2(1.0));
}


double PickingEvent::getPreviousDepth() const {
    return previousNDC_.z;
}


dvec2 PickingEvent::getPressedPosition() const {
    return dvec2(0.5) * (pressedNDC_.xy() + dvec2(1.0));
}


double PickingEvent::getPressedDepth() const {
    return pressedNDC_.z;
}

dvec2 PickingEvent::getDeltaPosition() const {
    return getPosition() - getPreviousPosition();
}

double PickingEvent::getDeltaDepth() const {
    return getDepth() - previousNDC_.z;
}

dvec2 PickingEvent::getDeltaPressedPosition() const {
    return getPosition() - getPressedPosition();
}

double PickingEvent::getDeltaPressedDepth() const {
    return getDepth() - pressedNDC_.z;
}

dvec3 PickingEvent::getNDC() const {
    if (event_) {
        switch (event_->hash()) {
            case MouseEvent::chash():
                return static_cast<MouseEvent*>(event_.get())->ndc();
            case WheelEvent::chash():
                return static_cast<WheelEvent*>(event_.get())->ndc();
            case GestureEvent::chash():
                return static_cast<GestureEvent*>(event_.get())->ndc();
            case TouchEvent::chash():
                return static_cast<TouchEvent*>(event_.get())->centerNDC();
        }
    }
    return dvec3(0.0f);
}

dvec3 PickingEvent::getPreviousNDC() const {
    return previousNDC_;
}

dvec3 PickingEvent::getPressedNDC() const {
    return pressedNDC_;
}



PickingState PickingEvent::getState() const {
    return state_;
}

void PickingEvent::invoke(Processor* p) {
    if (p == pickingAction_->getProcessor() && pickingAction_->isEnabled()) {
        (*pickingAction_)(this);
    }
}


const inviwo::PickingAction* PickingEvent::getPickingAction() const {
    return pickingAction_;
}

} // namespace

