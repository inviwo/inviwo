/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2024 Inviwo Foundation
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

#include <inviwo/core/util/chronoutils.h>
#include <sstream>
#include <iomanip>

namespace inviwo {

namespace util {

std::string msToString(double ms, bool includeZeros, bool spacing) {
    const std::string space = (spacing ? " " : "");
    std::stringstream ss;
    bool follows = false;

    size_t days = static_cast<size_t>(ms / (1000.0 * 3600.0 * 24.0));
    if (days > 0 || (follows && includeZeros)) {
        follows = true;
        ss << days << space << "d";
        ms -= 3600 * 1000 * 24 * days;
    }
    size_t hours = static_cast<size_t>(ms / (1000.0 * 3600.0));
    if (hours > 0 || (follows && includeZeros)) {
        if (follows) ss << " ";
        follows = true;
        ss << hours << space << "h";
        ms -= 3600 * 1000 * hours;
    }
    size_t minutes = static_cast<size_t>(ms / (1000.0 * 60.0));
    if (minutes > 0 || (follows && includeZeros)) {
        if (follows) ss << " ";
        follows = true;
        ss << minutes << space << "min";
        ms -= 60 * 1000 * minutes;
    }
    size_t seconds = static_cast<size_t>(ms / 1000.0);
    // combine seconds and milliseconds, iff there already something added to the string stream
    // _or_ there are more than one second
    if (seconds > 0 || (follows && includeZeros)) {
        if (follows) ss << " ";
        follows = true;
        ss << std::setprecision(4) << (ms / 1000.0) << space << "s";
    } else {
        // less than one second, no leading minutes/hours
        ss << std::setprecision(4) << ms << space << "ms";
    }

    return ss.str();
}

}  // namespace util

}  // namespace inviwo
