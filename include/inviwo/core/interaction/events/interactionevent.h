/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_INTERACTIONEVENT_H
#define IVW_INTERACTIONEVENT_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/io/serialization/ivwserializable.h>
#include <inviwo/core/interaction/events/event.h>

namespace inviwo {

class IVW_CORE_API InteractionEvent : public Event {
public:
    enum Modifier { 
        MODIFIER_NONE = 0, 
        MODIFIER_ALT = 1, 
        MODIFIER_CTRL = 2, 
        MODIFIER_SHIFT = 4, 
        MODIFIER_ANY = MODIFIER_ALT | MODIFIER_CTRL | MODIFIER_SHIFT
    };

    InteractionEvent(int modifiers = MODIFIER_NONE);
    InteractionEvent(const InteractionEvent& rhs);
    InteractionEvent& operator=(const InteractionEvent& that);
    virtual InteractionEvent* clone() const;
    virtual ~InteractionEvent();

    int modifiers() const;
    void setModifiers(int modifiers);
    std::string modifierNames() const;
 
    virtual std::string getClassIdentifier() const;

    virtual void serialize(IvwSerializer& s) const;
    virtual void deserialize(IvwDeserializer& d);

    virtual bool matching(const Event* aEvent) const;
    virtual bool equalSelectors(const Event* event) const;

protected:
    int modifiers_;
    static const std::string modifierNames_[4];
};

}  // namespace

#endif // IVW_INTERACTIONEVENT_H