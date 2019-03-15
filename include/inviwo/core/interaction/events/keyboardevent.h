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

#ifndef IVW_KEYBOARDEVENT_H
#define IVW_KEYBOARDEVENT_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/interaction/events/interactionevent.h>
#include <inviwo/core/interaction/events/keyboardkeys.h>
#include <inviwo/core/util/constexprhash.h>

namespace inviwo {
/**
 * @class KeyboardEvent
 * Generated when keys are pressed and released.
 */
class IVW_CORE_API KeyboardEvent : public InteractionEvent {
public:
    /**
     * @param key Enumerate representation of pressed key
     * @param state Pressed of released key
     * @param modifiers Modifier keys, e.g. shift, held while pressing key
     * @param nativeVirtualKey Platform dependent scancode of pressed key
     * @param utfText Unicode representation of pressed keys
     */
    KeyboardEvent(IvwKey key = IvwKey::Unknown, KeyState state = KeyState::Press,
                  KeyModifiers modifiers = KeyModifiers(flags::empty),
                  uint32_t nativeVirtualKey = 0, const std::string& utfText = u8"");

    KeyboardEvent(const KeyboardEvent& rhs) = default;
    KeyboardEvent& operator=(const KeyboardEvent& that) = default;
    virtual KeyboardEvent* clone() const override;
    virtual ~KeyboardEvent() = default;

    KeyState state() const;
    void setState(KeyState state);

    /*
     * Platform-independent code for key.
     * @note Does not differentiate between lower and uppercase letters. Use text instead.
     */
    virtual IvwKey key() const;
    void setKey(IvwKey key);

    /*
     * Returns virtual key representation of pressed key.
     * The key may be 0 even though the event contain information.
     * See https://msdn.microsoft.com/en-us/library/windows/desktop/ff468858(v=vs.85).aspx
     */
    uint32_t getNativeVirtualKey() const;
    void setNativeVirtualKey(uint32_t key);

    /*
     * Returns Unicode representation of pressed keys
     */
    std::string text() const { return text_; };
    void setText(const std::string& text) { text_ = text; }

    virtual uint64_t hash() const override;
    static constexpr uint64_t chash() { return util::constexpr_hash("org.inviwo.KeyboardEvent"); }

    virtual void print(std::ostream& ss) const override;

private:
    std::string text_;  ///< Unicode representation of pressed keys
    KeyState state_;
    IvwKey key_;
    uint32_t nativeVirtualKey_;
};

}  // namespace inviwo

#endif  // IVW_KEYBOARDEVENT_H
