/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/webbrowser/interaction/cefkeyboardmapping.h>

#include <warn/push>
#include <warn/ignore/all>
#include "include/cef_base.h"
#include <warn/pop>

namespace inviwo {

namespace cef {

int mapKey(IvwKey key) {
    switch (key) {
        case IvwKey::Backspace:
            return 8;
        case IvwKey::Tab:
            return 9;
        case IvwKey::Enter:
            return 13;
        case IvwKey::Left:
            return 37;
        case IvwKey::Up:
            return 38;
        case IvwKey::Right:
            return 39;
        case IvwKey::Down:
            return 40;
        case IvwKey::Delete:
            return 46;
        default:
            return static_cast<int>(key) + 32;
    }
}

unsigned int keyModifiers(KeyModifiers modifiers, IvwKey key) {
    unsigned int cefModifiers = 0;
    if (modifiers & KeyModifier::Control) {
        cefModifiers |= EVENTFLAG_CONTROL_DOWN;
    } else if (modifiers & KeyModifier::Shift) {
        cefModifiers |= EVENTFLAG_SHIFT_DOWN;
    } else if (modifiers & KeyModifier::Alt) {
        cefModifiers |= EVENTFLAG_ALT_DOWN;
    } else if (modifiers & KeyModifier::Super) {
        cefModifiers |= EVENTFLAG_COMMAND_DOWN;
    } else if (modifiers & KeyModifier::Menu) {
        cefModifiers |= EVENTFLAG_ALT_DOWN;
    } else if (modifiers & KeyModifier::Meta) {
    }

    if (key == IvwKey::CapsLock) {
        cefModifiers |= EVENTFLAG_CAPS_LOCK_ON;
    } else if (key == IvwKey::NumLock) {
        cefModifiers |= EVENTFLAG_NUM_LOCK_ON;
    }
    switch (key) {
        case IvwKey::Unknown:
            break;
        case IvwKey::Insert:
        case IvwKey::Delete:
        case IvwKey::Right:
        case IvwKey::Left:
        case IvwKey::Down:
        case IvwKey::Up:
        case IvwKey::PageUp:
        case IvwKey::PageDown:
        case IvwKey::Home:
        case IvwKey::End:
        case IvwKey::NumLock:
            cefModifiers |= EVENTFLAG_IS_KEY_PAD;
            break;
        case IvwKey::KP0:
        case IvwKey::KP1:
        case IvwKey::KP2:
        case IvwKey::KP3:
        case IvwKey::KP4:
        case IvwKey::KP5:
        case IvwKey::KP6:
        case IvwKey::KP7:
        case IvwKey::KP8:
        case IvwKey::KP9:
        case IvwKey::KPDecimal:
        case IvwKey::KPDivide:
        case IvwKey::KPMultiply:
        case IvwKey::KPSubtract:
        case IvwKey::KPAdd:
        case IvwKey::KPEnter:
        case IvwKey::KPEqual:
            cefModifiers |= EVENTFLAG_IS_KEY_PAD;
            break;
        case IvwKey::LeftShift:
        case IvwKey::LeftControl:
        case IvwKey::LeftAlt:
        case IvwKey::LeftSuper:
            cefModifiers |= EVENTFLAG_IS_LEFT;
            break;
        case IvwKey::RightShift:
        case IvwKey::RightControl:
        case IvwKey::RightAlt:
        case IvwKey::RightSuper:
            cefModifiers |= EVENTFLAG_IS_RIGHT;
            break;
        case IvwKey::Menu:
            break;
        case IvwKey::LeftMeta:
        case IvwKey::RightMeta:
            cefModifiers |= EVENTFLAG_IS_KEY_PAD;
            break;
        default:
            break;
    }
    return cefModifiers;
}

};  // namespace cef

};  // namespace inviwo
