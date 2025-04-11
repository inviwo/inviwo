/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/util/constexprhash.h>
#include <inviwo/core/util/glmvec.h>

#include <string_view>

namespace inviwo {

/**
 * A ContextMenuEvent is triggered when a custom context menu entry is selected.
 *
 * \see ContextMenuEntry
 */
class IVW_CORE_API ContextMenuEvent : public MouseEvent {
public:
    explicit ContextMenuEvent(std::string_view id, MouseButton button = MouseButton::Right,
                              MouseState state = MouseState::Release,
                              MouseButtons buttonState = MouseButtons(flags::empty),
                              KeyModifiers modifiers = KeyModifiers(flags::empty),
                              dvec2 normalizedPosition = dvec2(0), uvec2 canvasSize = uvec2(0),
                              double depth = 1.0);
    ContextMenuEvent(const ContextMenuEvent&) = default;
    ContextMenuEvent(ContextMenuEvent&&) = default;
    ContextMenuEvent& operator=(const ContextMenuEvent&) = default;
    ContextMenuEvent& operator=(ContextMenuEvent&&) = default;

    virtual ~ContextMenuEvent();

    virtual ContextMenuEvent* clone() const override;

    /**
     * ID of the triggered context menu action
     */
    std::string_view getId() const;

    virtual uint64_t hash() const override;
    static constexpr uint64_t chash() {
        return util::constexpr_hash("org.inviwo.ContextMenuEvent");
    }

private:
    std::string id_;
};

}  // namespace inviwo
