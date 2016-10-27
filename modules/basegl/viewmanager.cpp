/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2016 Inviwo Foundation
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

#include "viewmanager.h"
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/gestureevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <inviwo/core/interaction/events/wheelevent.h>
#include <inviwo/core/interaction/events/pickingevent.h>

#include <inviwo/core/util/exception.h>

namespace inviwo {

ViewManager::ViewManager() {}

std::unique_ptr<Event> ViewManager::handlePickingEvent(const PickingEvent* pe) {
    Event* e = pe->getEvent();

    std::unique_ptr<Event> newEvent;
    switch (e->hash()) {
        case MouseEvent::chash(): {
            newEvent = handleMouseEvent(static_cast<const MouseEvent*>(e));
            break;
        }
        case WheelEvent::chash(): {
            newEvent = handleWheelEvent(static_cast<const WheelEvent*>(e));
            break;
        }
        case GestureEvent::chash(): {
            newEvent = handleGestureEvent(static_cast<const GestureEvent*>(e));
            break;
        }
        case TouchEvent::chash(): {
            newEvent = handleTouchEvent(static_cast<const TouchEvent*>(e));
            break;
        }
        default:
            newEvent = nullptr;
            break;
    }

    if (newEvent, selectedView_.first) {
        auto pressPos = pe->getPressPosition();
        auto previousPos = pe->getPreviousPosition();

        auto offset = dvec2(views_[selectedView_.second].pos) / dvec2(pe->getCanvasSize() - uvec2(1));

        auto scale = dvec2(pe->getCanvasSize() - uvec2(1)) /
                     dvec2(views_[selectedView_.second].size - ivec2(1));

        auto pressNDC = dvec3(2.0 * scale * (pressPos - offset) - 1.0, pe->getPressDepth());
        auto previousNDC =
            dvec3(2.0 * scale * (previousPos - offset) - 1.0, pe->getPreviousDepth());

        auto newPe = new PickingEvent(pe->getPickingAction(), pe->getState(), std::move(newEvent),
                                      pressNDC, previousNDC, pe->getPickedId());
        return std::unique_ptr<Event>(newPe);
    }

    return nullptr;
}

std::unique_ptr<Event> ViewManager::handleMouseEvent(const MouseEvent* me) {
    selectedView_ = eventState_.getView(*this, me);

    if (selectedView_.first && selectedView_.second < views_.size()) {
        auto newEvent = me->clone();
        newEvent->setCanvasSize(uvec2(views_[selectedView_.second].size));
        auto offset = dvec2(views_[selectedView_.second].pos) / dvec2(me->canvasSize() - uvec2(1));
        auto scale = dvec2(me->canvasSize() - uvec2(1)) /
                     dvec2(views_[selectedView_.second].size - ivec2(1));
        newEvent->setPosNormalized(scale * (newEvent->posNormalized() - offset));
        return std::unique_ptr<Event>(newEvent);
    } else {
        return nullptr;
    }
}

std::unique_ptr<Event> ViewManager::handleWheelEvent(const WheelEvent* we) {
    selectedView_ = findView(we->pos());

    if (selectedView_.first && selectedView_.second < views_.size()) {
        auto newEvent = we->clone();
        newEvent->setCanvasSize(uvec2(views_[selectedView_.second].size));
        auto offset = dvec2(views_[selectedView_.second].pos) / dvec2(we->canvasSize() - uvec2(1));
        auto scale = dvec2(we->canvasSize() - uvec2(1)) /
                     dvec2(views_[selectedView_.second].size - ivec2(1));
        newEvent->setPosNormalized(scale * (newEvent->posNormalized() - offset));
        return std::unique_ptr<Event>(newEvent);
    } else {
        return nullptr;
    }
}

std::unique_ptr<Event> ViewManager::handleGestureEvent(const GestureEvent* ge) {
    selectedView_ = eventState_.getView(*this, ge);

    if (selectedView_.first && selectedView_.second < views_.size()) {
        auto newEvent = ge->clone();
        newEvent->setCanvasSize(uvec2(views_[selectedView_.second].size));
        auto offset = dvec2(views_[selectedView_.second].pos) / dvec2(ge->canvasSize() - uvec2(1));
        auto scale = dvec2(ge->canvasSize() - uvec2(1)) /
                     dvec2(views_[selectedView_.second].size - ivec2(1));
        newEvent->setScreenPosNormalized(scale * (newEvent->screenPosNormalized() - offset));
        return std::unique_ptr<Event>(newEvent);
    } else {
        return nullptr;
    }
}

std::unique_ptr<Event> ViewManager::handleTouchEvent(const TouchEvent* te) {
    /*
    const auto touchEvent = static_cast<const TouchEvent*>(event);
    activePosition_ = touchEvent->getCenterPoint();
    if (!viewportActive_ &&
        touchEvent->getTouchPoints().front().state() == TouchState::Started) {
        viewportActive_ = true;
        activeView_ = findView(activePosition_);
    } else if (viewportActive_ &&
               touchEvent->getTouchPoints().front().state() == TouchState::Finished) {
        viewportActive_ = false;
    }

    if (activeView_ >= 0 && activeView_ < static_cast<long>(views_.size())) {
        // Modify all touch points
        const ivec4& view = views_[activeView_];
        vec2 viewportOffset(view.x, view.y);
        vec2 viewportSize(view.z, view.w);
        std::vector<TouchPoint> modifiedTouchPoints;
        auto touchPoints = touchEvent->getTouchPoints();
        modifiedTouchPoints.reserve(touchPoints.size());
        // Loop over all touch points and modify their positions
        for (auto elem : touchPoints) {
            // Translate position to viewport
            vec2 pos = elem.getPos() - viewportOffset;
            vec2 posNormalized = pos / viewportSize;
            vec2 prevPos = elem.getPrevPos() - viewportOffset;
            vec2 prevPosNormalized = prevPos / viewportSize;
            modifiedTouchPoints.push_back(TouchPoint(elem.getId(), pos, posNormalized,
                                                     prevPos, prevPosNormalized,
                                                     elem.state()));
        }
        TouchEvent* newEvent = new TouchEvent(modifiedTouchPoints, viewportSize);

        return newEvent;
    } else {
    */
    return nullptr;
    //}
}

std::unique_ptr<Event> ViewManager::registerEvent(const Event* event) {
    switch (event->hash()) {
        case PickingEvent::chash(): {
            return handlePickingEvent(static_cast<const PickingEvent*>(event));
        }
        case MouseEvent::chash(): {
            return handleMouseEvent(static_cast<const MouseEvent*>(event));
        }
        case WheelEvent::chash(): {
            return handleWheelEvent(static_cast<const WheelEvent*>(event));
        }
        case GestureEvent::chash(): {
            return handleGestureEvent(static_cast<const GestureEvent*>(event));
        }
        case TouchEvent::chash(): {
            return handleTouchEvent(static_cast<const TouchEvent*>(event));
        }
        default:
            return nullptr;
    }
}

std::pair<bool, ViewManager::ViewId> ViewManager::getSelectedView() const {
    return selectedView_;
}

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
    }
    else {
        throw Exception("Out of range", IvwContext);
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
    return glm::all(glm::lessThan(view.pos, pos)) &&
           glm::all(glm::lessThan(pos, view.pos + view.size));
}

}  // namespace
