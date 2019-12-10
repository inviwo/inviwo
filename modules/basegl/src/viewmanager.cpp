/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <modules/basegl/viewmanager.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/gestureevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <inviwo/core/interaction/events/wheelevent.h>
#include <inviwo/core/interaction/events/pickingevent.h>

#include <inviwo/core/util/exception.h>

namespace inviwo {

ViewManager::ViewManager() = default;

bool ViewManager::propagatePickingEvent(PickingEvent* pe, Propagator propagator) {

    auto prop = [&](Event* newEvent, size_t ind) {
        if (newEvent) {
            auto pressPos = pe->getPressedPosition();
            auto previousPos = pe->getPreviousPosition();

            auto offset = dvec2(views_[ind].pos) / dvec2(pe->getCanvasSize() - uvec2(1));
            auto scale = dvec2(pe->getCanvasSize() - uvec2(1)) / dvec2(views_[ind].size - ivec2(1));

            auto pressNDC = dvec3(2.0 * scale * (pressPos - offset) - 1.0, pe->getPressedDepth());
            auto previousNDC =
                dvec3(2.0 * scale * (previousPos - offset) - 1.0, pe->getPreviousDepth());

            PickingEvent newPe(pe->getPickingAction(), static_cast<InteractionEvent*>(newEvent),
                               pe->getState(), pe->getPressState(), pe->getPressItem(),
                               pe->getHoverState(), pe->getPressItems(), pe->getGlobalPickingId(),
                               pe->getCurrentGlobalPickingId(), pe->getPressedGlobalPickingId(),
                               pe->getPreviousGlobalPickingId(), pressNDC, previousNDC);

            propagator(&newPe, ind);
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

bool ViewManager::propagateMouseEvent(MouseEvent* me, Propagator propagator) {
    selectedView_ = eventState_.getView(*this, me);

    if (selectedView_.first && selectedView_.second < views_.size()) {
        MouseEvent newEvent(*me);
        newEvent.setCanvasSize(uvec2(views_[selectedView_.second].size));
        auto offset = dvec2(views_[selectedView_.second].pos) / dvec2(me->canvasSize() - uvec2(1));
        auto scale = dvec2(me->canvasSize() - uvec2(1)) /
                     dvec2(views_[selectedView_.second].size - ivec2(1));
        newEvent.setPosNormalized(scale * (newEvent.posNormalized() - offset));
        propagator(&newEvent, selectedView_.second);
        if (newEvent.hasBeenUsed()) me->markAsUsed();
        for (auto p : newEvent.getVisitedProcessors()) me->markAsVisited(p);

        return true;
    } else {
        return false;
    }
}

bool ViewManager::propagateWheelEvent(WheelEvent* we, Propagator propagator) {
    selectedView_ = findView(we->pos());

    if (selectedView_.first && selectedView_.second < views_.size()) {
        WheelEvent newEvent(*we);
        newEvent.setCanvasSize(uvec2(views_[selectedView_.second].size));
        auto offset = dvec2(views_[selectedView_.second].pos) / dvec2(we->canvasSize() - uvec2(1));
        auto scale = dvec2(we->canvasSize() - uvec2(1)) /
                     dvec2(views_[selectedView_.second].size - ivec2(1));
        newEvent.setPosNormalized(scale * (newEvent.posNormalized() - offset));
        propagator(&newEvent, selectedView_.second);
        if (newEvent.hasBeenUsed()) we->markAsUsed();
        for (auto p : newEvent.getVisitedProcessors()) we->markAsVisited(p);

        return true;
    } else {
        return false;
    }
}

bool ViewManager::propagateGestureEvent(GestureEvent* ge, Propagator propagator) {
    selectedView_ = eventState_.getView(*this, ge);

    if (selectedView_.first && selectedView_.second < views_.size()) {
        GestureEvent newEvent(*ge);
        newEvent.setCanvasSize(uvec2(views_[selectedView_.second].size));
        auto offset = dvec2(views_[selectedView_.second].pos) / dvec2(ge->canvasSize() - uvec2(1));
        auto scale = dvec2(ge->canvasSize() - uvec2(1)) /
                     dvec2(views_[selectedView_.second].size - ivec2(1));
        newEvent.setScreenPosNormalized(scale * (newEvent.screenPosNormalized() - offset));
        propagator(&newEvent, selectedView_.second);
        if (newEvent.hasBeenUsed()) ge->markAsUsed();
        for (auto p : newEvent.getVisitedProcessors()) ge->markAsVisited(p);

        return true;
    } else {
        return false;
    }
}

bool ViewManager::propagateTouchEvent(TouchEvent* te, Propagator propagator) {
    auto& touchPoints = te->touchPoints();

    std::unordered_map<size_t, std::vector<TouchPoint>> viewIdToTouchPoints;
    std::vector<int> usedPointIds;

    auto idToView = eventState_.getView(*this, te);
    for (auto& point : touchPoints) {
        auto it = idToView.find(point.id());
        if (it != idToView.end()) {
            viewIdToTouchPoints[it->second].push_back(point);
        }
    }

    for (auto& item : viewIdToTouchPoints) {
        const auto& viewId = item.first;
        auto points = item.second;

        const auto canvasSize = uvec2(views_[viewId].size);
        const auto offset = dvec2(views_[viewId].pos) / dvec2(te->canvasSize() - uvec2(1));
        const auto scale =
            dvec2(te->canvasSize() - uvec2(1)) / dvec2(views_[viewId].size - ivec2(1));

        for (auto& p : points) {
            p.setCanvasSize(canvasSize);
            p.setPosNormalized(scale * (p.posNormalized() - offset));
            p.setPrevPosNormalized(scale * (p.prevPosNormalized() - offset));
            p.setPressedPosNormalized(scale * (p.pressedPosNormalized() - offset));
        }

        TouchEvent newEvent(points, te->getDevice(), te->modifiers());

        propagator(&newEvent, viewId);

        for (auto p : newEvent.getVisitedProcessors()) te->markAsVisited(p);
        if (newEvent.hasBeenUsed()) {
            for (const auto& p : points) {
                usedPointIds.push_back(p.id());
            }
        }
    }

    // remove the "used" points from the event
    util::erase_remove_if(touchPoints,
                          [&](const auto& p) { return util::contains(usedPointIds, p.id()); });

    if (touchPoints.empty()) {
        te->markAsUsed();
    }

    return touchPoints.empty();
}

bool ViewManager::propagateEvent(Event* event, Propagator propagator) {
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

std::pair<bool, ViewManager::ViewId> ViewManager::getSelectedView() const { return selectedView_; }

const ViewManager::ViewList& ViewManager::getViews() const { return views_; }

void ViewManager::push_back(View view) { views_.push_back(view); }

void ViewManager::erase(View view) {
    util::erase_remove_if(views_,
                          [&](const auto& v) { return view.pos == v.pos && view.size == v.size; });
}

void ViewManager::erase(ViewId ind) {
    if (ind < views_.size()) {
        views_.erase(views_.begin() + ind);
    }
}

void ViewManager::replace(ViewId ind, View view) {
    if (ind < views_.size()) {
        views_[ind] = view;
    } else {
        throw Exception("Out of range", IVW_CONTEXT);
    }
}

ViewManager::View& ViewManager::operator[](ViewId ind) { return views_[ind]; }

size_t ViewManager::size() const { return views_.size(); }

void ViewManager::clear() { views_.clear(); }

std::pair<bool, ViewManager::ViewId> ViewManager::findView(ivec2 pos) const {
    auto it = util::find_if(views_, [&](const auto& view) { return inView(view, pos); });
    if (it != views_.end()) {
        return {true, std::distance(views_.begin(), it)};
    } else {
        return {false, 0};
    }
}

bool ViewManager::inView(const View& view, const ivec2& pos) {
    return glm::all(glm::greaterThanEqual(pos, view.pos)) &&
           glm::all(glm::lessThan(pos, view.pos + view.size));
}

std::pair<bool, ViewManager::ViewId> ViewManager::EventState::getView(ViewManager& m,
                                                                      const MouseEvent* me) {
    if (!pressing_ && me->buttonState() != MouseButton::None) {  // Start Pressing
        pressing_ = true;
        pressedView_ = m.findView(static_cast<ivec2>(me->pos()));
    } else if (pressing_ && me->buttonState() == MouseButton::None) {  // Stop Pressing
        pressing_ = false;
        pressedView_ = {false, 0};
    }
    return pressing_ ? pressedView_ : m.findView(static_cast<ivec2>(me->pos()));
}

std::pair<bool, ViewManager::ViewId> ViewManager::EventState::getView(ViewManager& m,
                                                                      const GestureEvent* ge) {
    if (!pressing_ && ge->state() == GestureState::Started) {  // Start Pressing
        pressing_ = true;
        pressedView_ =
            m.findView(static_cast<ivec2>(dvec2(ge->canvasSize()) * ge->screenPosNormalized()));
    } else if (pressing_ && ge->state() == GestureState::Finished) {  // Stop Pressing
        pressing_ = false;
        auto tmp = pressedView_;
        pressedView_ = {false, 0};
        return tmp;
    }
    return pressedView_;
}

std::unordered_map<int, ViewManager::ViewId> ViewManager::EventState::getView(
    ViewManager& m, const TouchEvent* te) {

    std::unordered_map<int, ViewId> newTouchpointIdToViewID;
    std::vector<std::pair<bool, ViewManager::ViewId>> foundViews;

    for (const auto& tp : te->touchPoints()) {
        switch (tp.state()) {
            case TouchState::Started: {
                auto res = m.findView(static_cast<ivec2>(tp.pos()));
                if (res.first) {
                    touchpointIdToViewId_[tp.id()] = res.second;
                    newTouchpointIdToViewID[tp.id()] = res.second;
                }
                break;
            }
            case TouchState::Finished: {
                auto it = touchpointIdToViewId_.find(tp.id());
                if (it != touchpointIdToViewId_.end()) {
                    newTouchpointIdToViewID[tp.id()] = it->second;
                }
                touchpointIdToViewId_.erase(tp.id());
                break;
            }
            case TouchState::Stationary:
            case TouchState::Updated:
            case TouchState::None:
            default: {
                auto it = touchpointIdToViewId_.find(tp.id());
                if (it != touchpointIdToViewId_.end()) {
                    newTouchpointIdToViewID[tp.id()] = it->second;
                }
                break;
            }
        }
    }
    return newTouchpointIdToViewID;
}

}  // namespace inviwo
