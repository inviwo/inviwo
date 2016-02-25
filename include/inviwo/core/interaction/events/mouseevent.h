/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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
#include <inviwo/core/interaction/events/interactionevent.h>

namespace inviwo {

class IVW_CORE_API MouseEvent : public InteractionEvent {
public:
    enum MouseButton {
        MOUSE_BUTTON_NONE = 1 << 0,
        MOUSE_BUTTON_LEFT = 1 << 1,
        MOUSE_BUTTON_MIDDLE = 1 << 2,
        MOUSE_BUTTON_RIGHT = 1 << 3,
        MOUSE_BUTTON_ANY = MOUSE_BUTTON_LEFT | MOUSE_BUTTON_MIDDLE | MOUSE_BUTTON_RIGHT,
        MOUSE_BUTTON_ANY_AND_NONE = MOUSE_BUTTON_NONE | MOUSE_BUTTON_ANY
    };

    enum MouseState {
        MOUSE_STATE_NONE = 1 << 0,
        MOUSE_STATE_MOVE = 1 << 1,
        MOUSE_STATE_PRESS = 1 << 2,
        MOUSE_STATE_RELEASE = 1 << 3,
        MOUSE_STATE_WHEEL = 1 << 4,
        MOUSE_STATE_DOUBLE_CLICK = 1 << 5 , 
        MOUSE_STATE_ANY = MOUSE_STATE_MOVE | MOUSE_STATE_PRESS | MOUSE_STATE_RELEASE | MOUSE_STATE_WHEEL | MOUSE_STATE_DOUBLE_CLICK,
        MOUSE_STATE_ANY_AND_NONE = MOUSE_STATE_NONE | MOUSE_STATE_ANY
    };

    enum MouseWheelOrientation {
        MOUSE_WHEEL_NONE = 1 << 0,
        MOUSE_WHEEL_HORIZONTAL = 1 << 1,
        MOUSE_WHEEL_VERTICAL = 1 << 2,
        MOUSE_WHEEL_ANY = MOUSE_WHEEL_HORIZONTAL | MOUSE_WHEEL_VERTICAL,
        MOUSE_WHEEL_ANY_AND_NONE = MOUSE_WHEEL_NONE | MOUSE_WHEEL_ANY
    };

    MouseEvent();

    // Mouse and wheel event
    MouseEvent(ivec2 position, int delta, int button, int state = MOUSE_STATE_NONE,
               int orientation = MOUSE_WHEEL_NONE, int modifiers = InteractionEvent::MODIFIER_NONE,
               uvec2 canvasSize = uvec2(0), double depth = 1.0);

    // Mouse event
    MouseEvent(ivec2 position, int button, int state = MOUSE_STATE_NONE,
               int modifiers = InteractionEvent::MODIFIER_NONE, 
               uvec2 canvasSize = uvec2(0), double depth = 1.0);

    // Selector
    MouseEvent(int button, int modifiers = InteractionEvent::MODIFIER_NONE,
               int state = MOUSE_STATE_NONE, int orientation = MOUSE_WHEEL_NONE);

    MouseEvent(const MouseEvent& rhs);
    MouseEvent& operator=(const MouseEvent& that);
    virtual MouseEvent* clone() const;

    virtual ~MouseEvent();

    inline ivec2 pos() const { return position_; }
    inline vec2 posNormalized() const { return vec2(vec2(position_) / vec2(canvasSize_)); }
    inline int wheelSteps() const { return wheelSteps_; }
    inline unsigned int x() const { return position_.x; }
    inline unsigned int y() const { return position_.y; }
    inline int state() const { return state_; }
    inline int wheelOrientation() const { return wheelOrientation_; }
    inline uvec2 canvasSize() const { return canvasSize_; }
    /**
    * Retrieve depth value in normalized device coordinates at mouse position.
    * Defined in [-1 1], where -1 is the near plane and 1 is the far plane.
    * Will be 1 if no depth value is available.
    */
    inline double depth() const { return depth_; }
    inline int button() const { return button_; }
    inline void setButton(int button) { button_ = button; }
    std::string buttonName() const;

    void modify(ivec2, uvec2);

    virtual std::string getClassIdentifier() const;

    virtual void serialize(Serializer& s) const;
    virtual void deserialize(Deserializer& d);

    virtual bool matching(const Event* aEvent) const;
    virtual bool matching(const MouseEvent* aEvent) const;
    virtual bool equalSelectors(const Event* aEvent) const;
    
private:
    // Event selectors:
    int button_;
    int state_;
    int wheelOrientation_;

    // Event state:
    ivec2 position_;
    int wheelSteps_;
    uvec2 canvasSize_;
    double depth_; ///< Depth in normalized device coordinates [-1 1].

    static const std::string buttonNames_[4];
};

}  // namespace

#endif  // IVW_MOUSEEVENT_H