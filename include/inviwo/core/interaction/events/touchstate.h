/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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
#include <string_view>
#include <iosfwd>

namespace inviwo {

enum class TouchState {
    None = 0,
    Started = 1 << 0,     //!< Touches the surface of the TouchDevice
    Updated = 1 << 1,     //!< Moved
    Stationary = 1 << 2,  //!< No movement on TouchScreen, Pressed on TouchPad
    Finished = 1 << 3,    //!< Released
};

ALLOW_FLAGS_FOR_ENUM(TouchState)
using TouchStates = flags::flags<TouchState>;

IVW_CORE_API std::string_view enumToStr(TouchState b);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, TouchState s);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, TouchStates s);

}  // namespace inviwo

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template <>
struct fmt::formatter<inviwo::TouchState> : inviwo::FlagFormatter<inviwo::TouchState> {};
template <>
struct fmt::formatter<inviwo::TouchStates> : inviwo::FlagsFormatter<inviwo::TouchStates> {};
#endif
