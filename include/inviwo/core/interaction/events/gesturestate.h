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

#ifndef IVW_GESTURESTATE_H
#define IVW_GESTURESTATE_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/ostreamjoiner.h>

#include <flags/flags.h>

#include <iterator>
#include <ostream>

namespace inviwo {

enum class GestureType { Pan = 1 << 0, Pinch = 1 << 1, Swipe = 1 << 2 };

ALLOW_FLAGS_FOR_ENUM(GestureType)
using GestureTypes = flags::flags<GestureType>;

enum class GestureState {
    NoGesture = 1 << 0,
    Started = 1 << 2,
    Updated = 1 << 3,
    Finished = 1 << 4,
    Canceled = 1 << 5
};

ALLOW_FLAGS_FOR_ENUM(GestureState)
using GestureStates = flags::flags<GestureState>;

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss, GestureType t) {
    switch (t) {
        case GestureType::Pan:
            ss << "Pan";
            break;
        case GestureType::Pinch:
            ss << "Pinch";
            break;
        case GestureType::Swipe:
            ss << "Swipe";
            break;
    }
    return ss;
}

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss, GestureState s) {
    switch (s) {
        case GestureState::NoGesture:
            ss << "NoGesture";
            break;
        case GestureState::Started:
            ss << "Started";
            break;
        case GestureState::Updated:
            ss << "Updated";
            break;
        case GestureState::Finished:
            ss << "Finished";
            break;
        case GestureState::Canceled:
            ss << "Canceled";
            break;
    }
    return ss;
}

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss, GestureTypes s) {
    std::copy(s.begin(), s.end(), util::make_ostream_joiner(ss, "+"));
}

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                             GestureStates s) {
    std::copy(s.begin(), s.end(), util::make_ostream_joiner(ss, "+"));
}

}  // namespace inviwo

#endif  // IVW_GESTURESTATE_H
