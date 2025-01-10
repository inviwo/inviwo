/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2025 Inviwo Foundation
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

#include <inviwo/core/interaction/pickingstate.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/interaction/pickingaction.h>
#include <inviwo/core/util/assertion.h>

#include <inviwo/core/util/ostreamjoiner.h>
#include <inviwo/core/util/exception.h>

namespace inviwo {

std::string_view enumToStr(PickingState s) {
    switch (s) {
        case PickingState::None:
            return "None";
        case PickingState::Started:
            return "Started";
        case PickingState::Updated:
            return "Updated";
        case PickingState::Finished:
            return "Finished";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumToStr"), "Found invalid PickingState enum value '{}'",
                    static_cast<int>(s));
}
std::string_view enumToStr(PickingPressItem s) {
    switch (s) {
        case PickingPressItem::None:
            return "None";
        case PickingPressItem::Primary:
            return "Primary";
        case PickingPressItem::Secondary:
            return "Secondary";
        case PickingPressItem::Tertiary:
            return "Tertiary";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumToStr"),
                    "Found invalid PickingPressItem enum value '{}'", static_cast<int>(s));
}
std::string_view enumToStr(PickingPressState s) {
    switch (s) {
        case PickingPressState::None:
            return "None";
        case PickingPressState::Press:
            return "Press";
        case PickingPressState::Move:
            return "Move";
        case PickingPressState::Release:
            return "Release";
        case PickingPressState::DoubleClick:
            return "DoubleClick";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumToStr"),
                    "Found invalid PickingPressState enum value '{}'", static_cast<int>(s));
}
std::string_view enumToStr(PickingHoverState s) {
    switch (s) {
        case PickingHoverState::None:
            return "None";
        case PickingHoverState::Enter:
            return "Enter";
        case PickingHoverState::Move:
            return "Move";
        case PickingHoverState::Exit:
            return "Exit";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumToStr"),
                    "Found invalid PickingHoverState enum value '{}'", static_cast<int>(s));
}

std::ostream& operator<<(std::ostream& ss, PickingState s) { return ss << enumToStr(s); }
std::ostream& operator<<(std::ostream& ss, PickingStates s) {
    std::copy(s.begin(), s.end(), util::make_ostream_joiner(ss, "+"));
    return ss;
}

std::ostream& operator<<(std::ostream& ss, PickingPressItem s) { return ss << enumToStr(s); }
std::ostream& operator<<(std::ostream& ss, PickingPressItems s) {
    std::copy(s.begin(), s.end(), util::make_ostream_joiner(ss, "+"));
    return ss;
}

std::ostream& operator<<(std::ostream& ss, PickingPressState s) { return ss << enumToStr(s); }
std::ostream& operator<<(std::ostream& ss, PickingPressStates s) {
    std::copy(s.begin(), s.end(), util::make_ostream_joiner(ss, "+"));
    return ss;
}

std::ostream& operator<<(std::ostream& ss, PickingHoverState s) { return ss << enumToStr(s); }
std::ostream& operator<<(std::ostream& ss, PickingHoverStates s) {
    std::copy(s.begin(), s.end(), util::make_ostream_joiner(ss, "+"));
    return ss;
}

}  // namespace inviwo
