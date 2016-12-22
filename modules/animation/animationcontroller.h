/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#include <modules/animation/datastructures/animation.h>
#include <modules/animation/datastructures/animationtime.h>
#include <modules/animation/datastructures/animationstate.h>
#include <modules/animation/animationcontrollerobserver.h>

namespace inviwo {

class Timer;

namespace animation {

/**
 * \class AnimationController
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS
 */
class IVW_MODULE_ANIMATION_API AnimationController : public AnimationControllerObservable {
public:
    AnimationController(Animation* animation);
    virtual ~AnimationController();

    // Start animation
    void play();
    // Pause animation
    void pause();
    // Pause and reset to start
    void stop();

    // Progresses time and evaluates animation
    void tick();
    void eval(Seconds oldTime, Seconds newTime);

    void setAnimation(Animation* animation);
    void setCurrentTime(Seconds time);
    void setPlaySpeed(double framesPerSecond);

    Animation* getAnimation() { return animation_; }
    const Animation* getAnimation() const { return animation_; }
    const AnimationState& getState() const { return state_; }
    const Seconds getCurrentTime() const { return currentTime_; }
    const Seconds getPlaySpeedTime() const { return deltaTime_; }
    const double getPlaySpeedFps() const { return 1.0 / deltaTime_.count(); }

protected:
    Animation* animation_;  ///< non-owning reference
    AnimationState state_;
    Seconds currentTime_;
    Seconds deltaTime_;

    std::unique_ptr<Timer> timer_;
};

} // namespace

} // namespace

#endif // IVW_ANIMATIONCONTROLLER_H

