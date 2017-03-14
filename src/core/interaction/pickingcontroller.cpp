/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

PickingController::PickingController() = default;

void PickingController::propagateEvent(Event* event, EventPropagator* propagator) {
    if (!propagator) return;
    if (!event) return;

    switch (event->hash()) {
        case MouseEvent::chash(): {
            propagateEvent(static_cast<MouseEvent*>(event), propagator);
            break;
        }
        case WheelEvent::chash(): {
            propagateEvent(static_cast<WheelEvent*>(event), propagator);
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

void PickingController::propagateEvent(MouseInteractionEvent* e, EventPropagator* propagator) {
    auto ndc = e->ndc();
    auto pa = mstate_.update(*this, e);

    // Check if we have switched picking id, if so send a Finished event
    if (mstate_.previousPickingAction.action &&
        pa.index != mstate_.previousPickingAction.index) {
        auto localId = mstate_.previousPickingAction.action->getLocalPickingId(
            mstate_.previousPickingAction.index);
        PickingEvent pickingEvent(mstate_.previousPickingAction.action,
                                  PickingState::Finished, e, mstate_.pressNDC,
                                  mstate_.previousNDC, localId);
        propagator->propagateEvent(&pickingEvent, nullptr);
    }

    // if there was a picking action, then propagate a picking event.
    if (pa.action) {
        auto ps = pa.index == mstate_.previousPickingAction.index ? PickingState::Updated
            : PickingState::Started;

        auto localId = pa.action->getLocalPickingId(pa.index);
        PickingEvent pickingEvent(pa.action, ps, e, mstate_.pressNDC, mstate_.previousNDC, localId);
        propagator->propagateEvent(&pickingEvent, nullptr);
        if (pickingEvent.hasBeenUsed()) e->markAsUsed();
    }

    mstate_.previousPickingAction = pa;
    mstate_.previousNDC = ndc;
}

void PickingController::propagateEvent(TouchEvent* e, EventPropagator* propagator) {
    auto& touchPoints = e->touchPoints();

    std::unordered_map<size_t, std::vector<TouchPoint>> pickngIdToTouchPoints;
    std::unordered_map<size_t, const PickingAction*> pickingIdToAction;
    std::vector<int> usedPointIds;

    for (auto& point : touchPoints) {
        auto coord = glm::clamp(point.pos(), dvec2(0.0), dvec2(e->canvasSize() - uvec2(1)));
        if (point.state() == TouchState::Started) {
            auto pa = findPickingAction(coord);
            tstate_.pointIdToPickingId[point.id()] = pa;
            tstate_.pickingIdToPressNDC[pa.index] = point.ndc();
        }

        auto it = tstate_.pointIdToPickingId.find(point.id());       
        auto pa = it != tstate_.pointIdToPickingId.end() ? it->second : findPickingAction(coord);

        if (pa.action) {
            pickngIdToTouchPoints[pa.index].push_back(point);
            pickingIdToAction[pa.index] = pa.action;
        }
    }

    // Propagate Stated and Updated picking events
    for (auto& item : pickngIdToTouchPoints) {
        const auto& pickingId = item.first;
        const auto& points = item.second;

        auto ps = TouchEvent::getPickingState(points);

        TouchEvent te(points);
        auto prevPos = te.centerNDC(); // Need so save here since te might be modified
        auto localId = pickingIdToAction[pickingId]->getLocalPickingId(pickingId);
        PickingEvent pickingEvent(pickingIdToAction[pickingId], ps, &te,
                                  tstate_.pickingIdToPressNDC[pickingId],
                                  tstate_.pickingIdToPreviousNDC[pickingId], localId);
        propagator->propagateEvent(&pickingEvent, nullptr);
        if (pickingEvent.hasBeenUsed() || te.hasBeenUsed()) {
            for (const auto& p : points) usedPointIds.push_back(p.id());
        }

        tstate_.pickingIdToPreviousNDC[pickingId] = prevPos;
    }

    // Cleanup
    for (auto& point : touchPoints) {
        if (point.state() == TouchState::Finished) {
            tstate_.pointIdToPickingId.erase(point.id());
        }
    }

    util::map_erase_remove_if(tstate_.pickingIdToPressNDC, [&](const auto& item) {
        return pickingIdToAction.find(item.first) == pickingIdToAction.end();
    });
    util::map_erase_remove_if(tstate_.pickingIdToPreviousNDC, [&](const auto& item) {
        return pickingIdToAction.find(item.first) == pickingIdToAction.end();
    });

    // remove the "used" points from the event
    util::erase_remove_if(touchPoints, [&](const auto& p) {
        return util::contains(usedPointIds, p.id());
    });
    if (touchPoints.empty()) e->markAsUsed();
}

void PickingController::propagateEvent(GestureEvent* event, EventPropagator* propagator) {
    // TODO...
}

void PickingController::setPickingSource(const std::shared_ptr<const Image>& src) { src_ = src; }

PickingManager::Result PickingController::findPickingAction(const uvec2& coord) {
    if (src_ && pickingEnabled()) {
        auto value = src_->readPixel(size2_t(coord), LayerType::Picking);
        if (value.a > 0.0) {
            return PickingManager::getPtr()->getPickingActionFromColor(uvec3(value));
        }
    }
    return {0, nullptr};
}

bool PickingController::pickingEnabled() const {
    return PickingManager::getPtr()->pickingEnabled();
}

PickingManager::Result PickingController::PCMouseState::update(PickingController& pc,
                                                        MouseInteractionEvent* e) {
    auto coord = glm::clamp(e->pos(), dvec2(0.0), dvec2(e->canvasSize() - uvec2(1)));

    // Toggle states
    if (!mousePressed && e->buttonState() != MouseButton::None) {
        mousePressed = true;
        pressedPickingAction = pc.findPickingAction(coord);
        pressNDC = e->ndc();
    } else if (mousePressed && e->buttonState() == MouseButton::None) {
        mousePressed = false;
        pressedPickingAction = {0, nullptr};
        pressNDC = dvec3(0.0);
    }

    return mousePressed ? pressedPickingAction : pc.findPickingAction(coord);
}

} // namespace

