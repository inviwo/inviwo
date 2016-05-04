/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

namespace inviwo {

enum class IvwKey {
    // key codes based on glfw
    Unknown          = -1,
    
    /* Printable Keys */
    Space            = 32,
    Exclam           = 33,  /* ! */
    QuoteDbl         = 34,  /* " */
    NumberSign       = 35,
    Dollar           = 36,  /* $ */
    Percent          = 37,  /* % */
    Ampersand        = 38,  /* @ */
    Apostrophe       = 39,  /* ' */
    ParenLeft        = 40,  /* ( */
    ParenRight       = 41,  /* ) */
    Asterisk         = 42,  /* * */
    Plus             = 43,  /* + */
    Comma            = 44,  /* , */
    Minus            = 45,  /* - */
    Period           = 46,  /* . */
    Slash            = 47,  /* / */
    Num0             = 48,
    Num1             = 49,
    Num2             = 50,
    Num3             = 51,
    Num4             = 52,
    Num5             = 53,
    Num6             = 54,
    Num7             = 55,
    Num8             = 56,
    Num9             = 57,
    Colon            = 58,  /* : */
    Semicolon        = 59,  /* ; */
    Less             = 60,  /* < */
    Equal            = 61,  /* = */
    Greater          = 62,  /* > */
    Question         = 63,  /* ? */
    A                = 65,
    B                = 66,
    C                = 67,
    D                = 68,
    E                = 69,
    F                = 70,
    G                = 71,
    H                = 72,
    I                = 73,
    J                = 74,
    K                = 75,
    L                = 76,
    M                = 77,
    N                = 78,
    O                = 79,
    P                = 80,
    Q                = 81,
    R                = 82,
    S                = 83,
    T                = 84,
    U                = 85,
    V                = 86,
    W                = 87,
    X                = 88,
    Y                = 89,
    Z                = 90,
    BracketLeft      = 91,  /* [ */
    Backslash        = 92,  /* \ */
    BracketRight     = 93,  /* ] */
    GraveAccent      = 96,  /* ` */
    AsciiCircum      = 94,  /* ° */
    Underscore       = 95,  /* _ */
    QuoteLeft        = 96,
    BraceLeft        = 123, /* { */ // 0x7b
    Bar              = 124, /* | */ // 0x7c
    BraceRight       = 125, /* } */ // 0x7d
    AsciiTilde       = 126, /* ~ */ // 0x7e
    World1           = 161, /* non-US #1 */
    World2           = 162, /* non-US #2 */
    
    /* Function Keys */
    Escape           = 256,
    Enter            = 257,
    Tab              = 258,
    Backspace        = 259,
    Insert           = 260,
    Delete           = 261,
    Right            = 262,
    Left             = 263,
    Down             = 264,
    Up               = 265,
    PageUp           = 266,
    PageDown         = 267,
    Home             = 268,
    End              = 269,
    CapsLock         = 280,
    ScrollLock       = 281,
    NumLock          = 282,
    PrintScreen      = 283,
    Pause            = 284,
    F1               = 290,
    F2               = 291,
    F3               = 292,
    F4               = 293,
    F5               = 294,
    F6               = 295,
    F7               = 296,
    F8               = 297,
    F9               = 298,
    F10              = 299,
    F11              = 300,
    F12              = 301,
    F13              = 302,
    F14              = 303,
    F15              = 304,
    F16              = 305,
    F17              = 306,
    F18              = 307,
    F19              = 308,
    F20              = 309,
    F21              = 310,
    F22              = 311,
    F23              = 312,
    F24              = 313,
    F25              = 314,
    /* keypad keys */
    KP0              = 320,
    KP1              = 321,
    KP2              = 322,
    KP3              = 323,
    KP4              = 324,
    KP5              = 325,
    KP6              = 326,
    KP7              = 327,
    KP8              = 328,
    KP9              = 329,
    KPDecimal        = 330,
    KPDivide         = 331,
    KPMultiply       = 332,
    KPSubtract       = 333,
    KPAdd            = 334,
    KPEnter          = 335,
    KPEqual          = 336,
    /* modifier keys */
    LeftShift        = 340,
    LeftControl      = 341,
    LeftAlt          = 342,
    LeftSuper        = 343,
    RightShift       = 344,
    RightControl     = 345,
    RightAlt         = 346,
    RightSuper       = 347,
    Menu             = 348,
    LeftMeta         = 349,
    RightMeta        = 350,
};

}  // namespace inviwo

#endif  // IVW_KEYBOARDKEYS_H
