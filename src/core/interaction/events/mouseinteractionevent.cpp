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

#include <inviwo/core/interaction/events/mouseinteractionevent.h>

namespace inviwo {

MouseInteractionEvent::MouseInteractionEvent(MouseButtons buttonState, KeyModifiers modifiers,
                                             dvec2 normalizedPosition, uvec2 canvasSize,
                                             double depth)
    : InteractionEvent(modifiers)
    , buttonState_(buttonState)
    , position_(normalizedPosition)
    , canvasSize_(canvasSize)
    , depth_(depth) {}

MouseButtons MouseInteractionEvent::buttonState() const { return buttonState_; }

void MouseInteractionEvent::setButtonState(MouseButtons buttonState) { buttonState_ = buttonState; }

dvec2 MouseInteractionEvent::pos() const { return position_ * dvec2(canvasSize_ - uvec2(1)); }

void MouseInteractionEvent::setPos(dvec2 pos) { position_ = pos / dvec2(canvasSize_ - uvec2(1)); }

uvec2 MouseInteractionEvent::canvasSize() const { return canvasSize_; }

void MouseInteractionEvent::setCanvasSize(uvec2 size) { canvasSize_ = size; }

dvec2 MouseInteractionEvent::posNormalized() const { return position_; }

void MouseInteractionEvent::setPosNormalized(dvec2 pos) { position_ = pos; }

dvec3 MouseInteractionEvent::ndc() const {
    return dvec3(2.0 * position_.x - 1.0, 2.0 * position_.y - 1.0, depth_);
}

double MouseInteractionEvent::x() const { return position_.x * (canvasSize_.x - 1); }

double MouseInteractionEvent::y() const { return position_.y * (canvasSize_.y - 1); }

double MouseInteractionEvent::depth() const { return depth_; }

void MouseInteractionEvent::setDepth(double depth) { depth_ = depth; }

std::string MouseInteractionEvent::buttonName() const {
    std::stringstream ss;
    ss << buttonState_;
    return ss.str();
}

}  // namespace inviwo
