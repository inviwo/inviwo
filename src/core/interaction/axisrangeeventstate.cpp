/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/core/interaction/axisrangeeventstate.h>
#include <inviwo/core/util/exception.h>

namespace inviwo {

std::string_view enumToStr(AxisRangeEventState state) {
    using enum AxisRangeEventState;
    switch (state) {
        case None:
            return "None";
        case Started:
            return "Started";
        case Updated:
            return "Updated";
        case Finished:
            return "Finished";
    }
    throw Exception(SourceContext{}, "Found invalid AxisRangeEventState enum value '{}'",
                    static_cast<int>(state));
}

std::string_view enumToStr(AxisRangeInteraction interaction) {
    using enum AxisRangeInteraction;
    switch (interaction) {
        case None:
            return "None";
        case Selection:
            return "Selection";
        case Filtering:
            return "Filtering";
    }
    throw Exception(SourceContext{}, "Found invalid AxisRangeInteraction enum value '{}'",
                    static_cast<int>(interaction));
}

std::string_view enumToStr(AxisRangeInteractionMode mode) {
    using enum AxisRangeInteractionMode;
    switch (mode) {
        case None:
            return "None";
        case Replace:
            return "Replace";
        case Append:
            return "Append";
        case Clear:
            return "Clear";
    }
    throw Exception(SourceContext{}, "Found invalid AxisRangeInteractionMode enum value '{}'",
                    static_cast<int>(mode));
}

}  // namespace inviwo
