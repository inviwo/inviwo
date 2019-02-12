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

#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/interaction/events/eventutil.h>

namespace inviwo {

KeyboardEvent::KeyboardEvent(IvwKey key, KeyState state, KeyModifiers modifiers,
                             uint32_t nativeVirtualKey, const std::string& text)
    : InteractionEvent(modifiers)
    , text_(text)
    , state_(state)
    , key_(key)
    , nativeVirtualKey_(nativeVirtualKey) {}

KeyboardEvent* KeyboardEvent::clone() const { return new KeyboardEvent(*this); }

KeyState KeyboardEvent::state() const { return state_; }

IvwKey KeyboardEvent::key() const { return key_; }

void KeyboardEvent::setState(KeyState state) { state_ = state; }

void KeyboardEvent::setKey(IvwKey button) { key_ = button; }

uint32_t KeyboardEvent::getNativeVirtualKey() const { return nativeVirtualKey_; }

void KeyboardEvent::setNativeVirtualKey(uint32_t key) { nativeVirtualKey_ = key; }

uint64_t KeyboardEvent::hash() const { return chash(); }

void KeyboardEvent::print(std::ostream& ss) const {
    util::printEvent(ss, "KeyboardEvent", std::make_pair("state", state_),
                     std::make_pair("key", key_), std::make_pair("modifiers", modifiers_));
}

}  // namespace inviwo
