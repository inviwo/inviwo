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

#ifndef IVW_PICKINGSTATE_H
#define IVW_PICKINGSTATE_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/ostreamjoiner.h>

#include <flags/flags.h>

#include <iterator>
#include <ostream>

namespace inviwo {

enum class PickingState {
    None = 0,
    Started = 1 << 0,   // Hover enter / press
    Updated = 1 << 1,   // Moved
    Finished = 1 << 2,  // Hover leave / release
};

ALLOW_FLAGS_FOR_ENUM(PickingState)
using PickingStates = flags::flags<PickingState>;

// a button / touch / gesture
enum class PickingPressItem { None = 0, Primary = 1 << 0, Secondary = 1 << 1, Tertiary = 1 << 2 };

ALLOW_FLAGS_FOR_ENUM(PickingPressItem)
using PickingPressItems = flags::flags<PickingPressItem>;

enum class PickingPressState {
    None = 0,
    Press = 1 << 0,    // button / touch / gesture started
    Move = 1 << 1,     // button / touch / gesture updated
    Release = 1 << 2,  // button / touch / gesture ended
    DoubleClick = 1 << 3
};

ALLOW_FLAGS_FOR_ENUM(PickingPressState)
using PickingPressStates = flags::flags<PickingPressState>;

enum class PickingHoverState { None = 0, Enter = 1 << 0, Move = 1 << 1, Exit = 1 << 2 };

ALLOW_FLAGS_FOR_ENUM(PickingHoverState)
using PickingHoverStates = flags::flags<PickingHoverState>;

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss, PickingState s) {
    switch (s) {
        case PickingState::None:
            ss << "None";
            break;
        case PickingState::Started:
            ss << "Started";
            break;
        case PickingState::Updated:
            ss << "Updated";
            break;
        case PickingState::Finished:
            ss << "Finished";
            break;
    }
    return ss;
}
template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                             PickingStates s) {
    std::copy(s.begin(), s.end(), util::make_ostream_joiner(ss, "+"));
    return ss;
}

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                             PickingPressItem s) {
    switch (s) {
        case PickingPressItem::None:
            ss << "None";
            break;
        case PickingPressItem::Primary:
            ss << "Primary";
            break;
        case PickingPressItem::Secondary:
            ss << "Secondary";
            break;
        case PickingPressItem::Tertiary:
            ss << "Tertiary";
            break;
    }
    return ss;
}
template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                             PickingPressItems s) {
    std::copy(s.begin(), s.end(), util::make_ostream_joiner(ss, "+"));
    return ss;
}

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                             PickingPressState s) {
    switch (s) {
        case PickingPressState::None:
            ss << "None";
            break;
        case PickingPressState::Press:
            ss << "Press";
            break;
        case PickingPressState::Move:
            ss << "Move";
            break;
        case PickingPressState::Release:
            ss << "Release";
            break;
        case PickingPressState::DoubleClick:
            ss << "DoubleClick";
            break;
    }
    return ss;
}
template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                             PickingPressStates s) {
    std::copy(s.begin(), s.end(), util::make_ostream_joiner(ss, "+"));
    return ss;
}

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                             PickingHoverState s) {
    switch (s) {
        case PickingHoverState::None:
            ss << "None";
            break;
        case PickingHoverState::Enter:
            ss << "Enter";
            break;
        case PickingHoverState::Move:
            ss << "Move";
            break;
        case PickingHoverState::Exit:
            ss << "Exit";
            break;
    }
    return ss;
}
template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                             PickingHoverStates s) {
    std::copy(s.begin(), s.end(), util::make_ostream_joiner(ss, "+"));
    return ss;
}

}  // namespace inviwo

#endif  // IVW_PICKINGSTATE_H
