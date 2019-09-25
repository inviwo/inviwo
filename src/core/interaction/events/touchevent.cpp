/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <inviwo/core/interaction/events/eventutil.h>

namespace inviwo {

TouchPoint::TouchPoint(int id, TouchState touchState, dvec2 posNormalized, dvec2 prevPosNormalized,
                       dvec2 pressedPosNormalized, uvec2 canvasSize, double pressure, double depth)
    : id_(id)
    , state_(touchState)
    , posNormalized_(posNormalized)
    , prevPosNormalized_(prevPosNormalized)
    , pressedPosNormalized_(pressedPosNormalized)
    , pressure_(pressure)
    , canvasSize_(canvasSize)
    , depth_(depth) {}

bool TouchPoint::operator==(const TouchPoint& b) const {
    return id_ == b.id_ && state_ == b.state_ && posNormalized_ == b.posNormalized_ &&
           prevPosNormalized_ == b.prevPosNormalized_ &&
           pressedPosNormalized_ == b.pressedPosNormalized_ && pressure_ == b.pressure_ &&
           canvasSize_ == b.canvasSize_ && depth_ == b.depth_;
}

bool TouchPoint::operator!=(const TouchPoint& b) const { return !(*this == b); }

TouchState TouchPoint::state() const { return state_; }

int TouchPoint::id() const { return id_; }

void TouchPoint::setId(int id) { id_ = id; }

dvec2 TouchPoint::pos() const { return posNormalized_ * dvec2(canvasSize_ - uvec2(1)); }

void TouchPoint::setPos(dvec2 val) { posNormalized_ = val / dvec2(canvasSize_ - uvec2(1)); }

dvec2 TouchPoint::posNormalized() const { return posNormalized_; }

void TouchPoint::setPosNormalized(dvec2 val) { posNormalized_ = val; }

dvec2 TouchPoint::prevPos() const { return prevPosNormalized_ * dvec2(canvasSize_ - uvec2(1)); }

void TouchPoint::setPrevPos(dvec2 val) { prevPosNormalized_ = val / dvec2(canvasSize_ - uvec2(1)); }

dvec2 TouchPoint::prevPosNormalized() const { return prevPosNormalized_; }

void TouchPoint::setPrevPosNormalized(dvec2 val) { prevPosNormalized_ = val; }

dvec2 TouchPoint::pressedPos() const {
    return pressedPosNormalized_ * dvec2(canvasSize_ - uvec2(1));
}

void TouchPoint::setPressedPos(dvec2 val) {
    pressedPosNormalized_ = val / dvec2(canvasSize_ - uvec2(1));
}

dvec2 TouchPoint::pressedPosNormalized() const { return pressedPosNormalized_; }

void TouchPoint::setPressedPosNormalized(dvec2 val) { pressedPosNormalized_ = val; }

double TouchPoint::depth() const { return depth_; }

void TouchPoint::setDepth(double val) { depth_ = val; }

double TouchPoint::pressure() const { return pressure_; }

void TouchPoint::setPressure(double val) { pressure_ = val; }

uvec2 TouchPoint::canvasSize() const { return canvasSize_; }

void TouchPoint::setCanvasSize(uvec2 size) { canvasSize_ = size; }

dvec3 TouchPoint::ndc() const {
    return dvec3(2.0 * posNormalized_.x - 1.0, 2.0 * posNormalized_.y - 1.0, depth_);
}

TouchDevice::TouchDevice(DeviceType type, std::string name) : type_(type), name_(name) {}

TouchEvent::TouchEvent() = default;

TouchEvent::TouchEvent(const std::vector<TouchPoint>& touchPoints, const TouchDevice* source,
                       KeyModifiers modifiers)
    : InteractionEvent(modifiers), touchPoints_(touchPoints), device_(source) {}

TouchEvent* TouchEvent::clone() const { return new TouchEvent(*this); }

bool TouchEvent::hasTouchPoints() const { return !touchPoints_.empty(); }

const std::vector<TouchPoint>& TouchEvent::touchPoints() const { return touchPoints_; }

std::vector<TouchPoint>& TouchEvent::touchPoints() { return touchPoints_; }

void TouchEvent::setTouchPoints(std::vector<TouchPoint> val) { touchPoints_ = val; }

uvec2 TouchEvent::canvasSize() const {
    if (touchPoints_.empty()) {
        return uvec2(0);
    } else {
        return touchPoints_[0].canvasSize();
    }
}

dvec2 TouchEvent::centerPoint() const {
    if (touchPoints_.empty()) {
        return dvec2(0.0);
    } else {
        // Compute average position
        auto sum = std::accumulate(touchPoints_.begin(), touchPoints_.end(), dvec2(0.0),
                                   [](dvec2 s, const TouchPoint& p) { return s + p.pos(); });
        return sum / static_cast<double>(touchPoints_.size());
    }
}

dvec2 TouchEvent::centerPointNormalized() const {
    if (touchPoints_.empty()) {
        return dvec2(0.0);
    } else {
        // Compute average position
        auto sum =
            std::accumulate(touchPoints_.begin(), touchPoints_.end(), dvec2(0.0),
                            [](dvec2 s, const TouchPoint& p) { return s + p.posNormalized(); });
        return sum / static_cast<double>(touchPoints_.size());
    }
}

dvec2 TouchEvent::prevCenterPointNormalized() const {
    if (touchPoints_.empty()) {
        return dvec2(0.0);
    } else {
        // Compute average position
        auto sum =
            std::accumulate(touchPoints_.begin(), touchPoints_.end(), dvec2(0.0),
                            [](dvec2 s, const TouchPoint& p) { return s + p.prevPosNormalized(); });
        return sum / static_cast<double>(touchPoints_.size());
    }
}

dvec3 TouchEvent::centerNDC() const {
    if (touchPoints_.empty()) {
        return dvec3(0.0);
    } else {
        // Compute average position
        auto sum = std::accumulate(touchPoints_.begin(), touchPoints_.end(), dvec3(0.0),
                                   [](dvec3 s, const TouchPoint& p) { return s + p.ndc(); });
        return sum / static_cast<double>(touchPoints_.size());
    }
}

double TouchEvent::averageDepth() const {
    if (touchPoints_.empty()) {
        return 1.0;
    } else {
        // Compute average position
        auto sum = std::accumulate(touchPoints_.begin(), touchPoints_.end(), 0.0,
                                   [](double s, const TouchPoint& p) { return s + p.depth(); });
        return sum / static_cast<double>(touchPoints_.size());
    }
}

PickingState TouchEvent::getPickingState(const std::vector<TouchPoint>& points) {
    auto toPickingState = [](TouchState ts) {
        switch (ts) {
            case TouchState::Started:
                return PickingState::Started;
            case TouchState::Finished:
                return PickingState::Finished;
            case TouchState::Updated:
            case TouchState::None:
            case TouchState::Stationary:
            default:
                return PickingState::Updated;
        }
    };

    return std::accumulate(points.begin(), points.end(), toPickingState(points.front().state()),
                           [&](PickingState ps, const TouchPoint& tp) {
                               auto s = toPickingState(tp.state());
                               if (ps == s)
                                   return ps;
                               else
                                   return PickingState::Updated;
                           });
}

std::vector<const TouchPoint*> TouchEvent::findClosestTwoTouchPoints() const {
    std::vector<const TouchPoint*> returnVec;
    if (touchPoints_.size() > 1) {
        const TouchPoint* touchPoint1 = &touchPoints_[0];
        const TouchPoint* touchPoint2 = &touchPoints_[1];
        double distance = std::numeric_limits<float>::max();
        for (size_t i = 0; i < touchPoints_.size() - 1; ++i) {
            for (size_t j = i + 1; j < touchPoints_.size(); ++j) {
                double ijDistance = glm::distance2(touchPoints_[i].pos(), touchPoints_[j].pos());
                if (ijDistance < distance) {
                    distance = ijDistance;
                    touchPoint1 = &touchPoints_[i];
                    touchPoint2 = &touchPoints_[j];
                }
            }
        }
        returnVec.push_back(touchPoint1);
        returnVec.push_back(touchPoint2);
    } else if (!touchPoints_.empty()) {
        returnVec.push_back(&touchPoints_[0]);
    }
    return returnVec;
}

uint64_t TouchEvent::hash() const { return chash(); }

void TouchEvent::print(std::ostream& ss) const {
    util::printEvent(ss, "TouchEvent", std::make_pair("points", touchPoints_.size()),
                     std::make_pair("size", canvasSize()), std::make_pair("modifiers", modifiers_));

    for (const auto& p : touchPoints()) {
        util::printEvent(ss, "\n  TouchPoint", std::make_tuple("id", p.id(), 2),
                         std::make_pair("state", p.state()), std::make_pair("depth", p.depth()),
                         std::make_pair("pos", p.pos()), std::make_pair("prev", p.prevPos()));
    }
}

}  // namespace inviwo
