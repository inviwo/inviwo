/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#ifndef IVW_WHEELEVENT_H
#define IVW_WHEELEVENT_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/interaction/events/mouseinteractionevent.h>
#include <inviwo/core/interaction/events/mousebuttons.h>

namespace inviwo {

class IVW_CORE_API WheelEvent : public MouseInteractionEvent { 
public:
    WheelEvent(MouseButtons buttonState = MouseButtons(flags::empty),
               KeyModifiers modifiers = KeyModifiers(flags::empty), 
               ivec2 delta = ivec2(0),
               ivec2 position = ivec2(0),
               uvec2 canvasSize = uvec2(0),
               double depth = 1.0);

    WheelEvent(const WheelEvent& rhs) = default;
    WheelEvent& operator=(const WheelEvent& that) = default;
    virtual WheelEvent* clone() const override;

    virtual ~WheelEvent() = default;

    ivec2 delta() const;
    void setDelta(ivec2 delta);

    virtual std::string getClassIdentifier() const override;

private:
    ivec2 delta_;
    ivec2 position_;
    uvec2 canvasSize_;
    double depth_;  ///< Depth in normalized device coordinates [-1 1].
};

} // namespace

#endif // IVW_WHEELEVENT_H

