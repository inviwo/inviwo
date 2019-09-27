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

    const auto pickId = [&](size2_t coord) -> size_t {
        if (src_) {
            const auto value = src_->readPixel(coord, LayerType::Picking);
            if (value.a > 0.0) {
                return PickingManager::colorToIndex(uvec3(value));
            }
        }
        return 0;
    };

    switch (event->hash()) {
        case MouseEvent::chash(): {
            auto me = static_cast<MouseEvent*>(event);
            const auto coord =
                glm::clamp(me->pos(), dvec2(0.0), dvec2(me->canvasSize() - uvec2(1)));
            mouseState_.propagateEvent(me, propagator, pickId(coord));
            break;
        }
        case WheelEvent::chash(): {
            auto me = static_cast<WheelEvent*>(event);
            const auto coord =
                glm::clamp(me->pos(), dvec2(0.0), dvec2(me->canvasSize() - uvec2(1)));
            mouseState_.propagateEvent(me, propagator, pickId(coord));
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
    auto& touchPoints = e->touchPoints();

    std::unordered_map<size_t, std::vector<TouchPoint>> pickngIdToTouchPoints;
    std::unordered_map<size_t, const PickingAction*> pickingIdToAction;
    std::vector<int> usedPointIds;

    for (auto& point : touchPoints) {
        const auto coord = glm::clamp(point.pos(), dvec2(0.0), dvec2(e->canvasSize() - uvec2(1)));
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

        TouchEvent te(points, e->getDevice(), e->modifiers());
        auto prevPos = te.centerNDC();  // Need so save here since te might be modified

        size_t currentId = 0;   // TODO
        size_t pressedId = 0;   // TODO
        size_t previousId = 0;  // TODO
        PickingPressState pressState = [ps]() {
            switch (ps) {
                case PickingState::Started:
                    return PickingPressState::Press;
                case PickingState::Updated:
                    return PickingPressState::Move;
                case PickingState::Finished:
                    return PickingPressState::Release;
                default:
                    return PickingPressState::None;
            }
        }();
        PickingPressItem pressItem = [points, e]() {
            if (e->getDevice()->getType() == TouchDevice::DeviceType::TouchPad) {
                // Possible to press touchpads
                auto nPressed = std::accumulate(
                    points.begin(), points.end(), 0u, [](size_t v, const auto& p) -> size_t {
                        return v + (p.state() == TouchState::Stationary ? 1 : 0);
                    });
                if (nPressed == 0) {
                    return PickingPressItem::None;
                } else if (nPressed == 1) {
                    return PickingPressItem::Primary;
                } else if (nPressed == 2) {
                    return PickingPressItem::Secondary;
                } else {  // nPressed > 2
                    return PickingPressItem::Tertiary;
                }
            } else {
                // Treat touch point on TouchScreen as "button down"
                return PickingPressItem::Primary;
            }
        }();

        PickingEvent pickingEvent(pickingIdToAction[pickingId], &te, ps, pressState, pressItem,
                                  PickingHoverState::None, pressItem, pickingId, currentId,
                                  pressedId, previousId, tstate_.pickingIdToPressNDC[pickingId],
                                  tstate_.pickingIdToPreviousNDC[pickingId]);

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
    util::erase_remove_if(touchPoints,
                          [&](const auto& p) { return util::contains(usedPointIds, p.id()); });
    if (touchPoints.empty()) e->markAsUsed();
}

void PickingController::propagateEvent(GestureEvent*, EventPropagator*) {
    // TODO
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

}  // namespace inviwo
