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

#ifndef IVW_MOUSEEVENT_H
#define IVW_MOUSEEVENT_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/interaction/events/mouseinteractionevent.h>
#include <inviwo/core/interaction/events/mousebuttons.h>
#include <inviwo/core/util/constexprhash.h>

namespace inviwo {

class IVW_CORE_API MouseEvent : public MouseInteractionEvent {
public:
    MouseEvent(MouseButton button = MouseButton::Left, MouseState state = MouseState::Press,
               MouseButtons buttonState = MouseButtons(flags::empty),
               KeyModifiers modifiers = KeyModifiers(flags::empty),
               dvec2 normalizedPosition = dvec2(0), uvec2 canvasSize = uvec2(0),
               double depth = 1.0);

    MouseEvent(const MouseEvent& rhs) = default;
    MouseEvent& operator=(const MouseEvent& that) = default;
    virtual MouseEvent* clone() const override;
    virtual ~MouseEvent() = default;

    /**
     *	The button responsible for the current event.
     */
    MouseButton button() const;
    void setButton(MouseButton button);

    /**
     *	The state of the button that generated the event.
     */
    MouseState state() const;
    void setState(MouseState state);

    virtual uint64_t hash() const override;
    static constexpr uint64_t chash() { return util::constexpr_hash("org.inviwo.MouseEvent"); }

    virtual void print(std::ostream& ss) const override;

private:
    MouseButton button_;
    MouseState state_;
};

}  // namespace inviwo

#endif  // IVW_MOUSEEVENT_H
