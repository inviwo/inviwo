/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/core/interaction/events/axisrangeevent.h>
#include <inviwo/core/interaction/events/eventutil.h>

namespace inviwo {

AxisRangeEvent::AxisRangeEvent(AxisRangeEventState state, AxisRangeInteraction interaction,
                               AxisRangeInteractionMode mode, std::optional<Rectangle> rect)
    : state_{state}, interaction_{interaction}, mode_{mode}, rect_{std::move(rect)} {}

AxisRangeEvent* AxisRangeEvent::clone() const { return new AxisRangeEvent{*this}; }

auto AxisRangeEvent::rect() const -> std::optional<Rectangle> { return rect_; }

AxisRangeEventState AxisRangeEvent::state() const { return state_; }

AxisRangeInteraction AxisRangeEvent::interaction() const { return interaction_; }

AxisRangeInteractionMode AxisRangeEvent::mode() const { return mode_; }

uint64_t AxisRangeEvent::hash() const { return chash(); }

void AxisRangeEvent::print(fmt::memory_buffer& buff) const {
    if (rect_.has_value()) {
        util::printEvent(buff, "AxisRangeEvent", std::make_pair("state", state_),
                         std::make_pair("interaction", interaction_), std::make_pair("mode", mode_),
                         std::make_pair("start", (*rect_)[0]), std::make_pair("end", (*rect_)[1]));
    } else {
        util::printEvent(buff, "AxisRangeEvent", std::make_pair("state", state_),
                         std::make_pair("interaction", interaction_),
                         std::make_pair("mode", mode_));
    }
}

}  // namespace inviwo
