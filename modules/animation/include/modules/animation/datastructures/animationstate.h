/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_ANIMATIONSTATE_H
#define IVW_ANIMATIONSTATE_H

#include <modules/animation/animationmoduledefine.h>
#include <modules/animation/datastructures/animationtime.h>

namespace inviwo {

namespace animation {

enum class AnimationState { Paused = 0, Playing, Rendering };

enum class PlaybackMode { Once = 0, Loop, Swing };

struct AnimationTimeState {
    Seconds time;
    AnimationState state;
};

/** Keeps animation settings related to playing or rendering.
 *
 *   The settings allow to work either with numFrames or framesPerSecond.
 *   If one is set, the other is computed accordingly in order to stay consistent.
 *
 *   The parameters firstTime and lastTime do not need to coincide
 *   with the corresponding parameters of the animation.
 *   We can choose a smaller time window here,
 *   or a larger one just as well. No harm in doing the latter.
 *
 *   The smallest numFrames is 2, since we will visit at least
 *   firstTime and lastTime during an animation or rendering.
 *   The smallest framesPerSecond is 1e-3, as an arbitrary but positive, non-zero minimum.
 */
class IVW_MODULE_ANIMATION_API AnimationPlaySettings {
public:
    AnimationPlaySettings();

    Seconds getFirstTime() const;
    void setFirstTime(const Seconds timeValue);

    Seconds getLastTime() const;
    void setLastTime(const Seconds timeValue);

    /// Returns the number of frames to be rendered between firstTime and lastTime given the
    /// current @framesPerSecond.
    int getNumFrames() const;

    /** Sets the number of frames to be rendered between firstTime and lastTime, and adjusts
     *  framesPerSecond accordingly.
     *
     *   The smallest numFrames is 2, since we will visit at least
     *   firstTime and lastTime during an animation or rendering.
     *
     *   @returns true on success.
     */
    bool setNumFrames(const int desiredFrames);

    /// Returns the frames per second.
    double getFramesPerSecond() const;

    /** Sets the frames per second for the animation playback, and adjusts numFrames accordingly.
     *  The smallest @framesPerSecond is 1e-3, as an arbitrary but positive, non-zero minimum.
     *  @returns true on success.
     */
    bool setFramesPerSecond(const double desiredFPS);

    bool operator!=(const AnimationPlaySettings& other) const;

    PlaybackMode mode;

protected:
    Seconds firstTime;
    Seconds lastTime;

    /// Number of frames to generate between firstTime and lastTime
    int numFrames;

    /// Frames per second.
    double framesPerSecond;
};

}  // namespace animation

}  // namespace inviwo

#endif  // IVW_ANIMATIONSTATE_H
