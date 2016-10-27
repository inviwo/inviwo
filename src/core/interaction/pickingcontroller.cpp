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
#include <inviwo/core/interaction/pickingstate.h>

namespace inviwo {

PickingController::PickingController() = default;

void PickingController::handlePickingEvent(EventPropagator* propagator, Event* event) {
    if (!propagator) return;
    if (!event) return;

    switch (event->hash()) {
        case MouseEvent::chash(): {
            handlePickingEvent(propagator, static_cast<MouseEvent*>(event));
            break;
        }
        case WheelEvent::chash(): {
            handlePickingEvent(propagator, static_cast<WheelEvent*>(event));
            break;
        }
        case GestureEvent::chash(): {
            handlePickingEvent(propagator, static_cast<GestureEvent*>(event));
            break;
        }
        case TouchEvent::chash(): {
            handlePickingEvent(propagator, static_cast<TouchEvent*>(event));
            break;
        }
    }
}

void PickingController::handlePickingEvent(EventPropagator* propagator, MouseInteractionEvent* e) {
    auto ndc = e->ndc();
    auto pa = state_.update(*this, e);

    // Check if we have switched picking id, if so send a Finished event
    if (state_.previousPickingAction.action &&
        pa.index != state_.previousPickingAction.index) {
        PickingEvent pickingEvent(state_.previousPickingAction.action,
                                  PickingState::Finished, e, state_.pressNDC,
                                  state_.previousNDC, state_.previousPickingAction.index);
        propagator->propagateEvent(&pickingEvent, nullptr);
        if (pickingEvent.hasBeenUsed()) e->markAsUsed();
    }

    // if there was a picking action, then propagate a picking event.
    if (pa.action) {
        auto ps = pa.index == state_.previousPickingAction.index ? PickingState::Updated
            : PickingState::Started;

        PickingEvent pickingEvent(pa.action, ps, e, state_.pressNDC, state_.previousNDC,
                                  pa.index);
        propagator->propagateEvent(&pickingEvent, nullptr);
        if (pickingEvent.hasBeenUsed()) e->markAsUsed();
    }

    state_.previousPickingAction = pa;
    state_.previousNDC = ndc;
}

void PickingController::handlePickingEvent(EventPropagator* propagator, TouchEvent* e) {
    /* TODO   Old version from the PickingContainer....
    std::vector<TouchPoint>& touchPoints = e->getTouchPoints();

    // Clear the picked touch point map
    pickedTouchPoints_.clear();

    if (touchPoints.size() > 1 || touchPoints[0].state() != TouchState::Finished)
        touchPickingOn_ = true;
    else
        touchPickingOn_ = false;

    std::unordered_map<int, PickingObject*>::iterator touchPickObjs_it;
    std::unordered_map<PickingObject*, std::vector<TouchPoint>>::iterator pickedTouchPoints_it;

    auto touchPoint = touchPoints.begin();
    while (touchPoint != touchPoints.end()) {
        bool isAssociated = false;
        if (touchPoint->state() == TouchState::Started) {
            // Find out if new touch point is touching inside a picking object
            uvec2 coord = clampToScreenCoords(touchPoint->getPos(), e->canvasSize());
            PickingObject* pickObj = findPickingObject(coord);

            // If it is, put it in the TouchIDPickingMap
            if (pickObj) {
                touchPickObjs_.insert(std::pair<int, PickingObject*>(touchPoint->getId(), pickObj));

                // Associate touch point with picking object
                // which can already have other associated touch points.
                pickedTouchPoints_it = pickedTouchPoints_.find(pickObj);
                if (pickedTouchPoints_it != pickedTouchPoints_.end()){
                    pickedTouchPoints_it->second.push_back(*touchPoint);
                }
                else{
                    pickedTouchPoints_.insert(std::pair<PickingObject*, 
                        std::vector<TouchPoint>>(pickObj, std::vector<TouchPoint>{*touchPoint}));
                }
                isAssociated = true;
            }
        }
        else if (touchPoint->state() == TouchState::Finished) {
            // Erase touch point from TouchIDPickingMap
            size_t numberOfErasedElements = touchPickObjs_.erase(touchPoint->getId());
            isAssociated = (numberOfErasedElements > 0);
        }
        else {
            // Find out if touch point is in the TouchIDPickingMap
            // If it exists, associate touch point with picking object
            touchPickObjs_it = touchPickObjs_.find(touchPoint->getId());
            if (touchPickObjs_it != touchPickObjs_.end()){
                // Associate touch point with picking object
                // which can already have other associated touch points.
                pickedTouchPoints_it = pickedTouchPoints_.find(touchPickObjs_it->second);
                if (pickedTouchPoints_it != pickedTouchPoints_.end()){
                    pickedTouchPoints_it->second.push_back(*touchPoint);
                }
                else{
                    pickedTouchPoints_.insert(std::pair<PickingObject*, 
                        std::vector<TouchPoint>>(touchPickObjs_it->second, std::vector<TouchPoint>{*touchPoint}));
                }
                isAssociated = true;
            }
        }
        // Removed touch point from the actual event if it was associated with a picking object
        if (isAssociated)
            touchPoint = touchPoints.erase(touchPoint);
        else
            ++touchPoint;
    }

    // Build touch event for all picking objects with associated touch points
    for (pickedTouchPoints_it = pickedTouchPoints_.begin(); pickedTouchPoints_it != pickedTouchPoints_.end(); ++pickedTouchPoints_it){
        // Treat one touch point the same as mouse event, for now
        if (pickedTouchPoints_it->second.size() == 1){
            uvec2 coord = clampToScreenCoords(pickedTouchPoints_it->second[0].getPos(), e->canvasSize());
            if (pickedTouchPoints_it->second[0].state() & TouchState::Started){
                pickedTouchPoints_it->first->setPosition(normalizedCoordinates(coord));
                pickedTouchPoints_it->first->setDepth(pickedTouchPoints_it->second[0].getDepth());
                pickedTouchPoints_it->first->setDelta(vec2(0.f, 0.f));
            }
            else{
                uvec2 prevCoord = clampToScreenCoords(pickedTouchPoints_it->second[0].getPrevPos(), e->canvasSize());
                pickedTouchPoints_it->first->setDelta(normalizedMovement(prevCoord, coord));
            }
            // One touch point is currently treated as mouse event as well...
            // So prepare for that
            prevMouseCoord_ = coord;
            mousePickObj_ = pickedTouchPoints_it->first;
           // mousePickingOngoing_ = true;
        }

        pickedTouchPoints_it->first->setTouchEvent(TouchEvent(pickedTouchPoints_it->second, e->canvasSize()));
    }

    // One touch point is currently treated as mouse event as well...
    // So prepare for that
    if (touchPoints.size() == 1){
        prevMouseCoord_ = clampToScreenCoords(touchPoints[0].getPos(), e->canvasSize());
        touchPickingOn_ = false;
    }

    // Mark all picking objects in TouchIDPickingMap as picked.
    for (touchPickObjs_it = touchPickObjs_.begin(); touchPickObjs_it != touchPickObjs_.end(); ++touchPickObjs_it)
        touchPickObjs_it->second->picked();

    return !touchPickObjs_.empty();
    */
}

void PickingController::handlePickingEvent(EventPropagator*, GestureEvent*) {
    // TODO...
}

void PickingController::setPickingSource(const std::shared_ptr<const Image>& src) { src_ = src; }

PickingManager::Result PickingController::findPickingAction(const uvec2& coord) {
    if (src_ && pickingEnabled()) {
        if (const auto pickingLayer = src_->getPickingLayer()) {
            if (const auto pickingLayerRAM = pickingLayer->getRepresentation<LayerRAM>()) {
                const auto value = pickingLayerRAM->getAsNormalizedDVec4(coord);
                if (value.a > 0.0) {
                    return PickingManager::getPtr()->getPickingActionFromColor(
                        uvec3(value.rgb() * 255.0));
                }
            }
        }
    }
    return {0, nullptr};
}

bool PickingController::pickingEnabled() const {
    return PickingManager::getPtr()->pickingEnabled();
}

PickingManager::Result PickingController::State::update(PickingController& pc,
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

