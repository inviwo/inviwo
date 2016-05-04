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

#include <inviwo/qt/widgets/keyboardutils.h>

#ifdef _WIN32
 // needed for virtual key codes on Windows
#include <windows.h>
#include <winuser.h>
#endif

 // for Qt key codes
#include <QtCore/qnamespace.h>
#include <QKeyEvent>

namespace inviwo {

namespace util {

IvwKey mapKeyFromQt(const QKeyEvent *keyevent) {

#ifdef _WIN32
    // handle special keys here first, examples include numpad keys and left/right modifier keys.
    // Those cannot be distinguished from the regular key code of the event
    //
    // list of virtual key codes in Windows
    //     https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
    switch (keyevent->nativeVirtualKey()) {
        /* keypad keys */
    case VK_NUMPAD0:
        return IvwKey::KP0;
    case VK_NUMPAD1:
        return IvwKey::KP1;
    case VK_NUMPAD2:
        return IvwKey::KP2;
    case VK_NUMPAD3:
        return IvwKey::KP3;
    case VK_NUMPAD4:
        return IvwKey::KP4;
    case VK_NUMPAD5:
        return IvwKey::KP5;
    case VK_NUMPAD6:
        return IvwKey::KP6;
    case VK_NUMPAD7:
        return IvwKey::KP7;
    case VK_NUMPAD8:
        return IvwKey::KP8;
    case VK_NUMPAD9:
        return IvwKey::KP9;
    case VK_DECIMAL:
        return IvwKey::KPDecimal;
    case VK_DIVIDE:
        return IvwKey::KPDivide;
    case VK_MULTIPLY:
        return IvwKey::KPMultiply;
    case VK_SUBTRACT:
        return IvwKey::KPSubtract;
    case VK_ADD:
        return IvwKey::KPAdd;
    /* modifier keys */
    case VK_SHIFT:
        if (GetKeyState(VK_LSHIFT) & 0x8000) {
            return IvwKey::LeftShift;
        }
        else if (GetKeyState(VK_RSHIFT) & 0x8000) {
            return IvwKey::RightShift;
        }
        // cannot distinguish between left and right, 
        // use regular case further down as fallback option
        break;
    case VK_CONTROL:
        if (GetKeyState(VK_LCONTROL) & 0x8000) {
            return IvwKey::LeftControl;
        }
        else if (GetKeyState(VK_RCONTROL) & 0x8000) {
            return IvwKey::RightControl;
        }
        // cannot distinguish between left and right, 
        // use regular case further down as fallback option
        break;
    case VK_MENU:
        if (GetKeyState(VK_LMENU) & 0x8000) {
            return IvwKey::LeftAlt;
        }
        else if (GetKeyState(VK_RMENU) & 0x8000) {
            return IvwKey::RightAlt;
        }
        // cannot distinguish between left and right, 
        // use regular case further down as fallback option
        break;
    default:
        // do nothing, the virtual key might be 0 although key() contains information
        break;
    }
#endif // _WIN32

    switch (keyevent->key()) {
        /* Printable Keys */
    case Qt::Key_Space:
        return IvwKey::Space;
    case Qt::Key_Exclam:
        return IvwKey::Exclam;
    case Qt::Key_QuoteDbl:
        return IvwKey::QuoteDbl;
    case Qt::Key_NumberSign:
        return IvwKey::NumberSign;
    case Qt::Key_Dollar:
        return IvwKey::Dollar;
    case Qt::Key_Percent:
        return IvwKey::Percent;
    case Qt::Key_Ampersand:
        return IvwKey::Ampersand;
    case Qt::Key_Apostrophe:
        return IvwKey::Apostrophe;
    case Qt::Key_ParenLeft:
        return IvwKey::ParenLeft;
    case Qt::Key_ParenRight:
        return IvwKey::ParenRight;
    case Qt::Key_Asterisk:
        return IvwKey::Asterisk;
    case Qt::Key_Plus:
        return IvwKey::Plus;
    case Qt::Key_Comma:
        return IvwKey::Comma;
    case Qt::Key_Minus:
        return IvwKey::Minus;
    case Qt::Key_Period:
        return IvwKey::Period;
    case Qt::Key_Slash:
        return IvwKey::Slash;
    case Qt::Key_0:
        return IvwKey::Num0;
    case Qt::Key_1:
        return IvwKey::Num1;
    case Qt::Key_2:
        return IvwKey::Num2;
    case Qt::Key_3:
        return IvwKey::Num3;
    case Qt::Key_4:
        return IvwKey::Num4;
    case Qt::Key_5:
        return IvwKey::Num5;
    case Qt::Key_6:
        return IvwKey::Num6;
    case Qt::Key_7:
        return IvwKey::Num7;
    case Qt::Key_8:
        return IvwKey::Num8;
    case Qt::Key_9:
        return IvwKey::Num9;
    case Qt::Key_Colon:
        return IvwKey::Colon;
    case Qt::Key_Semicolon:
        return IvwKey::Semicolon;
    case Qt::Key_Less:
        return IvwKey::Less;
    case Qt::Key_Equal:
        return IvwKey::Equal;
    case Qt::Key_Greater:
        return IvwKey::Greater;
    case Qt::Key_Question:
        return IvwKey::Question;
    case Qt::Key_A:
        return IvwKey::A;
    case Qt::Key_B:
        return IvwKey::B;
    case Qt::Key_C:
        return IvwKey::C;
    case Qt::Key_D:
        return IvwKey::D;
    case Qt::Key_E:
        return IvwKey::E;
    case Qt::Key_F:
        return IvwKey::F;
    case Qt::Key_G:
        return IvwKey::G;
    case Qt::Key_H:
        return IvwKey::H;
    case Qt::Key_I:
        return IvwKey::I;
    case Qt::Key_J:
        return IvwKey::J;
    case Qt::Key_K:
        return IvwKey::K;
    case Qt::Key_L:
        return IvwKey::L;
    case Qt::Key_M:
        return IvwKey::M;
    case Qt::Key_N:
        return IvwKey::N;
    case Qt::Key_O:
        return IvwKey::O;
    case Qt::Key_P:
        return IvwKey::P;
    case Qt::Key_Q:
        return IvwKey::Q;
    case Qt::Key_R:
        return IvwKey::R;
    case Qt::Key_S:
        return IvwKey::S;
    case Qt::Key_T:
        return IvwKey::T;
    case Qt::Key_U:
        return IvwKey::U;
    case Qt::Key_V:
        return IvwKey::V;
    case Qt::Key_W:
        return IvwKey::W;
    case Qt::Key_X:
        return IvwKey::X;
    case Qt::Key_Y:
        return IvwKey::Y;
    case Qt::Key_Z:
        return IvwKey::Z;
    case Qt::Key_BracketLeft:
        return IvwKey::BracketLeft;
    case Qt::Key_Backslash:
        return IvwKey::Backslash;
    case Qt::Key_BracketRight:
        return IvwKey::BracketRight;
    case Qt::Key_Dead_Grave:
        return IvwKey::GraveAccent;
    case Qt::Key_AsciiCircum:
        return IvwKey::AsciiCircum;
    case Qt::Key_Underscore:
        return IvwKey::Underscore;
    case Qt::Key_QuoteLeft:
        return IvwKey::QuoteLeft;
    case Qt::Key_BraceLeft:
        return IvwKey::BraceLeft;
    case Qt::Key_Bar:
        return IvwKey::Bar;
    case Qt::Key_BraceRight:
        return IvwKey::BraceRight;
    case Qt::Key_AsciiTilde:
        return IvwKey::AsciiTilde;

        /* Function Keys */
    case Qt::Key_Escape:
        return IvwKey::Escape;
    case Qt::Key_Enter:
        return IvwKey::Enter;
    case Qt::Key_Tab:
        return IvwKey::Tab;
    case Qt::Key_Backspace:
        return IvwKey::Backspace;
    case Qt::Key_Insert:
        return IvwKey::Insert;
    case Qt::Key_Delete:
        return IvwKey::Delete;
    case Qt::Key_Right:
        return IvwKey::Right;
    case Qt::Key_Left:
        return IvwKey::Left;
    case Qt::Key_Down:
        return IvwKey::Down;
    case Qt::Key_Up:
        return IvwKey::Up;
    case Qt::Key_PageUp:
        return IvwKey::PageUp;
    case Qt::Key_PageDown:
        return IvwKey::PageDown;
    case Qt::Key_Home:
        return IvwKey::Home;
    case Qt::Key_End:
        return IvwKey::End;
    case Qt::Key_CapsLock:
        return IvwKey::CapsLock;
    case Qt::Key_ScrollLock:
        return IvwKey::ScrollLock;
    case Qt::Key_NumLock:
        return IvwKey::NumLock;
    case Qt::Key_Print:
        return IvwKey::PrintScreen;
    case Qt::Key_Pause:
        return IvwKey::Pause;
    case Qt::Key_F1:
        return IvwKey::F1;
    case Qt::Key_F2:
        return IvwKey::F2;
    case Qt::Key_F3:
        return IvwKey::F3;
    case Qt::Key_F4:
        return IvwKey::F4;
    case Qt::Key_F5:
        return IvwKey::F5;
    case Qt::Key_F6:
        return IvwKey::F6;
    case Qt::Key_F7:
        return IvwKey::F7;
    case Qt::Key_F8:
        return IvwKey::F8;
    case Qt::Key_F9:
        return IvwKey::F9;
    case Qt::Key_F10:
        return IvwKey::F10;
    case Qt::Key_F11:
        return IvwKey::F11;
    case Qt::Key_F12:
        return IvwKey::F12;
    case Qt::Key_F13:
        return IvwKey::F13;
    case Qt::Key_F14:
        return IvwKey::F14;
    case Qt::Key_F15:
        return IvwKey::F15;
    case Qt::Key_F16:
        return IvwKey::F16;
    case Qt::Key_F17:
        return IvwKey::F17;
    case Qt::Key_F18:
        return IvwKey::F18;
    case Qt::Key_F19:
        return IvwKey::F19;
    case Qt::Key_F20:
        return IvwKey::F20;
    case Qt::Key_F21:
        return IvwKey::F21;
    case Qt::Key_F22:
        return IvwKey::F22;
    case Qt::Key_F23:
        return IvwKey::F23;
    case Qt::Key_F24:
        return IvwKey::F24;
    case Qt::Key_F25:
        return IvwKey::F25;
    /* modifier keys */
    // TODO: Shift, Control, and Alt keys are ambiguous (see issue #1216)
    //       returning the corresponding left key for now!
    case Qt::Key_Shift:
        return IvwKey::LeftShift;
    case Qt::Key_Control:
        return IvwKey::LeftControl;
    case Qt::Key_Meta:
        return IvwKey::LeftMeta;
    case Qt::Key_Alt:
        return IvwKey::LeftAlt;
    case Qt::Key_Super_L:
        return IvwKey::LeftSuper;
    case Qt::Key_Super_R:
        return IvwKey::RightSuper;
    case Qt::Key_Menu:
        return IvwKey::Menu;

    default:
        return IvwKey::Unknown;
    }
}

} // namespace util

} //namespace inviwo
