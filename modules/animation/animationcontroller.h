/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#ifndef IVW_ANIMATIONCONTROLLER_H
#define IVW_ANIMATIONCONTROLLER_H

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/timer.h>
#include <inviwo/core/common/inviwoapplication.h>

#include <modules/animation/datastructures/animation.h>
#include <modules/animation/datastructures/animationtime.h>
#include <modules/animation/datastructures/animationstate.h>
#include <modules/animation/animationcontrollerobserver.h>

namespace inviwo {

namespace animation {

/**
 * The AnimationController is responsible for evaluating the animation and keeping track of the
 * animation time and state.
 */
class IVW_MODULE_ANIMATION_API AnimationController : public AnimationControllerObservable {
public:
    AnimationController(Animation* animation, InviwoApplication* app = InviwoApplication::getPtr());
    virtual ~AnimationController();

    // Start animation
    void play();
    // Pause animation
    void pause();
    // Pause and reset to start
    void stop();
    
    void setState(AnimationState newState);
    void setPlaybackMode(PlaybackMode mode);

    /**
     * Advances the animation to the next time step given by @setPlaySpeed if the animation is in
     * a playing state and evaluates, otherwise does nothing. 
     */
    void tick();
    void eval(Seconds oldTime, Seconds newTime);

    void setAnimation(Animation* animation);
    void setPlaySpeed(double framesPerSecond);

    Animation* getAnimation();
    const Animation* getAnimation() const;
    const AnimationState& getState() const;
    const PlaybackMode& getPlaybackMode() const;
    Seconds getCurrentTime() const;
    Seconds getPlaySpeedTime() const;
    double getPlaySpeedFps() const;

protected:
    void setTime(Seconds time); // Use eval to set time in the public interface.

    Animation* animation_;  ///< non-owning reference
    InviwoApplication* app_;
    AnimationState state_;
    PlaybackMode mode_;
    Seconds currentTime_;
    Seconds deltaTime_;
    Timer timer_;
};

} // namespace

} // namespace

#endif // IVW_ANIMATIONCONTROLLER_H

