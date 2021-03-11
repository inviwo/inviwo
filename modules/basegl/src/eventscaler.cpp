/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <modules/basegl/eventscaler.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/gestureevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <inviwo/core/interaction/events/wheelevent.h>
#include <inviwo/core/interaction/events/pickingevent.h>

namespace inviwo {

EventScaler::EventScaler(uvec2 inportImageSize) : size_{inportImageSize} {}

bool EventScaler::propagateEvent(Event* event, Propagator propagator) {
    switch (event->hash()) {
        case PickingEvent::chash(): {
            return propagatePickingEvent(static_cast<PickingEvent*>(event), propagator);
        }
        case MouseEvent::chash(): {
            return propagateMouseEvent(static_cast<MouseEvent*>(event), propagator);
        }
        case WheelEvent::chash(): {
            return propagateWheelEvent(static_cast<WheelEvent*>(event), propagator);
        }
        case GestureEvent::chash(): {
            return propagateGestureEvent(static_cast<GestureEvent*>(event), propagator);
        }
        case TouchEvent::chash(): {
            return propagateTouchEvent(static_cast<TouchEvent*>(event), propagator);
        }
        default:
            return false;
    }
}

bool EventScaler::propagatePickingEvent(PickingEvent* pe, Propagator propagator) {

    auto prop = [&](Event* newEvent) {
        if (newEvent) {
            PickingEvent newPe(pe->getPickingAction(), static_cast<InteractionEvent*>(newEvent),
                               pe->getState(), pe->getPressState(), pe->getPressItem(),
                               pe->getHoverState(), pe->getPressItems(), pe->getGlobalPickingId(),
                               pe->getCurrentGlobalPickingId(), pe->getPressedGlobalPickingId(),
                               pe->getPreviousGlobalPickingId(), pe->getPressedNDC(),
                               pe->getPreviousNDC());

            propagator(&newPe);
            if (newPe.hasBeenUsed()) newEvent->markAsUsed();
            for (auto p : newPe.getVisitedProcessors()) newEvent->markAsVisited(p);
        }
    };

    auto e = pe->getEvent();
    bool propagated = false;
    switch (e->hash()) {
        case MouseEvent::chash():
            propagated = propagateMouseEvent(static_cast<MouseEvent*>(e), prop);
            break;
        case WheelEvent::chash():
            propagated = propagateWheelEvent(static_cast<WheelEvent*>(e), prop);
            break;
        case GestureEvent::chash():
            propagated = propagateGestureEvent(static_cast<GestureEvent*>(e), prop);
            break;
        case TouchEvent::chash():
            propagated = propagateTouchEvent(static_cast<TouchEvent*>(e), prop);
            break;
        default:
            propagated = false;
            break;
    }
    if (e->hasBeenUsed()) pe->markAsUsed();
    for (auto p : e->getVisitedProcessors()) pe->markAsVisited(p);

    return propagated;
}

bool EventScaler::propagateMouseEvent(MouseEvent* me, Propagator propagator) {
    MouseEvent newEvent(*me);
    newEvent.setCanvasSize(size_);
    propagator(&newEvent);
    if (newEvent.hasBeenUsed()) me->markAsUsed();
    for (auto p : newEvent.getVisitedProcessors()) me->markAsVisited(p);

    return true;
}

bool EventScaler::propagateWheelEvent(WheelEvent* we, Propagator propagator) {
    WheelEvent newEvent(*we);
    newEvent.setCanvasSize(size_);
    propagator(&newEvent);
    if (newEvent.hasBeenUsed()) we->markAsUsed();
    for (auto p : newEvent.getVisitedProcessors()) we->markAsVisited(p);
    return true;
}

bool EventScaler::propagateGestureEvent(GestureEvent* ge, Propagator propagator) {
    GestureEvent newEvent(*ge);
    newEvent.setCanvasSize(size_);
    propagator(&newEvent);
    if (newEvent.hasBeenUsed()) ge->markAsUsed();
    for (auto p : newEvent.getVisitedProcessors()) ge->markAsVisited(p);

    return true;
}

bool EventScaler::propagateTouchEvent(TouchEvent* te, Propagator propagator) {

    auto touchPoints{te->touchPoints()};
    for (auto& p : touchPoints) {
        p.setCanvasSize(size_);
    }

    TouchEvent newEvent(touchPoints, te->getDevice(), te->modifiers());
    propagator(&newEvent);
    if (newEvent.hasBeenUsed()) te->markAsUsed();
    for (auto p : newEvent.getVisitedProcessors()) te->markAsVisited(p);

    return true;
}
}  // namespace inviwo
