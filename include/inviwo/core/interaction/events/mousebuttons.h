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

#include <string_view>
#include <iosfwd>
#include <array>

#include <flags/flags.h>
#include <fmt/format.h>

namespace inviwo {

enum class MouseButton { None = 0, Left = 1 << 0, Middle = 1 << 1, Right = 1 << 2 };
ALLOW_FLAGS_FOR_ENUM(MouseButton)
using MouseButtons = flags::flags<MouseButton>;

enum class MouseState { Press = 1 << 0, Move = 1 << 1, Release = 1 << 2, DoubleClick = 1 << 3 };
ALLOW_FLAGS_FOR_ENUM(MouseState)
using MouseStates = flags::flags<MouseState>;

IVW_CORE_API std::ostream& operator<<(std::ostream& ss, MouseButton b);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, MouseState s);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, MouseButtons bs);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, MouseStates s);

namespace util {
IVW_CORE_API std::string_view name(MouseButton b);
IVW_CORE_API std::string_view name(MouseState b);
}  // namespace util

}  // namespace inviwo

template <>
struct fmt::formatter<inviwo::MouseButton> : inviwo::FlagFormatter<inviwo::MouseButton> {};
template <>
struct fmt::formatter<inviwo::MouseButtons> : inviwo::FlagsFormatter<inviwo::MouseButtons> {};
template <>
struct fmt::formatter<inviwo::MouseState> : inviwo::FlagFormatter<inviwo::MouseState> {};
template <>
struct fmt::formatter<inviwo::MouseStates> : inviwo::FlagsFormatter<inviwo::MouseStates> {};
