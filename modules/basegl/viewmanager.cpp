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

#include <inviwo/core/util/exception.h>

namespace inviwo {

ViewManager::ViewManager() : viewportActive_(false), activePosition_(ivec2(0)), activeView_(-1) {}

Event* ViewManager::registerEvent(const Event* event) {
    switch (event->hash()) {
        case MouseEvent::chash(): {
            const auto mouseEvent = static_cast<const MouseEvent*>(event);

            activePosition_ = mouseEvent->pos();
            if (!viewportActive_ && mouseEvent->state() == MouseState::Press) {
                viewportActive_ = true;
                activeView_ = findView(activePosition_);
            } else if (viewportActive_ && mouseEvent->state() == MouseState::Release) {
                viewportActive_ = false;
            }

            if (activeView_ >= 0 && activeView_ < static_cast<long>(views_.size())) {
                auto newEvent = mouseEvent->clone();
                const ivec4& view = views_[activeView_];
                newEvent->setCanvasSize(uvec2(view.z, view.w));
                newEvent->setPos(activePosition_ - dvec2(view.x, view.y));
                return newEvent;
            } else {
                return nullptr;
            }
        }
        case WheelEvent::chash(): {
            const auto wheelEvent = static_cast<const WheelEvent*>(event);

            activePosition_ = wheelEvent->pos();
            activeView_ = findView(activePosition_);

            if (activeView_ >= 0 && activeView_ < static_cast<long>(views_.size())) {
                auto newEvent = wheelEvent->clone();
                const ivec4& view = views_[activeView_];
                newEvent->setCanvasSize(uvec2(view.z, view.w));
                newEvent->setPos(activePosition_ - dvec2(view.x, view.y));
                return newEvent;
            } else {
                return nullptr;
            }
        }
        case GestureEvent::chash(): {
            const auto gestureEvent = static_cast<const GestureEvent*>(event);
            activePosition_ = gestureEvent->canvasSize() * gestureEvent->screenPosNormalized();
            if (!viewportActive_ && gestureEvent->state() == GestureState::Started) {
                viewportActive_ = true;
                activeView_ = findView(activePosition_);
            } else if (viewportActive_ && gestureEvent->state() == GestureState::Finished) {
                viewportActive_ = false;
            }

            if (activeView_ >= 0 && activeView_ < static_cast<long>(views_.size())) {
                GestureEvent* newEvent = gestureEvent->clone();
                const ivec4& view = views_[activeView_];
                newEvent->modify(vec2(activePosition_ - dvec2(view.x, view.y)) /
                                 vec2(view.z, view.w));
                return newEvent;
            } else {
                return nullptr;
            }
        }
        case TouchEvent::chash(): {
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
                return nullptr;
            }
        }
        default:
            return nullptr;
    }
}

const std::vector<ivec4>& ViewManager::getViews() const { return views_; }

void ViewManager::push_back(ivec4 view) { views_.push_back(view); }

void ViewManager::erase(ivec4 view) {
    auto it = views_.begin();
    while (it != views_.end()) {
        if (glm::all(glm::equal(view, *it))) {
            views_.erase(it);
            break;
        }
        ++it;
    }
}

void ViewManager::erase(size_t ind) {
    if (ind < views_.size()) {
        views_.erase(views_.begin() + ind);
    }
}

void ViewManager::replace(size_t ind, ivec4 view) {
    if (ind < views_.size()) {
        views_[ind] = view;
    }
    else {
        throw Exception("Out of range", IvwContext);
    }
}

ivec4& ViewManager::operator[](size_t ind) { return views_[ind]; }

size_t ViewManager::size() const { return views_.size(); }

void ViewManager::clear() { views_.clear(); }

int ViewManager::findView(ivec2 pos) const {
    for (int i = 0; i < static_cast<int>(views_.size()); ++i) {
        if (inView(views_[i], pos)) {
            return i;
        }
    }
    return -1;
}

bool ViewManager::inView(const ivec4& view, const ivec2& pos) {
    return view.x < pos.x && pos.x < view.x + view.z && view.y < pos.y && pos.y < view.y + view.w;
}

}  // namespace
