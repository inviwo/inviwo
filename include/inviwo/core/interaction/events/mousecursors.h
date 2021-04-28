/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <iterator>
#include <ostream>

namespace inviwo {

// mouse cursors based on Qt::CursorShape
enum class MouseCursor {
    Arrow,
    UpArrow,
    Cross,
    Wait,
    IBeam,
    SizeVer,
    SizeHor,
    SizeBDiag,
    SizeFDiag,
    SizeAll,
    Blank,
    SplitV,
    SplitH,
    PointingHand,
    Forbidden,
    OpenHand,
    ClosedHand,
    WhatsThis,
    Busy
};

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss, MouseCursor c) {
    switch (c) {
        case MouseCursor::Arrow:
            ss << "Arrow";
            break;
        case MouseCursor::UpArrow:
            ss << "UpArrow";
            break;
        case MouseCursor::Cross:
            ss << "Cross";
            break;
        case MouseCursor::Wait:
            ss << "Wait";
            break;
        case MouseCursor::IBeam:
            ss << "IBeam";
            break;
        case MouseCursor::SizeVer:
            ss << "SizeVer";
            break;
        case MouseCursor::SizeHor:
            ss << "SizeHor";
            break;
        case MouseCursor::SizeBDiag:
            ss << "SizeBDiag";
            break;
        case MouseCursor::SizeFDiag:
            ss << "SizeFDiag";
            break;
        case MouseCursor::SizeAll:
            ss << "SizeAll";
            break;
        case MouseCursor::Blank:
            ss << "Blank";
            break;
        case MouseCursor::SplitV:
            ss << "SplitV";
            break;
        case MouseCursor::SplitH:
            ss << "SplitH";
            break;
        case MouseCursor::PointingHand:
            ss << "PointingHand";
            break;
        case MouseCursor::Forbidden:
            ss << "Forbidden";
            break;
        case MouseCursor::OpenHand:
            ss << "OpenHand";
            break;
        case MouseCursor::ClosedHand:
            ss << "ClosedHand";
            break;
        case MouseCursor::WhatsThis:
            ss << "WhatsThis";
            break;
        case MouseCursor::Busy:
            ss << "Busy";
            break;
    }
    return ss;
}

}  // namespace inviwo
