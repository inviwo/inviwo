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

#include <modules/animation/animationcontroller.h>
#include <modules/animation/animationcontrollerobserver.h>
#include <inviwo/core/util/timer.h>
#include <inviwo/core/network/networklock.h>

namespace inviwo {

namespace animation {

AnimationController::AnimationController(Animation* animation)
    : animation_(animation), state_(AnimationState::Paused), currentTime_(0), deltaTime_(0) {

    auto tickTime = std::chrono::duration_cast<std::chrono::milliseconds>(deltaTime_);
    timer_ = std::make_unique<Timer>(tickTime, [this] { tick(); });

    setPlaySpeed(60.0);
}

AnimationController::~AnimationController() = default;

void AnimationController::play() {
    auto oldState = state_;
    state_ = AnimationState::Playing;
    timer_->start();

    // TODO: Perhaps only trigger if oldstate != state_?
    notifyStateChanged(this, oldState, state_);
}

void AnimationController::pause() {
    auto oldState = state_;
    state_ = AnimationState::Paused;
    timer_->stop();

    // TODO: Perhaps only trigger if oldstate != state_?
    notifyStateChanged(this, oldState, state_);
}

void AnimationController::stop() {
    auto oldState = state_;
    state_ = AnimationState::Paused;
    timer_->stop();
    setCurrentTime(Seconds(0));

    // TODO: Perhaps only trigger if oldstate != state_?
    notifyStateChanged(this, oldState, state_);
}

void AnimationController::tick() {
    // TODO: Implement fully working solution for this.
    // What to do when network cannot be evaluated in the speed that is given by deltaTime?
    // Initial solution: Don't care about that, and let it evaluate fully in the speed that it can
    // muster
    // Since we probably want to generate an imagesequence or something for videos.

    if (state_ == AnimationState::Playing) {
        auto oldTime = currentTime_;
        currentTime_ += deltaTime_;

        if (currentTime_ > animation_->lastTime()) {
            currentTime_ = animation_->lastTime();

            state_ = AnimationState::Paused;
            notifyStateChanged(this, AnimationState::Playing, AnimationState::Paused);
        }

        // Evaluate animation
        eval(oldTime, currentTime_);

        notifyTimeChanged(this, oldTime, currentTime_);
    }
}

void AnimationController::eval(Seconds oldTime, Seconds newTime) {
    NetworkLock lock;
    (*animation_)(oldTime, newTime);
}

void AnimationController::setAnimation(Animation* animation) {
    auto oldAnim = animation_;
    auto oldState = state_;
    auto oldTime = currentTime_;

    animation_ = animation;
    state_ = AnimationState::Paused;
    currentTime_ = Seconds(0.0);

    notifyAnimationChanged(this, oldAnim, animation_);
    notifyStateChanged(this, oldState, state_);
    notifyTimeChanged(this, oldTime, currentTime_);
}

void AnimationController::setCurrentTime(Seconds time) {
    // TODO: Boundary check to not go outside of current animation?
    // Probably no, since you might want to set the time after the last keyframe of animation when
    // creating new ones
    auto oldTime = currentTime_;
    currentTime_ = std::max(Seconds(0), time);
    eval(oldTime, currentTime_);
    notifyTimeChanged(this, oldTime, currentTime_);
}

void AnimationController::setPlaySpeed(double framesPerSecond) {
    deltaTime_ = Seconds(1.0 / framesPerSecond);
    timer_->setInterval(std::chrono::duration_cast<std::chrono::milliseconds>(deltaTime_));
}

} // namespace


} // namespace

