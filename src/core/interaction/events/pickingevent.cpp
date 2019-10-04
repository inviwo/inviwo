/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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
#include <inviwo/core/datastructures/camera.h>
#include <inviwo/core/interaction/events/eventutil.h>

namespace inviwo {

PickingEvent::PickingEvent(const PickingAction* pickingAction, InteractionEvent* event,
                           PickingState state, PickingPressState pressState,
                           PickingPressItem pressItem, PickingHoverState hoverState,
                           PickingPressItems pressedState, size_t pickedGlobalId,
                           size_t currentGlobalId, size_t pressedGlobalId, size_t previousGlobalId,
                           dvec3 pressedNDC, dvec3 previousNDC)
    : Event()
    , pickingAction_(pickingAction)
    , owner_{nullptr}
    , event_{event}
    , state_{state}
    , pressState_{pressState}
    , pressItem_{pressItem}
    , hoverState_{hoverState}
    , pressedState_{pressedState}
    , pickedGlobalId_{pickedGlobalId}
    , currentGlobalId_{currentGlobalId}
    , pressedGlobalId_{pressedGlobalId}
    , previousGlobalId_{previousGlobalId}
    , pressedNDC_{pressedNDC}
    , previousNDC_{previousNDC} {}

PickingEvent::PickingEvent(const PickingAction* pickingAction,
                           std::unique_ptr<InteractionEvent> event, PickingState state,
                           PickingPressState pressState, PickingPressItem pressItem,
                           PickingHoverState hoverState, PickingPressItems pressedState,
                           size_t pickedGlobalId, size_t currentGlobalId, size_t pressedGlobalId,
                           size_t previousGlobalId, dvec3 pressedNDC, dvec3 previousNDC)
    : PickingEvent(pickingAction, event.get(), state, pressState, pressItem, hoverState,
                   pressedState, pickedGlobalId, currentGlobalId, pressedGlobalId, previousGlobalId,
                   pressedNDC, previousNDC) {
    owner_ = std::move(event);
}

PickingEvent::PickingEvent(const PickingEvent& rhs)
    : Event(rhs)
    , pickingAction_{rhs.pickingAction_}
    , owner_{rhs.event_->clone()}
    , event_{owner_.get()}
    , state_{rhs.state_}
    , pressState_{rhs.pressState_}
    , pressItem_{rhs.pressItem_}
    , hoverState_{rhs.hoverState_}
    , pressedState_{rhs.pressedState_}
    , pickedGlobalId_{rhs.pickedGlobalId_}
    , currentGlobalId_{rhs.currentGlobalId_}
    , pressedGlobalId_{rhs.pressedGlobalId_}
    , previousGlobalId_{rhs.previousGlobalId_}
    , pressedNDC_{rhs.pressedNDC_}
    , previousNDC_{rhs.previousNDC_} {}

PickingEvent& PickingEvent::operator=(const PickingEvent& that) {
    if (this != &that) {
        Event::operator=(that);
        pickingAction_ = that.pickingAction_;
        owner_.reset(that.event_->clone());
        event_ = owner_.get();
        state_ = that.state_;
        pressState_ = that.pressState_;
        pressItem_ = that.pressItem_;
        hoverState_ = that.hoverState_;
        pressedState_ = that.pressedState_;
        pickedGlobalId_ = that.pickedGlobalId_;
        currentGlobalId_ = that.currentGlobalId_;
        pressedGlobalId_ = that.pressedGlobalId_;
        previousGlobalId_ = that.previousGlobalId_;
        pressedNDC_ = that.pressedNDC_;
        previousNDC_ = that.previousNDC_;
    }
    return *this;
}

PickingEvent::~PickingEvent() = default;

PickingEvent* PickingEvent::clone() const { return new PickingEvent(*this); }

uint64_t PickingEvent::hash() const { return chash(); }

InteractionEvent* PickingEvent::getEvent() const { return event_; }

size_t PickingEvent::getGlobalPickingId() const { return pickedGlobalId_; }
size_t PickingEvent::getCurrentGlobalPickingId() const { return currentGlobalId_; }
size_t PickingEvent::getPreviousGlobalPickingId() const { return previousGlobalId_; }
size_t PickingEvent::getPressedGlobalPickingId() const { return pressedGlobalId_; }

size_t PickingEvent::getPickedId() const {
    return pickingAction_->getLocalPickingId(pickedGlobalId_);
}

std::pair<bool, size_t> PickingEvent::getCurrentLocalPickingId() const {
    if (pickingAction_ && pickingAction_->isIndex(currentGlobalId_)) {
        return {true, pickingAction_->getLocalPickingId(currentGlobalId_)};
    } else {
        return {false, 0};
    }
}
std::pair<bool, size_t> PickingEvent::getPreviousLocalPickingId() const {
    if (pickingAction_ && pickingAction_->isIndex(previousGlobalId_)) {
        return {true, pickingAction_->getLocalPickingId(previousGlobalId_)};
    } else {
        return {false, 0};
    }
}
std::pair<bool, size_t> PickingEvent::getPressedLocalPickingId() const {
    if (pickingAction_ && pickingAction_->isIndex(pressedGlobalId_)) {
        return {true, pickingAction_->getLocalPickingId(pressedGlobalId_)};
    } else {
        return {false, 0};
    }
}

dvec2 PickingEvent::getPosition() const {
    if (event_) {
        switch (event_->hash()) {
            case MouseEvent::chash():
                return static_cast<MouseEvent*>(event_)->posNormalized();
            case WheelEvent::chash():
                return static_cast<WheelEvent*>(event_)->posNormalized();
            case GestureEvent::chash():
                return static_cast<GestureEvent*>(event_)->screenPosNormalized();
            case TouchEvent::chash():
                return static_cast<TouchEvent*>(event_)->centerPointNormalized();
        }
    }
    return dvec2(0.0f);
}

double PickingEvent::getDepth() const {
    if (event_) {
        switch (event_->hash()) {
            case MouseEvent::chash():
                return static_cast<MouseEvent*>(event_)->depth();
            case WheelEvent::chash():
                return static_cast<WheelEvent*>(event_)->depth();
            case GestureEvent::chash():
                return static_cast<GestureEvent*>(event_)->depth();
            case TouchEvent::chash():
                return static_cast<TouchEvent*>(event_)->averageDepth();
        }
    }
    return 1.0f;
}

uvec2 PickingEvent::getCanvasSize() const {
    if (event_) {
        switch (event_->hash()) {
            case MouseEvent::chash():
                return static_cast<MouseEvent*>(event_)->canvasSize();
            case WheelEvent::chash():
                return static_cast<WheelEvent*>(event_)->canvasSize();
            case GestureEvent::chash():
                return static_cast<GestureEvent*>(event_)->canvasSize();
            case TouchEvent::chash():
                return static_cast<TouchEvent*>(event_)->canvasSize();
        }
    }
    return uvec2(0);
}

dvec2 PickingEvent::getPreviousPosition() const {
    return dvec2(0.5) * (dvec2(previousNDC_) + dvec2(1.0));
}

double PickingEvent::getPreviousDepth() const { return previousNDC_.z; }

dvec2 PickingEvent::getPressedPosition() const {
    return dvec2(0.5) * (dvec2(pressedNDC_) + dvec2(1.0));
}

double PickingEvent::getPressedDepth() const { return pressedNDC_.z; }

dvec2 PickingEvent::getDeltaPosition() const { return getPosition() - getPreviousPosition(); }

double PickingEvent::getDeltaDepth() const { return getDepth() - previousNDC_.z; }

dvec2 PickingEvent::getDeltaPressedPosition() const { return getPosition() - getPressedPosition(); }

double PickingEvent::getDeltaPressedDepth() const { return getDepth() - pressedNDC_.z; }

dvec3 PickingEvent::getNDC() const {
    if (event_) {
        switch (event_->hash()) {
            case MouseEvent::chash():
                return static_cast<MouseEvent*>(event_)->ndc();
            case WheelEvent::chash():
                return static_cast<WheelEvent*>(event_)->ndc();
            case GestureEvent::chash():
                return static_cast<GestureEvent*>(event_)->ndc();
            case TouchEvent::chash():
                return static_cast<TouchEvent*>(event_)->centerNDC();
        }
    }
    return dvec3(0.0f);
}

dvec3 PickingEvent::getPreviousNDC() const { return previousNDC_; }

dvec3 PickingEvent::getPressedNDC() const { return pressedNDC_; }

dvec3 PickingEvent::getWorldSpaceDeltaAtPressDepth(const Camera& camera) const {
    auto currNDC = getNDC();
    auto prevNDC = getPreviousNDC();

    // Use depth of initial press as reference to move in the image plane.
    auto refDepth = getPressedDepth();
    currNDC.z = refDepth;
    prevNDC.z = refDepth;

    auto corrWorld = camera.getWorldPosFromNormalizedDeviceCoords(static_cast<vec3>(currNDC));
    auto prevWorld = camera.getWorldPosFromNormalizedDeviceCoords(static_cast<vec3>(prevNDC));
    return (corrWorld - prevWorld);
}

PickingState PickingEvent::getState() const { return state_; }
PickingPressState PickingEvent::getPressState() const { return pressState_; }
PickingPressItem PickingEvent::getPressItem() const { return pressItem_; }
PickingHoverState PickingEvent::getHoverState() const { return hoverState_; }
PickingPressItems PickingEvent::getPressItems() const { return pressedState_; }

KeyModifiers PickingEvent::modifiers() const {
    if (event_) return event_->modifiers();
    return KeyModifiers{};
}

void PickingEvent::invoke(Processor* p) {
    if (p == pickingAction_->getProcessor() && pickingAction_->isEnabled()) {
        (*pickingAction_)(this);
    }
}

const InteractionEvent::ToolTipCallback& PickingEvent::getToolTipCallback() const {
    return event_->getToolTipCallback();
}

void PickingEvent::setToolTip(const std::string& tooltip) const { event_->setToolTip(tooltip); }

const PickingAction* PickingEvent::getPickingAction() const { return pickingAction_; }

void PickingEvent::print(std::ostream& ss) const {
    util::printEvent(
        ss, "PickingEvent", std::make_pair("state", state_),
        std::make_pair("pressState", pressState_), std::make_pair("pressItem", pressItem_),
        std::make_pair("hoverState", hoverState_), std::make_pair("pressedState", pressedState_),
        std::make_tuple("GlobalID"), std::make_tuple("pick", pickedGlobalId_, 4),
        std::make_tuple("curr", currentGlobalId_, 4), std::make_tuple("pres", pressedGlobalId_, 4),
        std::make_tuple("prev", previousGlobalId_, 4), std::make_pair("NDC", getNDC()));
}

}  // namespace inviwo
