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

#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/eventutil.h>

namespace inviwo {

MouseEvent::MouseEvent(MouseButton button, MouseState state, MouseButtons buttonState,
                       KeyModifiers modifiers, dvec2 normalizedPosition, uvec2 canvasSize,
                       double depth)
    : MouseInteractionEvent(buttonState, modifiers, normalizedPosition, canvasSize, depth)
    , button_(button)
    , state_(state) {}

MouseEvent* MouseEvent::clone() const { return new MouseEvent(*this); }

MouseButton MouseEvent::button() const { return button_; }

void MouseEvent::setButton(MouseButton button) { button_ = button; }

MouseState MouseEvent::state() const { return state_; }

void MouseEvent::setState(MouseState state) { state_ = state; }

uint64_t MouseEvent::hash() const { return chash(); }

void MouseEvent::print(std::ostream& ss) const {
    util::printEvent(ss, "MouseEvent", std::make_pair("state", state_),
                     std::make_pair("button", button_), std::make_pair("pos", pos()),
                     std::make_pair("depth", depth()), std::make_pair("size", canvasSize()),
                     std::make_pair("sState", buttonState()),
                     std::make_pair("modifiers", modifiers_));
}

}  // namespace inviwo
