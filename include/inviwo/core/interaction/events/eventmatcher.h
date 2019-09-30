/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_EVENTMATCHER_H
#define IVW_EVENTMATCHER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/valuewrapper.h>
#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/io/serialization/serializable.h>

#include <inviwo/core/interaction/events/mousebuttons.h>
#include <inviwo/core/interaction/events/keyboardkeys.h>
#include <inviwo/core/interaction/events/gesturestate.h>

namespace inviwo {

/**
 * \class EventMatcher
 * \brief A class to represent a event matcher for use in event properties.
 */
class IVW_CORE_API EventMatcher : public Serializable {
public:
    EventMatcher() = default;
    virtual ~EventMatcher() = default;
    virtual EventMatcher* clone() const = 0;

    virtual bool operator()(Event*) = 0;

    virtual void setCurrentStateAsDefault();
    virtual void resetToDefaultState();
    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

protected:
    EventMatcher(const EventMatcher&) = default;
    EventMatcher& operator=(const EventMatcher&) = default;
};

class IVW_CORE_API KeyboardEventMatcher : public EventMatcher {
public:
    KeyboardEventMatcher(IvwKey key, KeyStates states = KeyState::Press,
                         KeyModifiers modifier = KeyModifiers(flags::none));

    virtual ~KeyboardEventMatcher() = default;
    virtual KeyboardEventMatcher* clone() const override;

    virtual bool operator()(Event*) override;

    IvwKey key() const;
    void setKey(IvwKey key);

    KeyStates states() const;
    void setStates(KeyStates states);

    KeyModifiers modifiers() const;
    void setModifiers(KeyModifiers modifiers);

    virtual void setCurrentStateAsDefault() override;
    virtual void resetToDefaultState() override;
    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

protected:
    KeyboardEventMatcher(const KeyboardEventMatcher&) = default;
    KeyboardEventMatcher& operator=(const KeyboardEventMatcher&) = default;

private:
    ValueWrapper<IvwKey> key_;
    ValueWrapper<KeyStates> states_;
    ValueWrapper<KeyModifiers> modifiers_;
};

class IVW_CORE_API MouseEventMatcher : public EventMatcher {
public:
    MouseEventMatcher(MouseButtons buttons, MouseStates states = MouseState::Press,
                      KeyModifiers modifiers = KeyModifiers(flags::none));

    virtual ~MouseEventMatcher() = default;
    virtual MouseEventMatcher* clone() const override;

    virtual bool operator()(Event*) override;

    MouseButtons buttons() const;
    void setButtons(MouseButtons key);

    MouseStates states() const;
    void setStates(MouseStates states);

    KeyModifiers modifiers() const;
    void setModifiers(KeyModifiers modifiers);

    virtual void setCurrentStateAsDefault() override;
    virtual void resetToDefaultState() override;
    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

protected:
    MouseEventMatcher(const MouseEventMatcher&) = default;
    MouseEventMatcher& operator=(const MouseEventMatcher&) = default;

private:
    ValueWrapper<MouseButtons> buttons_;
    ValueWrapper<MouseStates> states_;
    ValueWrapper<KeyModifiers> modifiers_;
};

class IVW_CORE_API WheelEventMatcher : public EventMatcher {
public:
    WheelEventMatcher(KeyModifiers modifiers = KeyModifiers(flags::none));

    virtual ~WheelEventMatcher() = default;
    virtual WheelEventMatcher* clone() const override;

    virtual bool operator()(Event*) override;

    KeyModifiers modifiers() const;
    void setModifiers(KeyModifiers modifiers);

    virtual void setCurrentStateAsDefault() override;
    virtual void resetToDefaultState() override;
    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

protected:
    WheelEventMatcher(const WheelEventMatcher&) = default;
    WheelEventMatcher& operator=(const WheelEventMatcher&) = default;

private:
    ValueWrapper<KeyModifiers> modifiers_;
};

class IVW_CORE_API GestureEventMatcher : public EventMatcher {
public:
    GestureEventMatcher(GestureTypes types, GestureStates states = GestureStates(flags::any),
                        int numFingers = -1, KeyModifiers modifiers = KeyModifiers(flags::none));

    virtual ~GestureEventMatcher() = default;
    virtual GestureEventMatcher* clone() const override;

    virtual bool operator()(Event*) override;

    GestureTypes types() const;
    void setTypes(GestureTypes key);

    GestureStates states() const;
    void setStates(GestureState states);

    int numFingers() const;
    void setNumFingers(int numFingers);

    KeyModifiers modifiers() const;
    void setModifiers(KeyModifiers modifiers);

    virtual void setCurrentStateAsDefault() override;
    virtual void resetToDefaultState() override;
    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

protected:
    GestureEventMatcher(const GestureEventMatcher&) = default;
    GestureEventMatcher& operator=(const GestureEventMatcher&) = default;

private:
    ValueWrapper<GestureTypes> types_;
    ValueWrapper<GestureStates> states_;
    ValueWrapper<int> numFingers_;
    ValueWrapper<KeyModifiers> modifiers_;
};

class IVW_CORE_API GeneralEventMatcher : public EventMatcher {
public:
    GeneralEventMatcher(std::function<bool(Event*)> matcher);
    virtual ~GeneralEventMatcher() = default;
    virtual GeneralEventMatcher* clone() const override;

    virtual bool operator()(Event*) override;

    template <typename EventType>
    static std::unique_ptr<GeneralEventMatcher> create();

protected:
    GeneralEventMatcher(const GeneralEventMatcher&) = default;
    GeneralEventMatcher& operator=(const GeneralEventMatcher&) = default;

private:
    std::function<bool(Event*)> matcher_;
};

template <typename EventType>
std::unique_ptr<GeneralEventMatcher> GeneralEventMatcher::create() {
    return std::make_unique<GeneralEventMatcher>(
        [](Event* e) -> bool { return e->hash() == EventType::chash(); });
}

}  // namespace inviwo

#endif  // IVW_EVENTMATCHER_H
