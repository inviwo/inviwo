/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#ifndef IVW_KEYBOARDEVENT_H
#define IVW_KEYBOARDEVENT_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/interaction/events/interactionevent.h>

namespace inviwo {

class IVW_CORE_API KeyboardEvent : public InteractionEvent {
public:
    enum KeyState { 
        KEY_STATE_NONE = 0, 
        KEY_STATE_PRESS = 1, 
        KEY_STATE_RELEASE = 2,
        KEY_STATE_ANY = KEY_STATE_NONE | KEY_STATE_PRESS | KEY_STATE_RELEASE
    };

    KeyboardEvent(int key = 0,
                  int modifiers = InteractionEvent::MODIFIER_NONE,
                  int state = KeyboardEvent::KEY_STATE_PRESS);

    KeyboardEvent(const KeyboardEvent& rhs);
    KeyboardEvent& operator=(const KeyboardEvent& that);
    virtual KeyboardEvent* clone() const;
    virtual ~KeyboardEvent();

    int state() const;
    void setState(int state);
  
    virtual int button() const;
    void setButton(int button);

    virtual std::string getClassIdentifier() const;
    virtual void serialize(IvwSerializer& s) const;
    virtual void deserialize(IvwDeserializer& d);

    virtual bool matching(const Event* aEvent) const;
    virtual bool matching(const KeyboardEvent* aEvent) const;
    virtual bool equalSelectors(const Event* aEvent) const;

private:
    // Event selectors:
    int state_;
    int key_;
};

}  // namespace

#endif  // IVW_KEYBOARDEVENT_H