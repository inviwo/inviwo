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

#include <inviwo/core/interaction/events/gestureevent.h>
#include <inviwo/core/interaction/events/eventutil.h>

namespace inviwo {

GestureEvent::GestureEvent(dvec2 deltaPos, double deltaDistance, GestureType type,
                           GestureState state, int numFingers, dvec2 screenPosNorm,
                           uvec2 canvasSize, double depth)
    : InteractionEvent()
    , type_(type)
    , state_(state)
    , numFingers_(numFingers)
    , deltaPos_(deltaPos)
    , deltaDistance_(deltaDistance)
    , screenPosNorm_(screenPosNorm)
    , canvasSize_(canvasSize)
    , depth_(depth) {}

GestureEvent* GestureEvent::clone() const { return new GestureEvent(*this); }

dvec2 GestureEvent::deltaPos() const { return deltaPos_; }

double GestureEvent::deltaDistance() const { return deltaDistance_; }

inviwo::GestureType GestureEvent::type() const { return type_; }

inviwo::GestureState GestureEvent::state() const { return state_; }

int GestureEvent::numFingers() const { return numFingers_; }

dvec2 GestureEvent::screenPosNormalized() const { return screenPosNorm_; }

uvec2 GestureEvent::canvasSize() const { return canvasSize_; }

void GestureEvent::setCanvasSize(uvec2 size) { canvasSize_ = size; }

double GestureEvent::depth() const { return depth_; }

void GestureEvent::setDepth(double depth) { depth_ = depth; }

dvec3 GestureEvent::ndc() const {
    return dvec3(2.0 * screenPosNorm_.x - 1.0, 2.0 * screenPosNorm_.y - 1.0, depth_);
}

void GestureEvent::setScreenPosNormalized(dvec2 posNorm) { screenPosNorm_ = posNorm; }

uint64_t GestureEvent::hash() const { return chash(); }

void GestureEvent::print(std::ostream& ss) const {
    util::printEvent(ss, "GestureEvent", std::make_pair("state", state_),
                     std::make_pair("type", type_), std::make_pair("pos", screenPosNorm_),
                     std::make_pair("depth", depth_), std::make_pair("canvasSize", canvasSize_),
                     std::make_pair("deltaPos", deltaPos_),
                     std::make_pair("modifiers", modifiers_));
}

}  // namespace inviwo
