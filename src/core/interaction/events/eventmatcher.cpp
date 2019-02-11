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

#include <inviwo/core/interaction/events/eventmatcher.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/wheelevent.h>
#include <inviwo/core/interaction/events/gestureevent.h>

namespace inviwo {

void EventMatcher::setCurrentStateAsDefault() {}
void EventMatcher::resetToDefaultState() {}
void EventMatcher::serialize(Serializer&) const {}
void EventMatcher::deserialize(Deserializer&) {}

KeyboardEventMatcher::KeyboardEventMatcher(IvwKey key, KeyStates states, KeyModifiers modifiers)
    : EventMatcher()
    , key_("key", key)
    , states_("states", states)
    , modifiers_("modifiers", modifiers) {}

KeyboardEventMatcher* KeyboardEventMatcher::clone() const {
    return new KeyboardEventMatcher(*this);
}

bool KeyboardEventMatcher::operator()(Event* e) {
    if (e->hash() != KeyboardEvent::chash()) return false;

    auto ke = static_cast<KeyboardEvent*>(e);
    if (ke->key() == key_) {
        if (states_.value & ke->state()) {
            if (modifiers_ == KeyModifiers(flags::any) || modifiers_ == ke->modifiers()) {
                return true;
            }
        }
    }

    return false;
}

IvwKey KeyboardEventMatcher::key() const { return key_; }
void KeyboardEventMatcher::setKey(IvwKey key) { key_ = key; }

KeyStates KeyboardEventMatcher::states() const { return states_; }
void KeyboardEventMatcher::setStates(KeyStates states) { states_ = states; }

KeyModifiers KeyboardEventMatcher::modifiers() const { return modifiers_; }
void KeyboardEventMatcher::setModifiers(KeyModifiers modifiers) { modifiers_ = modifiers; }

void KeyboardEventMatcher::setCurrentStateAsDefault() {
    util::for_each_argument([](auto& x) { x.setAsDefault(); }, key_, states_, modifiers_);
}
void KeyboardEventMatcher::resetToDefaultState() {
    util::for_each_argument([](auto& x) { x.reset(); }, key_, states_, modifiers_);
}

void KeyboardEventMatcher::serialize(Serializer& s) const {
    EventMatcher::serialize(s);
    util::for_each_argument([&s](const auto& x) { x.serialize(s); }, key_, states_, modifiers_);
}
void KeyboardEventMatcher::deserialize(Deserializer& d) {
    EventMatcher::deserialize(d);
    util::for_each_argument([&d](auto& x) { x.deserialize(d); }, key_, states_, modifiers_);
}

MouseEventMatcher::MouseEventMatcher(MouseButtons buttons, MouseStates states,
                                     KeyModifiers modifiers)
    : EventMatcher()
    , buttons_("buttons", buttons)
    , states_("states", states)
    , modifiers_("modifiers", modifiers) {}

MouseEventMatcher* MouseEventMatcher::clone() const { return new MouseEventMatcher(*this); }

bool MouseEventMatcher::operator()(Event* e) {
    if (e->hash() != MouseEvent::chash()) return false;
    auto me = static_cast<MouseEvent*>(e);

    if (me->state() == MouseState::Move) {
        if (states_.value & me->state()) {
            if (modifiers_ == KeyModifiers(flags::any) || modifiers_ == me->modifiers()) {
                if (me->buttonState() != MouseButton::None) {
                    for (auto s : me->buttonState()) {
                        if (buttons_.value & s) {
                            return true;
                        }
                    }
                } else if (buttons_.value == MouseButton::None) {
                    return true;
                } else if (buttons_.value == MouseButtons(flags::any)) {
                    return true;
                }
            }
        }
    } else {
        if (buttons_.value & me->button()) {
            if (states_.value & me->state()) {
                if (modifiers_ == KeyModifiers(flags::any) || modifiers_ == me->modifiers()) {
                    return true;
                }
            }
        }
    }

    return false;
}

MouseButtons MouseEventMatcher::buttons() const { return buttons_; }
void MouseEventMatcher::setButtons(MouseButtons buttons) { buttons_ = buttons; }

MouseStates MouseEventMatcher::states() const { return states_; }
void MouseEventMatcher::setStates(MouseStates states) { states_ = states; }

KeyModifiers MouseEventMatcher::modifiers() const { return modifiers_; }
void MouseEventMatcher::setModifiers(KeyModifiers modifiers) { modifiers_ = modifiers; }

void MouseEventMatcher::setCurrentStateAsDefault() {
    util::for_each_argument([](auto& x) { x.setAsDefault(); }, buttons_, states_, modifiers_);
}
void MouseEventMatcher::resetToDefaultState() {
    util::for_each_argument([](auto& x) { x.reset(); }, buttons_, states_, modifiers_);
}

void MouseEventMatcher::serialize(Serializer& s) const {
    EventMatcher::serialize(s);
    util::for_each_argument([&s](const auto& x) { x.serialize(s); }, buttons_, states_, modifiers_);
}
void MouseEventMatcher::deserialize(Deserializer& d) {
    EventMatcher::deserialize(d);
    util::for_each_argument([&d](auto& x) { x.deserialize(d); }, buttons_, states_, modifiers_);
}

WheelEventMatcher::WheelEventMatcher(KeyModifiers modifiers)
    : EventMatcher(), modifiers_("modifiers", modifiers) {}

WheelEventMatcher* WheelEventMatcher::clone() const { return new WheelEventMatcher(*this); }

bool WheelEventMatcher::operator()(Event* e) {
    if (e->hash() != WheelEvent::chash()) return false;
    auto we = static_cast<WheelEvent*>(e);

    if (modifiers_ == KeyModifiers(flags::any) || modifiers_ == we->modifiers()) {
        return true;
    }

    return false;
}

KeyModifiers WheelEventMatcher::modifiers() const { return modifiers_; }
void WheelEventMatcher::setModifiers(KeyModifiers modifiers) { modifiers_ = modifiers; }

void WheelEventMatcher::setCurrentStateAsDefault() {
    util::for_each_argument([](auto& x) { x.setAsDefault(); }, modifiers_);
}
void WheelEventMatcher::resetToDefaultState() {
    util::for_each_argument([](auto& x) { x.reset(); }, modifiers_);
}

void WheelEventMatcher::serialize(Serializer& s) const {
    EventMatcher::serialize(s);
    util::for_each_argument([&s](const auto& x) { x.serialize(s); }, modifiers_);
}
void WheelEventMatcher::deserialize(Deserializer& d) {
    EventMatcher::deserialize(d);
    util::for_each_argument([&d](auto& x) { x.deserialize(d); }, modifiers_);
}

GestureEventMatcher::GestureEventMatcher(GestureTypes types, GestureStates states, int numFingers,
                                         KeyModifiers modifiers)
    : EventMatcher()
    , types_("types", types)
    , states_("states", states)
    , numFingers_("numFingers", numFingers)
    , modifiers_("modifiers", modifiers) {}

GestureEventMatcher* GestureEventMatcher::clone() const { return new GestureEventMatcher(*this); }

bool GestureEventMatcher::operator()(Event* e) {
    if (e->hash() != GestureEvent::chash()) return false;
    auto ge = static_cast<GestureEvent*>(e);

    if (types_.value & ge->type()) {
        if (states_.value & ge->state()) {
            if (numFingers_ == -1 || numFingers_ == ge->numFingers()) {
                if (modifiers_ == KeyModifiers(flags::any) || modifiers_ == ge->modifiers()) {
                    return true;
                }
            }
        }
    }

    return false;
}

GestureTypes GestureEventMatcher::types() const { return types_; }
void GestureEventMatcher::setTypes(GestureTypes types) { types_ = types; }

GestureStates GestureEventMatcher::states() const { return states_; }
void GestureEventMatcher::setStates(GestureState states) { states_ = states; }

int GestureEventMatcher::numFingers() const { return numFingers_; }
void GestureEventMatcher::setNumFingers(int numFingers) { numFingers_ = numFingers; }

KeyModifiers GestureEventMatcher::modifiers() const { return modifiers_; }
void GestureEventMatcher::setModifiers(KeyModifiers modifiers) { modifiers_ = modifiers; }

void GestureEventMatcher::setCurrentStateAsDefault() {
    util::for_each_argument([](auto& x) { x.setAsDefault(); }, types_, states_, numFingers_,
                            modifiers_);
}
void GestureEventMatcher::resetToDefaultState() {
    util::for_each_argument([](auto& x) { x.reset(); }, types_, states_, numFingers_, modifiers_);
}

void GestureEventMatcher::serialize(Serializer& s) const {
    EventMatcher::serialize(s);
    util::for_each_argument([&s](const auto& x) { x.serialize(s); }, types_, states_, numFingers_,
                            modifiers_);
}
void GestureEventMatcher::deserialize(Deserializer& d) {
    EventMatcher::deserialize(d);
    util::for_each_argument([&d](auto& x) { x.deserialize(d); }, types_, states_, numFingers_,
                            modifiers_);
}

GeneralEventMatcher::GeneralEventMatcher(std::function<bool(Event*)> matcher)
    : EventMatcher(), matcher_{std::move(matcher)} {}

GeneralEventMatcher* GeneralEventMatcher::clone() const { return new GeneralEventMatcher(*this); }

bool GeneralEventMatcher::operator()(Event* e) { return matcher_(e); }

}  // namespace inviwo
