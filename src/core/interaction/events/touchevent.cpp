/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2016 Inviwo Foundation
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

#include <inviwo/core/interaction/events/touchevent.h>

namespace inviwo {

TouchPoint::TouchPoint(int id, vec2 pos, vec2 posNormalized, vec2 prevPos, vec2 prevPosNormalized,
                       TouchState touchState, double depth)
    : id_(id)
    , pos_(pos)
    , posNormalized_(posNormalized)
    , prevPos_(prevPos)
    , prevPosNormalized_(prevPosNormalized)
    , state_(touchState)
    , depth_(depth) {}

inviwo::TouchState TouchPoint::state() const {
    return state_;
}

int TouchPoint::getId() const {
    return id_;
}

void TouchPoint::setId(int id) {
    id_ = id;
}

inviwo::vec2 TouchPoint::getPos() const {
    return pos_;
}

void TouchPoint::setPos(vec2 val) {
    pos_ = val;
}

inviwo::vec2 TouchPoint::getPosNormalized() const {
    return posNormalized_;
}

void TouchPoint::setPosNormalized(vec2 val) {
    posNormalized_ = val;
}

inviwo::vec2 TouchPoint::getPrevPos() const {
    return prevPos_;
}

void TouchPoint::setPrevPos(vec2 val) {
    prevPos_ = val;
}

inviwo::vec2 TouchPoint::getPrevPosNormalized() const {
    return prevPosNormalized_;
}

void TouchPoint::setPrevPosNormalized(vec2 val) {
    prevPosNormalized_ = val;
}

double TouchPoint::getDepth() const {
    return depth_;
}

void TouchPoint::setDepth(double val) {
    depth_ = val;
}

TouchEvent::TouchEvent(uvec2 canvasSize) : InteractionEvent(), canvasSize_(canvasSize) {}

TouchEvent::TouchEvent(std::vector<TouchPoint> touchPoints, uvec2 canvasSize)
    : InteractionEvent(), touchPoints_(touchPoints), canvasSize_(canvasSize) {}

TouchEvent* TouchEvent::clone() const { return new TouchEvent(*this); }

bool TouchEvent::hasTouchPoints() const { return !touchPoints_.empty(); }

const std::vector<TouchPoint>& TouchEvent::getTouchPoints() const { return touchPoints_; }

std::vector<TouchPoint>& TouchEvent::getTouchPoints() { return touchPoints_; }

void TouchEvent::setTouchPoints(std::vector<TouchPoint> val) { touchPoints_ = val; }

inviwo::uvec2 TouchEvent::canvasSize() const {
    return canvasSize_;
}

vec2 TouchEvent::getCenterPoint() const {
    if (touchPoints_.empty()) {
        return vec2(0);
    } else {
        // Compute average position
        vec2 sum(0);
        std::for_each(touchPoints_.begin(), touchPoints_.end(),
                      [&](const TouchPoint& p) { sum += p.getPos(); });
        return sum / static_cast<float>(touchPoints_.size());
    }
}

inviwo::vec2 TouchEvent::getCenterPointNormalized() const {
    if (touchPoints_.empty()) {
        return vec2(0);
    } else {
        // Compute average position
        vec2 sum(0);
        std::for_each(touchPoints_.begin(), touchPoints_.end(),
                      [&](const TouchPoint& p) { sum += p.getPosNormalized(); });
        return sum / static_cast<float>(touchPoints_.size());
    }
}

vec2 TouchEvent::getPrevCenterPointNormalized() const {
    if (touchPoints_.empty()) {
        return vec2(0);
    } else {
        // Compute average position
        vec2 sum(0);
        std::for_each(touchPoints_.begin(), touchPoints_.end(),
                      [&](const TouchPoint& p) { sum += p.getPrevPosNormalized(); });
        return sum / static_cast<float>(touchPoints_.size());
    }
}

std::vector<const TouchPoint*> TouchEvent::findClosestTwoTouchPoints() const {
    std::vector<const TouchPoint*> returnVec;
    if (touchPoints_.size() > 1) {
        const TouchPoint* touchPoint1 = &touchPoints_[0];
        const TouchPoint* touchPoint2 = &touchPoints_[1];
        float distance = std::numeric_limits<float>::max();
        for (size_t i = 0; i < touchPoints_.size() - 1; ++i) {
            for (size_t j = i + 1; j < touchPoints_.size(); ++j) {
                float ijDistance =
                    glm::distance2(touchPoints_[i].getPos(), touchPoints_[j].getPos());
                if (ijDistance < distance) {
                    distance = ijDistance;
                    touchPoint1 = &touchPoints_[i];
                    touchPoint2 = &touchPoints_[j];
                }
            }
        }
        returnVec.push_back(touchPoint1);
        returnVec.push_back(touchPoint2);
    } else if (!touchPoints_.empty())
        returnVec.push_back(&touchPoints_[0]);

    return returnVec;
}

uint64_t TouchEvent::hash() const {
    return chash();
}


}  // namespace