/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_TOUCHEVENT_H
#define IVW_TOUCHEVENT_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/interaction/events/interactionevent.h>

namespace inviwo {

class IVW_CORE_API TouchEvent : public InteractionEvent {
public:
    enum TouchState {
        TOUCH_STATE_NONE = 0,
        TOUCH_STATE_STARTED,
        TOUCH_STATE_UPDATED,
        TOUCH_STATE_ENDED
    };



    TouchEvent(ivec2 pos, TouchEvent::TouchState state);
    
    TouchEvent(const TouchEvent& rhs);
    TouchEvent& operator=(const TouchEvent& that);
    virtual TouchEvent* clone() const;
    virtual ~TouchEvent();

    inline ivec2 pos() const { return position_; }
    inline TouchEvent::TouchState state() const { return state_; }

    virtual std::string getClassIdentifier() const { return "org.inviwo.TouchEvent"; }

    virtual void serialize(IvwSerializer& s) const;
    virtual void deserialize(IvwDeserializer& d);

    virtual bool matching(const Event* aEvent) const;
    virtual bool matching(const TouchEvent* aEvent) const;
    virtual bool equalSelectors(const Event* aEvent) const;

private:
    ivec2 position_;
    TouchEvent::TouchState state_;
};

} // namespace

#endif // IVW_TOUCHEVENT_H