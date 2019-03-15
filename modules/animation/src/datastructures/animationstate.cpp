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

#include <modules/animation/datastructures/animationstate.h>

#include <algorithm>
#include <cmath>

namespace inviwo {

namespace animation {

AnimationPlaySettings::AnimationPlaySettings()
    : mode(PlaybackMode::Once), firstTime(0), lastTime(0), numFrames(2), framesPerSecond(25) {}

Seconds AnimationPlaySettings::getFirstTime() const { return firstTime; }

void AnimationPlaySettings::setFirstTime(const Seconds timeValue) {
    firstTime = timeValue;
    if (firstTime > lastTime) std::swap(firstTime, lastTime);
    setNumFrames(numFrames);
}

Seconds AnimationPlaySettings::getLastTime() const { return lastTime; }

void AnimationPlaySettings::setLastTime(const Seconds timeValue) {
    lastTime = timeValue;
    if (firstTime > lastTime) std::swap(firstTime, lastTime);
    setNumFrames(numFrames);
}

int AnimationPlaySettings::getNumFrames() const { return numFrames; }

bool AnimationPlaySettings::setNumFrames(const int desiredFrames) {
    if (desiredFrames < 2) return false;
    numFrames = desiredFrames;

    // Adjust fps
    const Seconds timeWindow(lastTime - firstTime);
    framesPerSecond = timeWindow.count() / double(numFrames);

    return true;
}

double AnimationPlaySettings::getFramesPerSecond() const { return framesPerSecond; }

bool AnimationPlaySettings::setFramesPerSecond(const double desiredFPS) {
    if (desiredFPS < 1e-3) return false;
    framesPerSecond = desiredFPS;

    // Adjust numFrames
    const Seconds timeWindow(lastTime - firstTime);
    numFrames = static_cast<int>(std::ceil(timeWindow.count() * framesPerSecond));
    if (numFrames < 2) numFrames = 2;  // Only happens for small time windows.

    return true;
}

bool AnimationPlaySettings::operator!=(const AnimationPlaySettings& other) const {
    return (mode != other.mode || firstTime != other.firstTime || lastTime != other.lastTime ||
            numFrames != other.numFrames || framesPerSecond != other.framesPerSecond);
}

}  // namespace animation

}  // namespace inviwo
