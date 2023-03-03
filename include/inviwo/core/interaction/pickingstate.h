/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2023 Inviwo Foundation
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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/fmtutils.h>

#include <flags/flags.h>

#include <iosfwd>

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

IVW_CORE_API std::string_view enumToStr(PickingState b);
IVW_CORE_API std::string_view enumToStr(PickingPressItem b);
IVW_CORE_API std::string_view enumToStr(PickingPressState b);
IVW_CORE_API std::string_view enumToStr(PickingHoverState b);

IVW_CORE_API std::ostream& operator<<(std::ostream& ss, PickingState s);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, PickingStates s);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, PickingPressItem s);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, PickingPressItems s);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, PickingPressState s);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, PickingPressStates s);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, PickingHoverState s);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, PickingHoverStates s);

}  // namespace inviwo

template <>
struct fmt::formatter<inviwo::PickingState> : inviwo::FlagFormatter<inviwo::PickingState> {};
template <>
struct fmt::formatter<inviwo::PickingStates> : inviwo::FlagsFormatter<inviwo::PickingStates> {};

template <>
struct fmt::formatter<inviwo::PickingPressItem> : inviwo::FlagFormatter<inviwo::PickingPressItem> {
};
template <>
struct fmt::formatter<inviwo::PickingPressItems>
    : inviwo::FlagsFormatter<inviwo::PickingPressItems> {};

template <>
struct fmt::formatter<inviwo::PickingPressState>
    : inviwo::FlagFormatter<inviwo::PickingPressState> {};
template <>
struct fmt::formatter<inviwo::PickingPressStates>
    : inviwo::FlagsFormatter<inviwo::PickingPressStates> {};

template <>
struct fmt::formatter<inviwo::PickingHoverState>
    : inviwo::FlagFormatter<inviwo::PickingHoverState> {};
template <>
struct fmt::formatter<inviwo::PickingHoverStates>
    : inviwo::FlagsFormatter<inviwo::PickingHoverStates> {};
