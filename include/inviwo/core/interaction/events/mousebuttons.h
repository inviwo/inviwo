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

#ifndef IVW_MOUSEBUTTONS_H
#define IVW_MOUSEBUTTONS_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/ostreamjoiner.h>

#include <flags/flags.h>

#include <iterator>
#include <ostream>

namespace inviwo {

enum class MouseButton { None = 0, Left = 1 << 0, Middle = 1 << 1, Right = 1 << 2 };
ALLOW_FLAGS_FOR_ENUM(MouseButton)
using MouseButtons = flags::flags<MouseButton>;

enum class MouseState { Press = 1 << 0, Move = 1 << 1, Release = 1 << 2, DoubleClick = 1 << 3 };
ALLOW_FLAGS_FOR_ENUM(MouseState)
using MouseStates = flags::flags<MouseState>;

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss, MouseButton b) {
    switch (b) {
        case MouseButton::None:
            ss << "None";
            break;
        case MouseButton::Left:
            ss << "Left";
            break;
        case MouseButton::Middle:
            ss << "Middle";
            break;
        case MouseButton::Right:
            ss << "Right";
            break;
    }
    return ss;
}
template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss, MouseState s) {
    switch (s) {
        case MouseState::Move:
            ss << "Move";
            break;
        case MouseState::Press:
            ss << "Press";
            break;
        case MouseState::Release:
            ss << "Release";
            break;
        case MouseState::DoubleClick:
            ss << "DoubleClick";
            break;
    }
    return ss;
}

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                             MouseButtons bs) {
    std::copy(bs.begin(), bs.end(), util::make_ostream_joiner(ss, "+"));
    return ss;
}
template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss, MouseStates s) {
    std::copy(s.begin(), s.end(), util::make_ostream_joiner(ss, "+"));
    return ss;
}

}  // namespace inviwo

#endif  // IVW_MOUSEBUTTONS_H
