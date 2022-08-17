/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2021 Inviwo Foundation
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

#include <inviwo/core/interaction/events/mousecursors.h>

#include <inviwo/core/util/exception.h>
#include <ostream>

namespace inviwo {

std::string_view enumToStr(MouseCursor b) {
    switch (b) {
        case MouseCursor::Arrow:
            return "Arrow";
        case MouseCursor::UpArrow:
            return "UpArrow";
        case MouseCursor::Cross:
            return "Cross";
        case MouseCursor::Wait:
            return "Wait";
        case MouseCursor::IBeam:
            return "IBeam";
        case MouseCursor::SizeVer:
            return "SizeVer";
        case MouseCursor::SizeHor:
            return "SizeHor";
        case MouseCursor::SizeBDiag:
            return "SizeBDiag";
        case MouseCursor::SizeFDiag:
            return "SizeFDiag";
        case MouseCursor::SizeAll:
            return "SizeAll";
        case MouseCursor::Blank:
            return "Blank";
        case MouseCursor::SplitV:
            return "SplitV";
        case MouseCursor::SplitH:
            return "SplitH";
        case MouseCursor::PointingHand:
            return "PointingHand";
        case MouseCursor::Forbidden:
            return "Forbidden";
        case MouseCursor::OpenHand:
            return "OpenHand";
        case MouseCursor::ClosedHand:
            return "ClosedHand";
        case MouseCursor::WhatsThis:
            return "WhatsThis";
        case MouseCursor::Busy:
            return "Busy";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumName"), "Found invalid MouseCursor enum value '{}'",
                    static_cast<int>(b));
}

std::ostream& operator<<(std::ostream& ss, MouseCursor c) { return ss << enumToStr(c); }

}  // namespace inviwo
