/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2024 Inviwo Foundation
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

#include <modules/qtwidgets/mousecursorutils.h>

#include <inviwo/core/interaction/events/mousecursors.h>  // for MouseCursor, MouseCursor::Arrow

namespace inviwo {

namespace util {

MouseCursor fromCursorShape(const Qt::CursorShape cursor) {
    switch (cursor) {
        case Qt::ArrowCursor:
            return MouseCursor::Arrow;
        case Qt::UpArrowCursor:
            return MouseCursor::UpArrow;
        case Qt::CrossCursor:
            return MouseCursor::Cross;
        case Qt::WaitCursor:
            return MouseCursor::Wait;
        case Qt::IBeamCursor:
            return MouseCursor::IBeam;
        case Qt::SizeVerCursor:
            return MouseCursor::SizeVer;
        case Qt::SizeHorCursor:
            return MouseCursor::SizeHor;
        case Qt::SizeBDiagCursor:
            return MouseCursor::SizeBDiag;
        case Qt::SizeFDiagCursor:
            return MouseCursor::SizeFDiag;
        case Qt::SizeAllCursor:
            return MouseCursor::SizeAll;
        case Qt::BlankCursor:
            return MouseCursor::Blank;
        case Qt::SplitVCursor:
            return MouseCursor::SplitV;
        case Qt::SplitHCursor:
            return MouseCursor::SplitH;
        case Qt::PointingHandCursor:
            return MouseCursor::PointingHand;
        case Qt::ForbiddenCursor:
            return MouseCursor::Forbidden;
        case Qt::OpenHandCursor:
            return MouseCursor::OpenHand;
        case Qt::ClosedHandCursor:
            return MouseCursor::ClosedHand;
        case Qt::WhatsThisCursor:
            return MouseCursor::WhatsThis;
        case Qt::BusyCursor:
            return MouseCursor::Busy;
        default:
            return MouseCursor::Arrow;
    }
}

Qt::CursorShape toCursorShape(const MouseCursor cursor) {
    switch (cursor) {
        case MouseCursor::Arrow:
            return Qt::ArrowCursor;
        case MouseCursor::UpArrow:
            return Qt::UpArrowCursor;
        case MouseCursor::Cross:
            return Qt::CrossCursor;
        case MouseCursor::Wait:
            return Qt::WaitCursor;
        case MouseCursor::IBeam:
            return Qt::IBeamCursor;
        case MouseCursor::SizeVer:
            return Qt::SizeVerCursor;
        case MouseCursor::SizeHor:
            return Qt::SizeHorCursor;
        case MouseCursor::SizeBDiag:
            return Qt::SizeBDiagCursor;
        case MouseCursor::SizeFDiag:
            return Qt::SizeFDiagCursor;
        case MouseCursor::SizeAll:
            return Qt::SizeAllCursor;
        case MouseCursor::Blank:
            return Qt::BlankCursor;
        case MouseCursor::SplitV:
            return Qt::SplitVCursor;
        case MouseCursor::SplitH:
            return Qt::SplitHCursor;
        case MouseCursor::PointingHand:
            return Qt::PointingHandCursor;
        case MouseCursor::Forbidden:
            return Qt::ForbiddenCursor;
        case MouseCursor::OpenHand:
            return Qt::OpenHandCursor;
        case MouseCursor::ClosedHand:
            return Qt::ClosedHandCursor;
        case MouseCursor::WhatsThis:
            return Qt::WhatsThisCursor;
        case MouseCursor::Busy:
            return Qt::BusyCursor;
        default:
            return Qt::ArrowCursor;
    }
}

}  // namespace util

}  // namespace inviwo
