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
#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/interaction/axisrangeeventstate.h>
#include <inviwo/core/util/constexprhash.h>
#include <inviwo/core/util/glmvec.h>

#include <array>
#include <optional>

namespace inviwo {

/**
 * Event propagating axis range events from upwards in the network, which can for example be
 * used for selections.
 *
 * @see OrthoGraphicAxis2D
 * @see BoxSelection
 */
class IVW_CORE_API AxisRangeEvent : public Event {
public:
    using Rectangle = std::array<dvec2, 2>;

    AxisRangeEvent(AxisRangeEventState state, AxisRangeInteraction interaction,
                   AxisRangeInteractionMode mode, std::optional<Rectangle> rect);
    AxisRangeEvent(const AxisRangeEvent& rhs) = default;
    AxisRangeEvent(AxisRangeEvent&& rhs) noexcept = default;
    AxisRangeEvent& operator=(const AxisRangeEvent& rhs) = default;
    AxisRangeEvent& operator=(AxisRangeEvent&& rhs) noexcept = default;
    virtual ~AxisRangeEvent() = default;

    virtual AxisRangeEvent* clone() const override;

    std::optional<Rectangle> rect() const;
    AxisRangeEventState state() const;
    AxisRangeInteraction interaction() const;
    AxisRangeInteractionMode mode() const;

    virtual uint64_t hash() const override;
    static constexpr uint64_t chash();

    virtual void print(fmt::memory_buffer& buff) const override;

private:
    AxisRangeEventState state_;
    AxisRangeInteraction interaction_;
    AxisRangeInteractionMode mode_;
    std::optional<Rectangle> rect_;
};

constexpr uint64_t AxisRangeEvent::chash() {
    return util::constexpr_hash("org.inviwo.AxisRangeEvent");
}

}  // namespace inviwo
