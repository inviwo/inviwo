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

#ifndef IVW_KEYBOARDKEYS_H
#define IVW_KEYBOARDKEYS_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/ostreamjoiner.h>

#include <flags/flags.h>

#include <iterator>
#include <ostream>

namespace inviwo {

enum class KeyModifier {
    None = 0,
    Control = 1 << 0,
    Shift = 1 << 1,
    Alt = 1 << 2,
    Super = 1 << 3,
    Menu = 1 << 4,
    Meta = 1 << 5
};
ALLOW_FLAGS_FOR_ENUM(KeyModifier)
using KeyModifiers = flags::flags<KeyModifier>;

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss, KeyModifier m) {
    switch (m) {
        case KeyModifier::None:
            ss << "None";
            break;
        case KeyModifier::Control:
            ss << "Control";
            break;
        case KeyModifier::Shift:
            ss << "Shift";
            break;
        case KeyModifier::Alt:
            ss << "Alt";
            break;
        case KeyModifier::Super:
            ss << "Super";
            break;
        case KeyModifier::Menu:
            ss << "Menu";
            break;
        case KeyModifier::Meta:
            ss << "Meta";
            break;
        default:
            break;
    }
    return ss;
}
template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                             KeyModifiers ms) {
    std::copy(ms.begin(), ms.end(), util::make_ostream_joiner(ss, "+"));
    return ss;
}

enum class KeyState {
    Press = 1 << 0,
    Release = 1 << 1,
};
ALLOW_FLAGS_FOR_ENUM(KeyState)
using KeyStates = flags::flags<KeyState>;

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss, KeyState s) {
    switch (s) {
        case KeyState::Press:
            ss << "Press";
            break;
        case KeyState::Release:
            ss << "Release";
            break;
    }
    return ss;
}
template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss, KeyStates s) {
    std::copy(s.begin(), s.end(), util::make_ostream_joiner(ss, "+"));
    return ss;
}

enum class IvwKey {
    Undefined = -2,
    // key codes based on glfw
    Unknown = -1,

    /* Printable Keys */
    Space = 32,
    Exclam = 33,   /* ! */
    QuoteDbl = 34, /* " */
    NumberSign = 35,
    Dollar = 36,     /* $ */
    Percent = 37,    /* % */
    Ampersand = 38,  /* @ */
    Apostrophe = 39, /* ' */
    ParenLeft = 40,  /* ( */
    ParenRight = 41, /* ) */
    Asterisk = 42,   /* * */
    Plus = 43,       /* + */
    Comma = 44,      /* , */
    Minus = 45,      /* - */
    Period = 46,     /* . */
    Slash = 47,      /* / */
    Num0 = 48,
    Num1 = 49,
    Num2 = 50,
    Num3 = 51,
    Num4 = 52,
    Num5 = 53,
    Num6 = 54,
    Num7 = 55,
    Num8 = 56,
    Num9 = 57,
    Colon = 58,     /* : */
    Semicolon = 59, /* ; */
    Less = 60,      /* < */
    Equal = 61,     /* = */
    Greater = 62,   /* > */
    Question = 63,  /* ? */
    A = 65,
    B = 66,
    C = 67,
    D = 68,
    E = 69,
    F = 70,
    G = 71,
    H = 72,
    I = 73,
    J = 74,
    K = 75,
    L = 76,
    M = 77,
    N = 78,
    O = 79,
    P = 80,
    Q = 81,
    R = 82,
    S = 83,
    T = 84,
    U = 85,
    V = 86,
    W = 87,
    X = 88,
    Y = 89,
    Z = 90,
    BracketLeft = 91,
    Backslash = 92,
    BracketRight = 93,
    GraveAccent = 96,  // `
    AsciiCircum = 94,  // °
    Underscore = 95,   // _
    BraceLeft = 123,   // { 0x7b
    Bar = 124,         // | 0x7c
    BraceRight = 125,  // } 0x7d
    AsciiTilde = 126,  // ~ 0x7e
    World1 = 161,      // non-US #1
    World2 = 162,      // non-US #2

    /* Function Keys */
    Escape = 256,
    Enter = 257,
    Tab = 258,
    Backspace = 259,
    Insert = 260,
    Delete = 261,
    Right = 262,
    Left = 263,
    Down = 264,
    Up = 265,
    PageUp = 266,
    PageDown = 267,
    Home = 268,
    End = 269,
    CapsLock = 280,
    ScrollLock = 281,
    NumLock = 282,
    PrintScreen = 283,
    Pause = 284,
    F1 = 290,
    F2 = 291,
    F3 = 292,
    F4 = 293,
    F5 = 294,
    F6 = 295,
    F7 = 296,
    F8 = 297,
    F9 = 298,
    F10 = 299,
    F11 = 300,
    F12 = 301,
    F13 = 302,
    F14 = 303,
    F15 = 304,
    F16 = 305,
    F17 = 306,
    F18 = 307,
    F19 = 308,
    F20 = 309,
    F21 = 310,
    F22 = 311,
    F23 = 312,
    F24 = 313,
    F25 = 314,
    /* keypad keys */
    KP0 = 320,
    KP1 = 321,
    KP2 = 322,
    KP3 = 323,
    KP4 = 324,
    KP5 = 325,
    KP6 = 326,
    KP7 = 327,
    KP8 = 328,
    KP9 = 329,
    KPDecimal = 330,
    KPDivide = 331,
    KPMultiply = 332,
    KPSubtract = 333,
    KPAdd = 334,
    KPEnter = 335,
    KPEqual = 336,
    /* modifier keys */
    LeftShift = 340,
    LeftControl = 341,
    LeftAlt = 342,
    LeftSuper = 343,
    RightShift = 344,
    RightControl = 345,
    RightAlt = 346,
    RightSuper = 347,
    Menu = 348,
    LeftMeta = 349,
    RightMeta = 350,
};

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss, IvwKey k) {
    switch (k) {
        case IvwKey::Undefined:
            ss << "Undefined";
            break;
        case IvwKey::Unknown:
            ss << "Unknown";
            break;
        case IvwKey::Space:
            ss << "Space";
            break;
        case IvwKey::Exclam:
            ss << "Exclam";
            break;
        case IvwKey::QuoteDbl:
            ss << "QuoteDbl";
            break;
        case IvwKey::NumberSign:
            ss << "NumberSign";
            break;
        case IvwKey::Dollar:
            ss << "Dollar";
            break;
        case IvwKey::Percent:
            ss << "Percent";
            break;
        case IvwKey::Ampersand:
            ss << "Ampersand";
            break;
        case IvwKey::Apostrophe:
            ss << "Apostrophe";
            break;
        case IvwKey::ParenLeft:
            ss << "ParenLeft";
            break;
        case IvwKey::ParenRight:
            ss << "ParenRight";
            break;
        case IvwKey::Asterisk:
            ss << "Asterisk";
            break;
        case IvwKey::Plus:
            ss << "Plus";
            break;
        case IvwKey::Comma:
            ss << "Comma";
            break;
        case IvwKey::Minus:
            ss << "Minus";
            break;
        case IvwKey::Period:
            ss << "Period";
            break;
        case IvwKey::Slash:
            ss << "Slash";
            break;
        case IvwKey::Num0:
            ss << "Num0";
            break;
        case IvwKey::Num1:
            ss << "Num1";
            break;
        case IvwKey::Num2:
            ss << "Num2";
            break;
        case IvwKey::Num3:
            ss << "Num3";
            break;
        case IvwKey::Num4:
            ss << "Num4";
            break;
        case IvwKey::Num5:
            ss << "Num5";
            break;
        case IvwKey::Num6:
            ss << "Num6";
            break;
        case IvwKey::Num7:
            ss << "Num7";
            break;
        case IvwKey::Num8:
            ss << "Num8";
            break;
        case IvwKey::Num9:
            ss << "Num9";
            break;
        case IvwKey::Colon:
            ss << "Colon";
            break;
        case IvwKey::Semicolon:
            ss << "Semicolon";
            break;
        case IvwKey::Less:
            ss << "Less";
            break;
        case IvwKey::Equal:
            ss << "Equal";
            break;
        case IvwKey::Greater:
            ss << "Greater";
            break;
        case IvwKey::Question:
            ss << "Question";
            break;
        case IvwKey::A:
            ss << "A";
            break;
        case IvwKey::B:
            ss << "B";
            break;
        case IvwKey::C:
            ss << "C";
            break;
        case IvwKey::D:
            ss << "D";
            break;
        case IvwKey::E:
            ss << "E";
            break;
        case IvwKey::F:
            ss << "F";
            break;
        case IvwKey::G:
            ss << "G";
            break;
        case IvwKey::H:
            ss << "H";
            break;
        case IvwKey::I:
            ss << "I";
            break;
        case IvwKey::J:
            ss << "J";
            break;
        case IvwKey::K:
            ss << "K";
            break;
        case IvwKey::L:
            ss << "L";
            break;
        case IvwKey::M:
            ss << "M";
            break;
        case IvwKey::N:
            ss << "N";
            break;
        case IvwKey::O:
            ss << "O";
            break;
        case IvwKey::P:
            ss << "P";
            break;
        case IvwKey::Q:
            ss << "Q";
            break;
        case IvwKey::R:
            ss << "R";
            break;
        case IvwKey::S:
            ss << "S";
            break;
        case IvwKey::T:
            ss << "T";
            break;
        case IvwKey::U:
            ss << "U";
            break;
        case IvwKey::V:
            ss << "V";
            break;
        case IvwKey::W:
            ss << "W";
            break;
        case IvwKey::X:
            ss << "X";
            break;
        case IvwKey::Y:
            ss << "Y";
            break;
        case IvwKey::Z:
            ss << "Z";
            break;
        case IvwKey::BracketLeft:
            ss << "BracketLeft";
            break;
        case IvwKey::Backslash:
            ss << "Backslash";
            break;
        case IvwKey::BracketRight:
            ss << "BracketRight";
            break;
        case IvwKey::GraveAccent:
            ss << "GraveAccent";
            break;
        case IvwKey::AsciiCircum:
            ss << "AsciiCircum";
            break;
        case IvwKey::Underscore:
            ss << "Underscore";
            break;
        case IvwKey::BraceLeft:
            ss << "BraceLeft";
            break;
        case IvwKey::Bar:
            ss << "Bar";
            break;
        case IvwKey::BraceRight:
            ss << "BraceRight";
            break;
        case IvwKey::AsciiTilde:
            ss << "AsciiTilde";
            break;
        case IvwKey::World1:
            ss << "World1";
            break;
        case IvwKey::World2:
            ss << "World2";
            break;
        case IvwKey::Escape:
            ss << "Escape";
            break;
        case IvwKey::Enter:
            ss << "Enter";
            break;
        case IvwKey::Tab:
            ss << "Tab";
            break;
        case IvwKey::Backspace:
            ss << "Backspace";
            break;
        case IvwKey::Insert:
            ss << "Insert";
            break;
        case IvwKey::Delete:
            ss << "Delete";
            break;
        case IvwKey::Right:
            ss << "Right";
            break;
        case IvwKey::Left:
            ss << "Left";
            break;
        case IvwKey::Down:
            ss << "Down";
            break;
        case IvwKey::Up:
            ss << "Up";
            break;
        case IvwKey::PageUp:
            ss << "PageUp";
            break;
        case IvwKey::PageDown:
            ss << "PageDown";
            break;
        case IvwKey::Home:
            ss << "Home";
            break;
        case IvwKey::End:
            ss << "End";
            break;
        case IvwKey::CapsLock:
            ss << "CapsLock";
            break;
        case IvwKey::ScrollLock:
            ss << "ScrollLock";
            break;
        case IvwKey::NumLock:
            ss << "NumLock";
            break;
        case IvwKey::PrintScreen:
            ss << "PrintScreen";
            break;
        case IvwKey::Pause:
            ss << "Pause";
            break;
        case IvwKey::F1:
            ss << "F1";
            break;
        case IvwKey::F2:
            ss << "F2";
            break;
        case IvwKey::F3:
            ss << "F3";
            break;
        case IvwKey::F4:
            ss << "F4";
            break;
        case IvwKey::F5:
            ss << "F5";
            break;
        case IvwKey::F6:
            ss << "F6";
            break;
        case IvwKey::F7:
            ss << "F7";
            break;
        case IvwKey::F8:
            ss << "F8";
            break;
        case IvwKey::F9:
            ss << "F9";
            break;
        case IvwKey::F10:
            ss << "F10";
            break;
        case IvwKey::F11:
            ss << "F11";
            break;
        case IvwKey::F12:
            ss << "F12";
            break;
        case IvwKey::F13:
            ss << "F13";
            break;
        case IvwKey::F14:
            ss << "F14";
            break;
        case IvwKey::F15:
            ss << "F15";
            break;
        case IvwKey::F16:
            ss << "F16";
            break;
        case IvwKey::F17:
            ss << "F17";
            break;
        case IvwKey::F18:
            ss << "F18";
            break;
        case IvwKey::F19:
            ss << "F19";
            break;
        case IvwKey::F20:
            ss << "F20";
            break;
        case IvwKey::F21:
            ss << "F21";
            break;
        case IvwKey::F22:
            ss << "F22";
            break;
        case IvwKey::F23:
            ss << "F23";
            break;
        case IvwKey::F24:
            ss << "F24";
            break;
        case IvwKey::F25:
            ss << "F25";
            break;
        case IvwKey::KP0:
            ss << "KP0";
            break;
        case IvwKey::KP1:
            ss << "KP1";
            break;
        case IvwKey::KP2:
            ss << "KP2";
            break;
        case IvwKey::KP3:
            ss << "KP3";
            break;
        case IvwKey::KP4:
            ss << "KP4";
            break;
        case IvwKey::KP5:
            ss << "KP5";
            break;
        case IvwKey::KP6:
            ss << "KP6";
            break;
        case IvwKey::KP7:
            ss << "KP7";
            break;
        case IvwKey::KP8:
            ss << "KP8";
            break;
        case IvwKey::KP9:
            ss << "KP9";
            break;
        case IvwKey::KPDecimal:
            ss << "KPDecimal";
            break;
        case IvwKey::KPDivide:
            ss << "KPDivide";
            break;
        case IvwKey::KPMultiply:
            ss << "KPMultiply";
            break;
        case IvwKey::KPSubtract:
            ss << "KPSubtract";
            break;
        case IvwKey::KPAdd:
            ss << "KPAdd";
            break;
        case IvwKey::KPEnter:
            ss << "KPEnter";
            break;
        case IvwKey::KPEqual:
            ss << "KPEqual";
            break;
        case IvwKey::LeftShift:
            ss << "LeftShift";
            break;
        case IvwKey::LeftControl:
            ss << "LeftControl";
            break;
        case IvwKey::LeftAlt:
            ss << "LeftAlt";
            break;
        case IvwKey::LeftSuper:
            ss << "LeftSuper";
            break;
        case IvwKey::RightShift:
            ss << "RightShift";
            break;
        case IvwKey::RightControl:
            ss << "RightControl";
            break;
        case IvwKey::RightAlt:
            ss << "RightAlt";
            break;
        case IvwKey::RightSuper:
            ss << "RightSuper";
            break;
        case IvwKey::Menu:
            ss << "Menu";
            break;
        case IvwKey::LeftMeta:
            ss << "LeftMeta";
            break;
        case IvwKey::RightMeta:
            ss << "RightMeta";
            break;
        default:
            break;
    }
    return ss;
}

}  // namespace inviwo

#endif  // IVW_KEYBOARDKEYS_H
