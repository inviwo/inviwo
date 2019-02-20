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

#include <inviwo/core/interaction/events/wheelevent.h>
#include <inviwo/core/interaction/events/eventutil.h>

namespace inviwo {

WheelEvent::WheelEvent(MouseButtons buttonState, KeyModifiers modifiers, dvec2 delta,
                       dvec2 normalizedPosition, uvec2 canvasSize, double depth)
    : MouseInteractionEvent(buttonState, modifiers, normalizedPosition, canvasSize, depth)
    , delta_(delta) {}

WheelEvent* WheelEvent::clone() const { return new WheelEvent(*this); }

dvec2 WheelEvent::delta() const { return delta_; }

void WheelEvent::setDelta(dvec2 delta) { delta_ = delta; }

uint64_t WheelEvent::hash() const { return chash(); }

void WheelEvent::print(std::ostream& ss) const {
    util::printEvent(ss, "WheelEvent", std::make_pair("delta", delta_),
                     std::make_pair("pos", pos()), std::make_pair("depth", depth()),
                     std::make_pair("size", canvasSize()), std::make_pair("sState", buttonState()),
                     std::make_pair("modifiers", modifiers_));
}

}  // namespace inviwo
