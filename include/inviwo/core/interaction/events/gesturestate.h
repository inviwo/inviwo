/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2021 Inviwo Foundation
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

IVW_CORE_API std::string_view enumToStr(GestureType b);
IVW_CORE_API std::string_view enumToStr(GestureState b);

IVW_CORE_API std::ostream& operator<<(std::ostream& ss, GestureType t);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, GestureState s);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, GestureTypes s);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, GestureStates s);

}  // namespace inviwo

template <>
struct fmt::formatter<inviwo::GestureType> : inviwo::FlagFormatter<inviwo::GestureType> {};
template <>
struct fmt::formatter<inviwo::GestureTypes> : inviwo::FlagsFormatter<inviwo::GestureTypes> {};
template <>
struct fmt::formatter<inviwo::GestureState> : inviwo::FlagFormatter<inviwo::GestureState> {};
template <>
struct fmt::formatter<inviwo::GestureStates> : inviwo::FlagsFormatter<inviwo::GestureStates> {};
