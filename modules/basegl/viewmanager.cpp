/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

namespace inviwo {

ViewManager::ViewManager() : viewportActive_(false), activePosition_(ivec2(0)), activeView_(-1) {}

Event* ViewManager::registerEvent(Event* event) {
    if (MouseEvent* mouseEvent = dynamic_cast<MouseEvent*>(event)) {
        activePosition_ = flipY(mouseEvent->pos(), mouseEvent->canvasSize());
        if (!viewportActive_ && mouseEvent->state() == MouseEvent::MOUSE_STATE_PRESS) {
            viewportActive_ = true;
            activeView_ = findView(activePosition_);
        } else if (viewportActive_ && mouseEvent->state() == MouseEvent::MOUSE_STATE_RELEASE) {
            viewportActive_ = false;
        }

        if (activeView_ >= 0 && activeView_ < views_.size()) {
            MouseEvent* newEvent = mouseEvent->clone();
            const ivec4& view = views_[activeView_];
            newEvent->modify(flipY(activePosition_ - ivec2(view.x, view.y), ivec2(view.z, view.w)),
                             uvec2(view.z, view.w));
            return newEvent;
        } else {
            return nullptr;
        }

    } else if (GestureEvent* gestureEvent = dynamic_cast<GestureEvent*>(event)) {
        activePosition_ = flipY(gestureEvent->canvasSize() * gestureEvent->screenPosNormalized(),
                                gestureEvent->canvasSize());
        if (!viewportActive_ && gestureEvent->state() == GestureEvent::GESTURE_STATE_STARTED) {
            viewportActive_ = true;
            activeView_ = findView(activePosition_);
        } else if (viewportActive_ && gestureEvent->state() == GestureEvent::GESTURE_STATE_ENDED) {
            viewportActive_ = false;
        }

        if (activeView_ >= 0 && activeView_ < views_.size()) {
            GestureEvent* newEvent = gestureEvent->clone();
            const ivec4& view = views_[activeView_];
            newEvent->modify(
                vec2(flipY(activePosition_ - ivec2(view.x, view.y), ivec2(view.z, view.w))) /
                vec2(view.z, view.w));
            return newEvent;
        } else {
            return nullptr;
        }

    } else if (TouchEvent* touchEvent = dynamic_cast<TouchEvent*>(event)) {
        // TODO fix TouchEvents
        activePosition_ = flipY(touchEvent->getCenterPoint(), touchEvent->canvasSize());
        if (!viewportActive_ && touchEvent->getTouchPoints().front().state() == TouchPoint::TOUCH_STATE_STARTED) {
            viewportActive_ = true;
            activeView_ = findView(activePosition_);
        } else if (viewportActive_ && touchEvent->getTouchPoints().front().state() ==
            TouchPoint::TOUCH_STATE_ENDED) {
            viewportActive_ = false;
        }

        if (activeView_ >= 0 && activeView_ < views_.size()) {
            // Modify all touch points
            const ivec4& view = views_[activeView_];
            vec2 viewportOffset(view.x, view.y);
            vec2 viewportSize(view.z, view.w);
            std::vector<TouchPoint> modifiedTouchPoints;
            auto touchPoints = touchEvent->getTouchPoints();
            modifiedTouchPoints.reserve(touchPoints.size());
            // Loop over all touch points and modify their positions
            for (auto elem : touchPoints) {
                vec2 pos = flipY(elem.getPos() - viewportOffset, viewportSize);
                vec2 posNormalized = pos / viewportSize;
                vec2 prevPos = flipY(elem.getPrevPos() - viewportOffset, viewportSize);
                vec2 prevPosNormalized = prevPos / viewportSize;
                modifiedTouchPoints.push_back(TouchPoint(pos, posNormalized, prevPos, prevPosNormalized, elem.state()));
            }
            TouchEvent* newEvent = new TouchEvent(modifiedTouchPoints, viewportSize);

            return newEvent;
        } else {
            return nullptr;
        }
    }

    return nullptr;
}

const std::vector<ivec4>& ViewManager::getViews() const { return views_; }

void ViewManager::push_back(ivec4 view) { views_.push_back(view); }

ivec4& ViewManager::operator[](size_t ind) { return views_[ind]; }

size_t ViewManager::size() const { return views_.size(); }

void ViewManager::clear() { views_.clear(); }

size_t ViewManager::findView(ivec2 pos) const {
    for (size_t i = 0; i < views_.size(); ++i) {
        if (inView(views_[i], pos)) {
            return i;
        }
    }
    return -1;
}

inviwo::ivec2 ViewManager::flipY(ivec2 pos, ivec2 size) { return ivec2(pos.x, size.y - pos.y); }

bool ViewManager::inView(const ivec4& view, const ivec2& pos) {
    return view.x < pos.x && pos.x < view.x + view.z && view.y < pos.y && pos.y < view.y + view.w;
}

}  // namespace
