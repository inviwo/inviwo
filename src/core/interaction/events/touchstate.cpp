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

#include <inviwo/core/interaction/events/touchstate.h>
#include <inviwo/core/util/ostreamjoiner.h>

#include <inviwo/core/util/exception.h>
#include <ostream>

namespace inviwo {

std::string_view enumToStr(TouchState s) {
    switch (s) {
        case TouchState::None:
            return "None";
        case TouchState::Started:
            return "Started";
        case TouchState::Updated:
            return "Updated";
        case TouchState::Stationary:
            return "Stationary";
        case TouchState::Finished:
            return "Finished";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumToStr"), "Found invalid TouchState enum value '{}'",
                    static_cast<int>(s));
}

std::ostream& operator<<(std::ostream& ss, TouchState s) { return ss << enumToStr(s); }

std::ostream& operator<<(std::ostream& ss, TouchStates s) {
    std::copy(s.begin(), s.end(), util::make_ostream_joiner(ss, "+"));
    return ss;
}

}  // namespace inviwo
