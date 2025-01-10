/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#include <inviwo/core/interaction/events/mousebuttons.h>

#include <inviwo/core/util/ostreamjoiner.h>
#include <inviwo/core/util/exception.h>
#include <ostream>

namespace inviwo {

std::string_view enumToStr(MouseButton b) {
    switch (b) {
        case MouseButton::None:
            return "None";
        case MouseButton::Left:
            return "Left";
        case MouseButton::Middle:
            return "Middle";
        case MouseButton::Right:
            return "Right";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumToStr"), "Found invalid MouseButton enum value '{}'",
                    static_cast<int>(b));
}

std::string_view enumToStr(MouseState s) {
    switch (s) {
        case MouseState::Move:
            return "Move";
        case MouseState::Press:
            return "Press";
        case MouseState::Release:
            return "Release";
        case MouseState::DoubleClick:
            return "DoubleClick";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumToStr"), "Found invalid MouseState enum value '{}'",
                    static_cast<int>(s));
}

std::ostream& operator<<(std::ostream& ss, MouseButton b) { return ss << enumToStr(b); }
std::ostream& operator<<(std::ostream& ss, MouseState s) { return ss << enumToStr(s); }
std::ostream& operator<<(std::ostream& ss, MouseButtons bs) {
    std::copy(bs.begin(), bs.end(), util::make_ostream_joiner(ss, "+"));
    return ss;
}
std::ostream& operator<<(std::ostream& ss, MouseStates s) {
    std::copy(s.begin(), s.end(), util::make_ostream_joiner(ss, "+"));
    return ss;
}

}  // namespace inviwo
