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

#ifndef IVW_GESTUREEVENT_H
#define IVW_GESTUREEVENT_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/interaction/events/interactionevent.h>

namespace inviwo {

class IVW_CORE_API GestureEvent : public InteractionEvent {
public:
    enum GestureType { PAN = 0, PINCH, SWIPE, COUNT };

    enum GestureState {
        GESTURE_STATE_NONE = 0,
        GESTURE_STATE_STARTED = 1,
        GESTURE_STATE_UPDATED = 2,
        GESTURE_STATE_ENDED = 4,
        GESTURE_STATE_CANCELED = 8,
        GESTURE_STATE_ANY = GESTURE_STATE_STARTED | GESTURE_STATE_UPDATED | GESTURE_STATE_ENDED |
                            GESTURE_STATE_CANCELED
    };

    GestureEvent(vec2 deltaPos, double deltaDistance, GestureEvent::GestureType type,
                 int state, int numFingers, vec2 screenPosNorm, uvec2 canvasSize);

    GestureEvent(GestureEvent::GestureType type, int state, int numFingers);

    GestureEvent(const GestureEvent& rhs);
    GestureEvent& operator=(const GestureEvent& that);
    virtual GestureEvent* clone() const;
    virtual ~GestureEvent();

    inline vec2 deltaPos() const { return deltaPos_; }
    inline double deltaDistance() const { return deltaDistance_; }
    inline GestureEvent::GestureType type() const { return type_; }
    inline int state() const { return state_; }
    inline int numFingers() { return numFingers_; }
    inline vec2 screenPosNormalized() { return screenPosNorm_; }
    inline vec2 canvasSize() { return canvasSize_; }

    void modify(vec2);

    virtual std::string getClassIdentifier() const { return "org.inviwo.GestureEvent"; }

    virtual void serialize(IvwSerializer& s) const;
    virtual void deserialize(IvwDeserializer& d);

    virtual bool matching(const Event* aEvent) const;
    virtual bool matching(const GestureEvent* aEvent) const;
    virtual bool equalSelectors(const Event* aEvent) const;

private:
    // Event selectors
    GestureEvent::GestureType type_;
    int state_;
    int numFingers_;

    // Event state
    vec2 deltaPos_;
    double deltaDistance_;
    vec2 screenPosNorm_;
    uvec2 canvasSize_;
};

}  // namespace

#endif  // IVW_GESTUREEVENT_H