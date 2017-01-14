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

#include <modules/animation/animationcontroller.h>
#include <modules/animation/animationcontrollerobserver.h>
#include <inviwo/core/network/networklock.h>

namespace inviwo {

namespace animation {

AnimationController::AnimationController(Animation* animation, InviwoApplication* app)
    : animation_(animation)
    , app_(app)
    , state_(AnimationState::Paused)
    , mode_(PlaybackMode::Once)
    , currentTime_(0)
    , deltaTime_(Seconds(1.0 / 60.0))
    , timer_{std::chrono::duration_cast<std::chrono::milliseconds>(deltaTime_),
             [this] { tick(); }} {}

AnimationController::~AnimationController() = default;

void AnimationController::setState(AnimationState newState) {
    if (state_ == newState) return;
    auto oldState = state_;
    state_ = newState;
    switch (newState) {
        case AnimationState::Playing: {
            app_->getInteractionStateManager().beginInteraction();
            timer_.start();
            break;
        }
        case AnimationState::Paused: {
            timer_.stop();
            app_->getInteractionStateManager().endInteraction();
            break;
        }

        default:
            break;
    }
    notifyStateChanged(this, oldState, state_);
}

void AnimationController::setPlaybackMode(PlaybackMode mode) {
    if (mode_ != mode) {
        auto oldmode = mode_;
        mode_ = mode;
        notifyPlaybackModeChanged(this, oldmode, mode_);
    }
}

void AnimationController::setTime(Seconds time) {
    // No upper boundary check since you might want to set the time after the last keyframe of
    // animation when creating new ones
    if (currentTime_ != time) {
        auto oldTime = currentTime_;
        currentTime_ = std::max(Seconds(0), time);
        notifyTimeChanged(this, oldTime, currentTime_);
    }
}

void AnimationController::play() {
    setState(AnimationState::Playing);
}

void AnimationController::pause() {
    setState(AnimationState::Paused);
}

void AnimationController::stop() {
    setState(AnimationState::Paused);
    eval(currentTime_, Seconds(0));
}

void AnimationController::tick() {
    // TODO: Implement fully working solution for this.
    // What to do when network cannot be evaluated in the speed that is given by deltaTime?
    // Initial solution: Don't care about that, and let it evaluate fully in the speed that it can
    // muster
    // Since we probably want to generate an imagesequence or something for videos.

    if (state_ == AnimationState::Playing) {
        auto newTime = currentTime_ + deltaTime_;

        if (newTime > animation_->lastTime()) {
            switch (mode_) {
                case PlaybackMode::Once: {
                    newTime = animation_->lastTime();
                    setState(AnimationState::Paused);
                    break;
                }
                case PlaybackMode::Loop: {
                    newTime = animation_->firstTime();
                    break;
                }
                default:
                    break;
            }
        }

        // Evaluate animation
        eval(currentTime_, newTime);
    }
}

void AnimationController::eval(Seconds oldTime, Seconds newTime) {
    NetworkLock lock;
    auto ts = (*animation_)(oldTime, newTime, state_);
    setState(ts.state);
    setTime(ts.time);
}

void AnimationController::setAnimation(Animation* animation) {
    auto oldAnim = animation_;
    animation_ = animation;

    notifyAnimationChanged(this, oldAnim, animation_);
    setState(AnimationState::Paused);
    setTime(Seconds(0.0));
}

void AnimationController::setPlaySpeed(double framesPerSecond) {
    deltaTime_ = Seconds(1.0 / framesPerSecond);
    timer_.setInterval(std::chrono::duration_cast<std::chrono::milliseconds>(deltaTime_));
}

const Animation* AnimationController::getAnimation() const { return animation_; }

Animation* AnimationController::getAnimation() { return animation_; }

const AnimationState& AnimationController::getState() const { return state_; }

const inviwo::animation::PlaybackMode& AnimationController::getPlaybackMode() const {
    return mode_;
}

const Seconds AnimationController::getCurrentTime() const { return currentTime_; }

const Seconds AnimationController::getPlaySpeedTime() const { return deltaTime_; }

const double AnimationController::getPlaySpeedFps() const { return 1.0 / deltaTime_.count(); }

} // namespace


} // namespace

