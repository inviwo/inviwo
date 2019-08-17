/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_EVENTPROPERTY_H
#define IVW_EVENTPROPERTY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/interaction/events/eventmatcher.h>
#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/interaction/events/mousebuttons.h>
#include <inviwo/core/interaction/events/keyboardkeys.h>

namespace inviwo {

/**
 * \ingroup properties
 * Property which contains one event matcher and one action to represent the current
 * key binding for the contained action.
 * @see EventPropertyWidgetQt
 */
class IVW_CORE_API EventProperty : public Property {
public:
    using Action = std::function<void(Event*)>;

    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    /**
     * \brief Constructor used to create a new action-key binding.
     *
     * The constructor creates a new binding between a specified action and event.
     *
     * @param identifier
     * @param displayName
     * @param matcher The selection of events to bind to an action
     * @param action The action to executed upon the event.
     * @param invalidationLevel
     * @param semantics
     */
    EventProperty(const std::string& identifier, const std::string& displayName, Action action,
                  std::unique_ptr<EventMatcher> matcher,
                  InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                  PropertySemantics semantics = PropertySemantics::Default);

    EventProperty(const std::string& identifier, const std::string& displayName, Action action,
                  IvwKey key, KeyStates states = KeyState::Press,
                  KeyModifiers modifier = KeyModifiers(flags::none),
                  InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                  PropertySemantics semantics = PropertySemantics::Default);

    EventProperty(const std::string& identifier, const std::string& displayName, Action action,
                  MouseButtons buttons, MouseStates states = MouseState::Press,
                  KeyModifiers modifiers = KeyModifiers(flags::none),
                  InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                  PropertySemantics semantics = PropertySemantics::Default);

    EventProperty(const EventProperty& rhs);
    virtual EventProperty* clone() const override;
    virtual ~EventProperty() = default;

    void invokeEvent(Event*);

    void setEventMatcher(std::unique_ptr<EventMatcher> matcher);
    EventMatcher* getEventMatcher() const;

    void setAction(Action action);
    Action getAction() const;

    bool isEnabled() const;
    void setEnabled(bool enabled);

    virtual EventProperty& setCurrentStateAsDefault() override;
    virtual EventProperty& resetToDefaultState() override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

private:
    std::unique_ptr<EventMatcher> matcher_;
    Action action_;
    bool enabled_ = true;
};

}  // namespace inviwo

#endif  // IVW_EVENTPROPERTY_H
