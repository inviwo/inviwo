/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <inviwo/core/interaction/pickingcontroller.h>
#include <inviwo/core/interaction/pickingmanager.h>
#include <inviwo/core/interaction/pickingaction.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/interaction/events/eventpropagator.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/wheelevent.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <inviwo/core/interaction/events/gestureevent.h>
#include <inviwo/core/interaction/pickingstate.h>

namespace inviwo {

PickingController::PickingController() : mouseState_{PickingManager::getPtr()} {}

PickingController::~PickingController() = default;

void PickingController::propagateEvent(Event* event, EventPropagator* propagator) {
    if (!propagator) return;
    if (!event) return;
    if (!pickingEnabled()) return;

    switch (event->hash()) {
        case MouseEvent::chash(): {
            auto me = static_cast<MouseEvent*>(event);
            mouseState_.propagateEvent(me, propagator, pickId(me->pos()));
            break;
        }
        case WheelEvent::chash(): {
            auto me = static_cast<WheelEvent*>(event);
            mouseState_.propagateEvent(me, propagator, pickId(me->pos()));
            break;
        }
        case GestureEvent::chash(): {
            propagateEvent(static_cast<GestureEvent*>(event), propagator);
            break;
        }
        case TouchEvent::chash(): {
            propagateEvent(static_cast<TouchEvent*>(event), propagator);
            break;
        }
    }
}

void PickingController::propagateEvent(TouchEvent* e, EventPropagator* propagator) {
    if (!propagator) return;
    if (!e) return;
    if (!pickingEnabled()) return;
    // Strategy for treating touch points for picking objects
    //
    // One touch event per initially picked object.
    //  + Single PickingEvent (enter/move/leave) with multiple points for initially picked object
    //  + Picked objects will have access to multiple touch points
    //  - Points initially picking an object may split to multiple object (ignored here)
    //     - Mostly only relevant for touchpad where a touch point may not mean "button pressed"
    //     - Case:
    //            Two touch points over object A. One touch point moves over object B, both A and B
    //            should enter hover state - tricky to maintain state since the point state is
    //            treated as one!
    //
    //
    // Other alternative considered:
    // Treat each touch point individually.
    //  + Easy to track state
    //  - Multiple picking events (Enter/Leave/Press) per object possible
    //  - PickingEvent will only contain a single touch point, making it difficult to handle
    //  multiple touch points.
    //

    // Strategy: One touch event per initially picked object.
    auto& touchPoints = e->touchPoints();
    std::vector<int> usedPointIds;
    std::unordered_map<size_t, std::vector<TouchPoint>> pickingIdToTouchPoints;

    for (auto& point : touchPoints) {

        auto it = touchPointStartPickingId_.find(point.id());
        // Not sure if this check is necessary anymore (I recall that there previously have been
        // instabilities)
        if (it == touchPointStartPickingId_.end()) {
            // Should mean that point.state() == TouchState::Started
            touchPointStartPickingId_[point.id()] = pickId(point.pos());
            it = touchPointStartPickingId_.find(point.id());
        }

        auto pickedId = it->second;
        if (touchStates_.find(pickedId) == touchStates_.end()) {
            touchStates_[pickedId] = PickingControllerTouchState{PickingManager::getPtr()};
        }
        pickingIdToTouchPoints[it->second].push_back(point);
    }

    for (auto& item : pickingIdToTouchPoints) {
        const auto& startPickingId = item.first;
        const auto& points = item.second;
        TouchEvent te(points, e->getDevice(), e->modifiers());
        // Heuristic for obtaining picked object among the touch points:
        // Select first non-background object (points are generally sorted in the order they where
        // pressed)
        size_t pickedId = PickingManager::VoidId;
        for (const auto& point : points) {
            pickedId = pickId(point.pos());
            if (pickedId != PickingManager::VoidId) {
                break;
            }
        }
        touchStates_[startPickingId].propagateEvent(&te, propagator, pickedId);
        if (te.hasBeenUsed()) {
            for (const auto& p : te.touchPoints()) {
                usedPointIds.push_back(p.id());
            }
        }

        if (TouchEvent::getPickingState(points) == PickingState::Finished) {
            touchStates_.erase(startPickingId);
        }
    }
    // Cleanup
    for (auto& point : touchPoints) {
        if (point.state() == TouchState::Finished) {
            touchPointStartPickingId_.erase(point.id());
        }
    }

    // remove the "used" points from the event
    std::erase_if(touchPoints, [&](const auto& p) { return util::contains(usedPointIds, p.id()); });
    if (touchPoints.empty()) e->markAsUsed();
}

void PickingController::propagateEvent(GestureEvent*, EventPropagator*) {
    // TODO
}

void PickingController::setPickingSource(const std::shared_ptr<const Image>& src) { src_ = src; }

size_t PickingController::pickId(const dvec2& coord) {
    if (auto src = src_.lock()) {
        if (auto dim = src->getDimensions();
            glm::any(glm::lessThan(coord, dvec2{0}) ||
                     glm::greaterThan(coord, dvec2{dim} - dvec2{1.0}))) {
            return PickingManager::VoidId;
        }
        if (const auto value = src->readPixel(uvec2{coord}, LayerType::Picking); value.a > 0.0) {
            return PickingManager::colorToIndex(uvec3(value));
        }
    }
    return PickingManager::VoidId;
}

bool PickingController::pickingEnabled() const {
    return PickingManager::getPtr()->pickingEnabled();
}

}  // namespace inviwo
