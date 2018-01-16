/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2018 Inviwo Foundation
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
    , currentTime_(0)
    , deltaTime_(Seconds(1.0 / 60.0))
    , timer_{std::chrono::duration_cast<std::chrono::milliseconds>(deltaTime_),
             [this] { if (state_ == AnimationState::Rendering) tickRender(); else tick(); }}
    , propPlayOptions("PlayOptions", "Play Settings")
    , propPlayFirstLastTimeOption("PlayFirstLastTimeOption", "")
    , propPlayFirstLastTime("PlayFirstLastTime", "Time Window", 0, 10, 0, 1e5, 1)
    , propPlayFramesPerSecond("PlayFramesPerSecond", "Frames per Second")
    , propPlayMode("PlayMode", "Mode")
    , propRenderOptions("RenderOptions", "Render Animation")
    , propRenderSizeOptions("RenderSizeOptions", "Size")
    , propRenderSize("RenderSize", "Pixels")
    , propRenderNumFrames("RenderNumFrames", "# Frames", 100, 2)
    , propRenderLocationDir("RenderLocationDir", "Directory")
    , propRenderLocationBaseName("RenderLocationBaseName", "BaseName")
    , propRenderAction("RenderAction", "Render") {

    ///////////////////////////
    // Play Settings

    propPlayFirstLastTimeOption.addOption("FullTimeWindow", "Play full animation", 0);
    propPlayFirstLastTimeOption.addOption("UserTimeWindow", "Selected time window", 1);
    propPlayFirstLastTimeOption.onChange([&](){
        propPlayFirstLastTime.setVisible(propPlayFirstLastTimeOption.get() == 1);
    });
    propPlayFirstLastTimeOption.setCurrentStateAsDefault();

    propPlayFirstLastTime.setSemantics(PropertySemantics::Text);
    propPlayFirstLastTime.setVisible(propPlayFirstLastTimeOption.get() == 1);

    propPlayFramesPerSecond.setSemantics(PropertySemantics::Text);

    propPlayMode.addOption("Once", "Play once", (int)PlaybackMode::Once);
    propPlayMode.addOption("Loop", "Loop animation", (int)PlaybackMode::Loop);
    propPlayMode.addOption("Swing", "Swing animation", (int)PlaybackMode::Swing);
    //propPlayMode.onChange([&](){
    //});
    propPlayMode.setCurrentStateAsDefault();

    propPlayOptions.addProperty(propPlayFirstLastTimeOption);
    propPlayOptions.addProperty(propPlayFirstLastTime);
    propPlayOptions.addProperty(propPlayFramesPerSecond);
    propPlayOptions.addProperty(propPlayMode);
    addProperty(propPlayOptions);


    ///////////////////////////
    // Rendering Settings

    propRenderSizeOptions.addOption("CurrentCanvasSize", "Use current size of canvases", 0);
    propRenderSizeOptions.addOption("SaveImageSize", "Use SaveImg size of canvases", 1);
    propRenderSizeOptions.addOption("720p", "720p for all canvases", 2);
    propRenderSizeOptions.addOption("1080p", "1080p for all canvases", 3);
    propRenderSizeOptions.addOption("CustomSize", "User-defined resolution for all canvases", 4);
    propRenderSizeOptions.onChange([&](){
        propRenderSize.setVisible(propRenderSizeOptions.get() == 4);
    });
    propRenderSizeOptions.setCurrentStateAsDefault();

    propRenderSize.setSemantics(PropertySemantics::Text);
    propRenderSize.setVisible(propRenderSizeOptions.get() == 4);

    propRenderNumFrames.setSemantics(PropertySemantics::Text);

    propRenderAction.onChange([&](){
        render();
    });

    propRenderOptions.addProperty(propRenderSizeOptions);
    propRenderOptions.addProperty(propRenderSize);
    propRenderOptions.addProperty(propRenderNumFrames);
    propRenderOptions.addProperty(propRenderLocationDir);
    propRenderOptions.addProperty(propRenderLocationBaseName);
    propRenderOptions.addProperty(propRenderAction);
    addProperty(propRenderOptions);
}

AnimationController::~AnimationController() = default;

void AnimationController::setState(AnimationState newState) {
    if (state_ == newState) return;
    auto oldState = state_;
    state_ = newState;
    switch (newState) {
        case AnimationState::Playing:
        case AnimationState::Rendering: {
            timer_.start();
            break;
        }
        case AnimationState::Paused: {
            timer_.stop();
            break;
        }

        default:
            break;
    }
    notifyStateChanged(this, oldState, state_);
}


void AnimationController::setPlaybackSettings(const AnimationPlaySettings& newSettings) {
    if (settingsPlay_ != newSettings) {
        auto oldSettings = settingsPlay_;
        settingsPlay_ = newSettings;
        notifyPlaybackSettingsChanged(this, oldSettings, settingsPlay_);
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
    deltaTime_ = Seconds(fabs(deltaTime_.count())); //Make sure we play forward.
    setState(AnimationState::Playing);
}

void AnimationController::render() {
    setState(AnimationState::Rendering);
}

void AnimationController::pause() {
    setState(AnimationState::Paused);
}

void AnimationController::stop() {
    setState(AnimationState::Paused);
    eval(currentTime_, Seconds(0));
}

void AnimationController::tick() {
    if (state_ != AnimationState::Playing) {
        setState(AnimationState::Paused);
        return;
    }

    // TODO: Implement fully working solution for this.
    // What to do when network cannot be evaluated in the speed that is given by deltaTime?
    // Initial solution: Don't care about that, and let it evaluate fully in the speed that it can
    // muster.
    auto newTime = currentTime_ + deltaTime_;

    //Get active time window for playing
    // - init with sub-window, overwrite with full window if necessary
    Seconds firstTime = Seconds(propPlayFirstLastTime.get()[0]);
    Seconds lastTime = Seconds(propPlayFirstLastTime.get()[1]);
    if (propPlayFirstLastTimeOption.get() == 0) {
        //Full animation window
        firstTime = animation_->firstTime();
        lastTime = animation_->lastTime();
    }

    //Ping at the end of time
    if (newTime > lastTime) {
        switch (propPlayMode.get()) {
            case PlaybackMode::Once: {
                newTime = lastTime;
                setState(AnimationState::Paused);
                break;
            }
            case PlaybackMode::Loop: {
                newTime = firstTime;
                break;
            }
            case PlaybackMode::Swing: {
                deltaTime_ = -deltaTime_;
                newTime = lastTime + deltaTime_;
                break;
            }
            default:
                break;
        }
    }

    //Pong at the beginning of time
    if (newTime < firstTime) {
        switch (propPlayMode.get()) {
            case PlaybackMode::Once: {
                newTime = firstTime;
                setState(AnimationState::Paused);
                break;
            }
            case PlaybackMode::Loop: {
                newTime = lastTime;
                break;
            }
            case PlaybackMode::Swing: {
                deltaTime_ = -deltaTime_;
                newTime = firstTime + deltaTime_;
                break;
            }
            default:
                break;
        }
    }

    // Evaluate animation
    eval(currentTime_, newTime);
}

void AnimationController::tickRender() {
    if (state_ != AnimationState::Rendering) {
        setState(AnimationState::Paused);
        return;
    }

    //We render with equidistant steps
    auto newTime = currentTime_ + deltaTime_;

    //To be implemented.

    setState(AnimationState::Paused);

    // Evaluate animation
    eval(currentTime_, newTime);
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

Seconds AnimationController::getCurrentTime() const { return currentTime_; }

} // namespace


} // namespace

