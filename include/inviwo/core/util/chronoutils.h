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
#pragma once

#include <inviwo/core/common/inviwocoredefine.h>

#include <string>
#include <chrono>

namespace inviwo {

namespace util {

/**
 * \brief convert the given duration from milliseconds to a string.
 * The returned string will have the format "%dd %dh %dmin %dsec %.3fms", where days, hours,
 * minutes, seconds, ... are suppressed up to the first non-zero unit if not needed. Milliseconds
 * and seconds are combined if larger than 1 second.
 *
 * @param ms  in milliseconds
 * @param includeZeros   if true, time units for zero values are always shown, e.g.
 *                       "2d 0h 0min 23s" vs. "2d 23s" and "2h 0min 0s" vs. "2h"
 * @param spacing   if true, a space is inserted between digits and units
 * @return duration formatted as string
 */
IVW_CORE_API std::string msToString(double ms, bool includeZeros = true, bool spacing = false);

/**
 * \brief convenience function for converting a std::chrono::duration to a string calling
 * msToString(double).
 *
 * @param duration       duration
 * @param includeZeros   if true, time units for zero values are always shown, e.g.
 *                       "2d 0h 0min 23s" vs. "2d 23s" and "2h 0min 0s" vs. "2h"
 * @param spacing   if true, a space is inserted between digits and units
 * @return duration formatted as string
 */
template <class Rep, class Period = std::ratio<1>>
std::string durationToString(std::chrono::duration<Rep, Period> duration, bool includeZeros = true,
                             bool spacing = false) {
    using milliseconds = std::chrono::duration<double, std::milli>;
    auto ms = std::chrono::duration_cast<milliseconds>(duration);
    return msToString(ms.count(), includeZeros, spacing);
}

}  // namespace util

}  // namespace inviwo
