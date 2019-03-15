/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#ifndef IVW_EDITORCONSTANTS_H
#define IVW_EDITORCONSTANTS_H

#include <modules/animationqt/animationqtmoduledefine.h>

#include <modules/animation/datastructures/animationtime.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QtGlobal>
#include <warn/pop>

namespace inviwo {

namespace animation {

constexpr int trackHeight = 31;
constexpr int trackHeightNudge = 2;
constexpr int timelineHeight = 25;
constexpr int keyframeWidth = 15;
constexpr int keyframeHeight = trackHeight;
constexpr int widthPerSecond = 96;

enum class ItemTypes { Keyframe, KeyframeSequence, ControlTrack, PropertyTrack };

/// We snap to certain times depending on the scale (zoom) level and keyboard modifiers.
/// It is important to supply scene coordinates to this function!
IVW_MODULE_ANIMATIONQT_API qreal getSnapTime(const qreal& actualTime, const qreal& scale);

constexpr double timeToScenePos(Seconds time) { return time.count() * widthPerSecond; }
constexpr Seconds scenePosToTime(double pos) { return Seconds{pos / widthPerSecond}; }

struct FindDivisionsResult {
    double start;
    double step;
    size_t count;
    int integerDigits;
    int fractionalDigits;
};

IVW_MODULE_ANIMATIONQT_API FindDivisionsResult findDivisions(double start, double stop,
                                                             int divisions);

}  // namespace animation

}  // namespace inviwo

#endif  // IVW_EDITORCONSTANTS_H
