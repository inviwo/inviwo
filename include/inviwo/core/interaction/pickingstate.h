/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2026 Inviwo Foundation
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

#include <cstdint>
#include <flags/flags.h>

namespace inviwo {

enum class PickingState : std::uint8_t {
    None = 0,
    Started = 1U << 0U,   // Hover enter / press
    Updated = 1U << 1U,   // Moved
    Finished = 1U << 2U,  // Hover leave / release
};

ALLOW_FLAGS_FOR_ENUM(PickingState)
using PickingStates = flags::flags<PickingState>;

// a button / touch / gesture
enum class PickingPressItem : std::uint8_t {
    None = 0,
    Primary = 1U << 0U,
    Secondary = 1U << 1U,
    Tertiary = 1U << 2U
};

ALLOW_FLAGS_FOR_ENUM(PickingPressItem)
using PickingPressItems = flags::flags<PickingPressItem>;

enum class PickingPressState : std::uint8_t {
    None = 0,
    Press = 1U << 0U,    // button / touch / gesture started
    Move = 1U << 1U,     // button / touch / gesture updated
    Release = 1U << 2U,  // button / touch / gesture ended
    DoubleClick = 1U << 3U
};

ALLOW_FLAGS_FOR_ENUM(PickingPressState)
using PickingPressStates = flags::flags<PickingPressState>;

enum class PickingHoverState : std::uint8_t {
    None = 0,
    Enter = 1U << 0U,
    Move = 1U << 1U,
    Exit = 1U << 2U
};

ALLOW_FLAGS_FOR_ENUM(PickingHoverState)
using PickingHoverStates = flags::flags<PickingHoverState>;

IVW_CORE_API std::string_view enumToStr(PickingState b);
IVW_CORE_API std::string_view enumToStr(PickingPressItem b);
IVW_CORE_API std::string_view enumToStr(PickingPressState b);
IVW_CORE_API std::string_view enumToStr(PickingHoverState b);

}  // namespace inviwo

#ifndef DOXYGEN_SHOULD_SKIP_THIS
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
#endif
