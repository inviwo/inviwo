/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/core/interaction/events/gesturestate.h>
#include <inviwo/core/util/ostreamjoiner.h>
#include <inviwo/core/util/exception.h>

#include <iterator>
#include <ostream>

namespace inviwo {

std::string_view enumToStr(GestureType t) {
    switch (t) {
        case GestureType::Pan:
            return "Pan";
        case GestureType::Pinch:
            return "Pinch";
        case GestureType::Swipe:
            return "Swipe";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumName"), "Found invalid GestureType enum value '{}'", static_cast<int>(t));
}
std::string_view enumToStr(GestureState s) {
    switch (s) {
        case GestureState::NoGesture:
            return "NoGesture";
        case GestureState::Started:
            return "Started";
        case GestureState::Updated:
            return "Updated";
        case GestureState::Finished:
            return "Finished";
        case GestureState::Canceled:
            return "Canceled";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumName"), "Found invalid GestureState enum value '{}'",
                    static_cast<int>(s));
}

std::ostream& operator<<(std::ostream& ss, GestureType t) { return ss << enumToStr(t); }
std::ostream& operator<<(std::ostream& ss, GestureState s) { return ss << enumToStr(s); }
std::ostream& operator<<(std::ostream& ss, GestureTypes s) {
    std::copy(s.begin(), s.end(), util::make_ostream_joiner(ss, "+"));
    return ss;
}
std::ostream& operator<<(std::ostream& ss, GestureStates s) {
    std::copy(s.begin(), s.end(), util::make_ostream_joiner(ss, "+"));
    return ss;
}

}  // namespace inviwo
