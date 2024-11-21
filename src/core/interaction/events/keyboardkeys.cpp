/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2024 Inviwo Foundation
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

#include <inviwo/core/interaction/events/keyboardkeys.h>
#include <inviwo/core/util/ostreamjoiner.h>
#include <inviwo/core/util/exception.h>
#include <ostream>

namespace inviwo {

std::string_view enumToStr(KeyModifier m) {
    switch (m) {
        case KeyModifier::None:
            return "None";
        case KeyModifier::Control:
            return "Control";
        case KeyModifier::Shift:
            return "Shift";
        case KeyModifier::Alt:
            return "Alt";
        case KeyModifier::Super:
            return "Super";
        case KeyModifier::Menu:
            return "Menu";
        case KeyModifier::Meta:
            return "Meta";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumToStr"), "Found invalid KeyModifier enum value '{}'",
                    static_cast<int>(m));
}
std::string_view enumToStr(KeyState s) {
    switch (s) {
        case KeyState::Press:
            return "Press";
        case KeyState::Release:
            return "Release";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumToStr"), "Found invalid KeyState enum value '{}'",
                    static_cast<int>(s));
}
std::string_view enumToStr(IvwKey k) {
    switch (k) {
        case IvwKey::Undefined:
            return "Undefined";
        case IvwKey::Unknown:
            return "Unknown";
        case IvwKey::Space:
            return "Space";
        case IvwKey::Exclam:
            return "Exclam";
        case IvwKey::QuoteDbl:
            return "QuoteDbl";
        case IvwKey::NumberSign:
            return "NumberSign";
        case IvwKey::Dollar:
            return "Dollar";
        case IvwKey::Percent:
            return "Percent";
        case IvwKey::Ampersand:
            return "Ampersand";
        case IvwKey::Apostrophe:
            return "Apostrophe";
        case IvwKey::ParenLeft:
            return "ParenLeft";
        case IvwKey::ParenRight:
            return "ParenRight";
        case IvwKey::Asterisk:
            return "Asterisk";
        case IvwKey::Plus:
            return "Plus";
        case IvwKey::Comma:
            return "Comma";
        case IvwKey::Minus:
            return "Minus";
        case IvwKey::Period:
            return "Period";
        case IvwKey::Slash:
            return "Slash";
        case IvwKey::Num0:
            return "Num0";
        case IvwKey::Num1:
            return "Num1";
        case IvwKey::Num2:
            return "Num2";
        case IvwKey::Num3:
            return "Num3";
        case IvwKey::Num4:
            return "Num4";
        case IvwKey::Num5:
            return "Num5";
        case IvwKey::Num6:
            return "Num6";
        case IvwKey::Num7:
            return "Num7";
        case IvwKey::Num8:
            return "Num8";
        case IvwKey::Num9:
            return "Num9";
        case IvwKey::Colon:
            return "Colon";
        case IvwKey::Semicolon:
            return "Semicolon";
        case IvwKey::Less:
            return "Less";
        case IvwKey::Equal:
            return "Equal";
        case IvwKey::Greater:
            return "Greater";
        case IvwKey::Question:
            return "Question";
        case IvwKey::A:
            return "A";
        case IvwKey::B:
            return "B";
        case IvwKey::C:
            return "C";
        case IvwKey::D:
            return "D";
        case IvwKey::E:
            return "E";
        case IvwKey::F:
            return "F";
        case IvwKey::G:
            return "G";
        case IvwKey::H:
            return "H";
        case IvwKey::I:
            return "I";
        case IvwKey::J:
            return "J";
        case IvwKey::K:
            return "K";
        case IvwKey::L:
            return "L";
        case IvwKey::M:
            return "M";
        case IvwKey::N:
            return "N";
        case IvwKey::O:
            return "O";
        case IvwKey::P:
            return "P";
        case IvwKey::Q:
            return "Q";
        case IvwKey::R:
            return "R";
        case IvwKey::S:
            return "S";
        case IvwKey::T:
            return "T";
        case IvwKey::U:
            return "U";
        case IvwKey::V:
            return "V";
        case IvwKey::W:
            return "W";
        case IvwKey::X:
            return "X";
        case IvwKey::Y:
            return "Y";
        case IvwKey::Z:
            return "Z";
        case IvwKey::BracketLeft:
            return "BracketLeft";
        case IvwKey::Backslash:
            return "Backslash";
        case IvwKey::BracketRight:
            return "BracketRight";
        case IvwKey::GraveAccent:
            return "GraveAccent";
        case IvwKey::AsciiCircum:
            return "AsciiCircum";
        case IvwKey::Underscore:
            return "Underscore";
        case IvwKey::BraceLeft:
            return "BraceLeft";
        case IvwKey::Bar:
            return "Bar";
        case IvwKey::BraceRight:
            return "BraceRight";
        case IvwKey::AsciiTilde:
            return "AsciiTilde";
        case IvwKey::World1:
            return "World1";
        case IvwKey::World2:
            return "World2";
        case IvwKey::Escape:
            return "Escape";
        case IvwKey::Enter:
            return "Enter";
        case IvwKey::Tab:
            return "Tab";
        case IvwKey::Backspace:
            return "Backspace";
        case IvwKey::Insert:
            return "Insert";
        case IvwKey::Delete:
            return "Delete";
        case IvwKey::Right:
            return "Right";
        case IvwKey::Left:
            return "Left";
        case IvwKey::Down:
            return "Down";
        case IvwKey::Up:
            return "Up";
        case IvwKey::PageUp:
            return "PageUp";
        case IvwKey::PageDown:
            return "PageDown";
        case IvwKey::Home:
            return "Home";
        case IvwKey::End:
            return "End";
        case IvwKey::CapsLock:
            return "CapsLock";
        case IvwKey::ScrollLock:
            return "ScrollLock";
        case IvwKey::NumLock:
            return "NumLock";
        case IvwKey::PrintScreen:
            return "PrintScreen";
        case IvwKey::Pause:
            return "Pause";
        case IvwKey::F1:
            return "F1";
        case IvwKey::F2:
            return "F2";
        case IvwKey::F3:
            return "F3";
        case IvwKey::F4:
            return "F4";
        case IvwKey::F5:
            return "F5";
        case IvwKey::F6:
            return "F6";
        case IvwKey::F7:
            return "F7";
        case IvwKey::F8:
            return "F8";
        case IvwKey::F9:
            return "F9";
        case IvwKey::F10:
            return "F10";
        case IvwKey::F11:
            return "F11";
        case IvwKey::F12:
            return "F12";
        case IvwKey::F13:
            return "F13";
        case IvwKey::F14:
            return "F14";
        case IvwKey::F15:
            return "F15";
        case IvwKey::F16:
            return "F16";
        case IvwKey::F17:
            return "F17";
        case IvwKey::F18:
            return "F18";
        case IvwKey::F19:
            return "F19";
        case IvwKey::F20:
            return "F20";
        case IvwKey::F21:
            return "F21";
        case IvwKey::F22:
            return "F22";
        case IvwKey::F23:
            return "F23";
        case IvwKey::F24:
            return "F24";
        case IvwKey::F25:
            return "F25";
        case IvwKey::KP0:
            return "KP0";
        case IvwKey::KP1:
            return "KP1";
        case IvwKey::KP2:
            return "KP2";
        case IvwKey::KP3:
            return "KP3";
        case IvwKey::KP4:
            return "KP4";
        case IvwKey::KP5:
            return "KP5";
        case IvwKey::KP6:
            return "KP6";
        case IvwKey::KP7:
            return "KP7";
        case IvwKey::KP8:
            return "KP8";
        case IvwKey::KP9:
            return "KP9";
        case IvwKey::KPDecimal:
            return "KPDecimal";
        case IvwKey::KPDivide:
            return "KPDivide";
        case IvwKey::KPMultiply:
            return "KPMultiply";
        case IvwKey::KPSubtract:
            return "KPSubtract";
        case IvwKey::KPAdd:
            return "KPAdd";
        case IvwKey::KPEnter:
            return "KPEnter";
        case IvwKey::KPEqual:
            return "KPEqual";
        case IvwKey::LeftShift:
            return "LeftShift";
        case IvwKey::LeftControl:
            return "LeftControl";
        case IvwKey::LeftAlt:
            return "LeftAlt";
        case IvwKey::LeftSuper:
            return "LeftSuper";
        case IvwKey::RightShift:
            return "RightShift";
        case IvwKey::RightControl:
            return "RightControl";
        case IvwKey::RightAlt:
            return "RightAlt";
        case IvwKey::RightSuper:
            return "RightSuper";
        case IvwKey::Menu:
            return "Menu";
        case IvwKey::LeftMeta:
            return "LeftMeta";
        case IvwKey::RightMeta:
            return "RightMeta";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumToStr"), "Found invalid IvwKey enum value '{}'",
                    static_cast<int>(k));
}

std::ostream& operator<<(std::ostream& ss, KeyModifier m) { return ss << enumToStr(m); }
std::ostream& operator<<(std::ostream& ss, KeyModifiers ms) {
    std::copy(ms.begin(), ms.end(), util::make_ostream_joiner(ss, "+"));
    return ss;
}
std::ostream& operator<<(std::ostream& ss, KeyState s) { return ss << enumToStr(s); }
std::ostream& operator<<(std::ostream& ss, KeyStates s) {
    std::copy(s.begin(), s.end(), util::make_ostream_joiner(ss, "+"));
    return ss;
}
std::ostream& operator<<(std::ostream& ss, IvwKey k) { return ss << enumToStr(k); }

}  // namespace inviwo
