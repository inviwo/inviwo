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
    , propPlayFirstLastTimeOption("PlayFirstLastTimeOption", "Time")
    , propPlayFirstLastTime("PlayFirstLastTime", "Window", 0, 10, 0, 1e5, 1)
    , propPlayFramesPerSecond("PlayFramesPerSecond", "Frames per Second")
    , propPlayMode("PlayMode", "Mode")
    , propRenderOptions("RenderOptions", "Render Animation")
    , propRenderFirstLastTimeOption("RenderFirstLastTimeOption", "Time")
    , propRenderFirstLastTime("RenderFirstLastTime", "Window", 0, 10, 0, 1e5, 1)
    , propRenderSizeOptions("RenderSizeOptions", "Size")
    , propRenderSize("RenderSize", "Pixels", ivec2(1024), ivec2(1), ivec2(20000), ivec2(256))
    , propRenderSizeAspectRatio("RenderSizeAspectRatio", "Aspect Ratio")
    , propRenderNumFrames("RenderNumFrames", "# Frames", 100, 2, 1e6)
    , propRenderLocationDir("RenderLocationDir", "Directory")
    , propRenderLocationBaseName("RenderLocationBaseName", "Base Name")
    , propRenderImageExtension("RenderImageExtension", "Type")
    , propRenderAction("RenderAction", "Render")
    , propRenderActionStop("RenderActionStop", "Stop")
	, propControlOptions("ControlOptions", "Control Track")
	, propControlInsertPauseFrame("ControlInsertPauseFrame", "Insert Pause-Frame") {

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

    propRenderFirstLastTimeOption.addOption("FullTimeWindow", "Render full animation", 0);
    propRenderFirstLastTimeOption.addOption("UserTimeWindow", "Selected time window", 1);
    propRenderFirstLastTimeOption.onChange([&](){
        propRenderFirstLastTime.setVisible(propRenderFirstLastTimeOption.get() == 1);
    });
    propRenderFirstLastTimeOption.setCurrentStateAsDefault();

    propRenderFirstLastTime.setSemantics(PropertySemantics::Text);
    propRenderFirstLastTime.setVisible(propRenderFirstLastTimeOption.get() == 1);

    propRenderSizeOptions.addOption("CurrentCanvas", "Use current settings of canvases", 0);
    propRenderSizeOptions.addOption("720p", "720p for all canvases", 1);
    propRenderSizeOptions.addOption("1080p", "1080p for all canvases", 2);
    propRenderSizeOptions.addOption("CustomSize", "User-defined resolution for all canvases", 3);
    propRenderSizeOptions.onChange([&](){
        propRenderSize.setVisible(propRenderSizeOptions.get() == 3);
        propRenderSizeAspectRatio.setVisible(propRenderSizeOptions.get() > 0);
    });
    propRenderSizeOptions.setCurrentStateAsDefault();

    propRenderSize.setSemantics(PropertySemantics::Text);
    propRenderSize.setVisible(propRenderSizeOptions.get() == 3);

    propRenderSizeAspectRatio.addOption("Ignore", "Ignore aspect ratio", 0);
    propRenderSizeAspectRatio.addOption("KeepInside", "Keep aspect ratio within given resolution", 1);
    propRenderSizeAspectRatio.addOption("KeepEnlarge", "Keep aspect ratio exceeding given resolution", 2);
    propRenderSizeAspectRatio.set(1);
    propRenderSizeAspectRatio.setCurrentStateAsDefault();
    propRenderSizeAspectRatio.setVisible(propRenderSizeOptions.get() > 0);

    propRenderNumFrames.setSemantics(PropertySemantics::Text);

    //Add all supported image extensions to option property
    auto factory = app_->getDataWriterFactory();
    if (factory) {
        std::string defaultExt; // save first writer extension matching "png" to be used as default
        for (auto ext : factory->getExtensionsForType<Layer>()) {
            propRenderImageExtension.addOption(ext.toString(), ext.toString());
            if (defaultExt.empty() && ext.extension_ == "png") {
                defaultExt = ext.toString();
            }
        }
        if (!defaultExt.empty()) {
            propRenderImageExtension.setSelectedIdentifier(defaultExt);
        }
    }
    propRenderImageExtension.setCurrentStateAsDefault();

    propRenderAction.onChange([&](){
        render();
    });
    propRenderAction.setVisible(state_ != AnimationState::Rendering);

    propRenderActionStop.onChange([&](){
        pause();
    });
    propRenderActionStop.setVisible(state_ == AnimationState::Rendering);

    propRenderOptions.addProperty(propRenderFirstLastTimeOption);
    propRenderOptions.addProperty(propRenderFirstLastTime);
    propRenderOptions.addProperty(propRenderSizeOptions);
    propRenderOptions.addProperty(propRenderSizeAspectRatio);
    propRenderOptions.addProperty(propRenderSize);
    propRenderOptions.addProperty(propRenderNumFrames);
    propRenderOptions.addProperty(propRenderLocationDir);
    propRenderOptions.addProperty(propRenderLocationBaseName);
    propRenderOptions.addProperty(propRenderImageExtension);
    propRenderOptions.addProperty(propRenderAction);
    propRenderOptions.addProperty(propRenderActionStop);
    propRenderOptions.setCollapsed(true);
    addProperty(propRenderOptions);

	///////////////////////////
	// Control Track

	propControlInsertPauseFrame.onChange([&]() {
		auto time = getCurrentTime();
		ControlKeyframeSequence seq;
		seq.add(ControlKeyframe(time, ControlAction::Pause));
		getAnimation()->getControlTrack().addTyped(seq);
	});

	propControlOptions.addProperty(propControlInsertPauseFrame);
	addProperty(propControlOptions);
}

AnimationController::~AnimationController() = default;

void AnimationController::setState(AnimationState newState) {
    if (state_ == newState) return;

    //We will be switching state: clean after rendering
    if (state_ == AnimationState::Rendering) afterRender();

    //Switch state
    auto oldState = state_;
    state_ = newState;

    //Act on new state
    switch (newState) {
        case AnimationState::Playing: {
            timer_.start();
            break;
        }
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
    setState(AnimationState::Playing);
}

void AnimationController::render() {
    //Gather rendering info
    renderState_.firstTime = (propRenderFirstLastTimeOption.get() == 0) ? animation_->firstTime() : Seconds(propRenderFirstLastTime.get()[0]);
    renderState_.lastTime = (propRenderFirstLastTimeOption.get() == 0) ? animation_->lastTime() : Seconds(propRenderFirstLastTime.get()[1]);
    renderState_.numFrames = propRenderNumFrames.get();
    if (renderState_.numFrames < 2) renderState_.numFrames = 2;
    renderState_.currentFrame = -1; //first run, see below in tickRender()
    renderState_.baseFileName = propRenderLocationDir.get() + "/" + propRenderLocationBaseName.get();
    // - digits of the frame counter
    renderState_.digits = 0;
    int number(renderState_.numFrames - 1);
    while (number) {
        number /= 10;
        renderState_.digits++;
    }
    // - use at least 4 digits, so we nicely overwrite the files from a previous test rendering with less frames
    renderState_.digits = std::max(renderState_.digits, 4);

    //Get all active canvases
    ProcessorNetwork* pNetwork = app_->getProcessorNetwork();
    std::vector<CanvasProcessor*> AllCanvases = pNetwork->getProcessorsByType<CanvasProcessor>();
    std::vector<CanvasProcessor*> ActiveCanvases;
    ActiveCanvases.reserve(AllCanvases.size());
    for (auto ThisCanvas : AllCanvases) {
        if (ThisCanvas->isSink()) ActiveCanvases.push_back(ThisCanvas);
    }

    //Canvas indication replacement in filename
    renderState_.canvasIndicator = ActiveCanvases.size() > 1 ? "_UPN_" : "";

    //Alter the settings of the canvases, so we can shoot in the right resolution
    // - This will be restored later
    renderState_.origCanvasSettings.clear();
    if (propRenderSizeOptions.get() != 0) {
        renderState_.origCanvasSettings.resize(ActiveCanvases.size());
        //For each active canvas
        for (auto ThisCanvas : ActiveCanvases) {

            //Save original state
            renderState_.origCanvasSettings.emplace_back();
            TRenderCanvasSize& ThisSettings = renderState_.origCanvasSettings.back();
            ThisSettings.enableCustomInputDimensions_ = ThisCanvas->enableCustomInputDimensions_.get();
            ThisSettings.customInputDimensions_ = ThisCanvas->customInputDimensions_.get();
            ThisSettings.keepAspectRatio_ = ThisCanvas->keepAspectRatio_.get();
            ThisSettings.canvasIdentifier = ThisCanvas->getIdentifier();

            //Calculate new dimensions
            // - dimensions of the canvas widget
            const ivec2& ActualDims = ThisCanvas->dimensions_.get();
            // - basic dimensions desired
            ivec2 DesiredDims;
            switch (propRenderSizeOptions.get()) {
                case 1: {
                    DesiredDims = ivec2(1280, 720);
                    break;
                }
                case 2: {
                    DesiredDims = ivec2(1920, 1080);
                    break;
                }
                case 3: {
                    DesiredDims = propRenderSize.get();
                    break;
                }
                default: {
                    ivwAssert(false, "Should not happen.");
                }
            }
            // - adjust basic dimensions to the aspect ratio
            if (propRenderSizeAspectRatio.get() > 0) {
                const double WidthFactor = double(DesiredDims.x) / double(ActualDims.x);
                const double HeightFactor = double(DesiredDims.y) / double(ActualDims.y);
                //1 - image is at most the given resolution, or smaller
                //2 - image is at least the given resolution, or larger
                const double Factor = (propRenderSizeAspectRatio.get() == 1) ? std::min(WidthFactor, HeightFactor) : std::max(WidthFactor, HeightFactor);
                DesiredDims.x = Factor * ActualDims.x;
                DesiredDims.y = Factor * ActualDims.y;
            }

            //Set new state
            ThisCanvas->enableCustomInputDimensions_.set(true);
            ThisCanvas->keepAspectRatio_.set(false);
            ThisCanvas->customInputDimensions_.set(DesiredDims);
        }
    }

    //Switch Buttons
    propRenderAction.setVisible(false);
    propRenderActionStop.setVisible(true);

    //Go for it!
    setState(AnimationState::Rendering);
}

void AnimationController::afterRender() {
    //Switch Buttons
    propRenderActionStop.setVisible(false);
    propRenderAction.setVisible(true);

    //Restore original state of Canvases
    ProcessorNetwork* pNetwork = app_->getProcessorNetwork();
    if (!pNetwork) return;
    for (auto& origSettings : renderState_.origCanvasSettings) {
        CanvasProcessor* pCanvas = dynamic_cast<CanvasProcessor*>(pNetwork->getProcessorByIdentifier(origSettings.canvasIdentifier));
        if (pCanvas) {
            pCanvas->enableCustomInputDimensions_.set(origSettings.enableCustomInputDimensions_);
            pCanvas->keepAspectRatio_.set(origSettings.keepAspectRatio_);
            pCanvas->customInputDimensions_.set(origSettings.customInputDimensions_);
        }
    }
}

void AnimationController::pause() {
    setState(AnimationState::Paused);
}

void AnimationController::stop() {
    setState(AnimationState::Paused);
	deltaTime_ = Seconds(fabs(deltaTime_.count())); //Make sure we play forward.
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

    //Save all active canvases from last rendering tick:
    // Apparently, we need to be outside of tickRender() to let the system refresh its state
    // The first call to tickRender() is done with renderState_.currentFrame == -1 to bring the system to a proper state
    // - generate filename pattern
    if (renderState_.currentFrame >= 0) {
        std::stringstream fileNamePattern;
        fileNamePattern << propRenderLocationBaseName.get()
            << renderState_.canvasIndicator
            << std::setfill('0') << std::setw(renderState_.digits) << renderState_.currentFrame;
        auto ext = FileExtension::createFileExtensionFromString(propRenderImageExtension.get());
        // - save active canvases
        util::saveAllCanvases(app_->getProcessorNetwork(), propRenderLocationDir.get(),
                              fileNamePattern.str(), ext.extension_, true);
    }

    //Next!
    renderState_.currentFrame++;

    //Are we finished?
    if (renderState_.currentFrame >= renderState_.numFrames) {
    setState(AnimationState::Paused);
        return;
    }

    //We render with equidistant steps
    Seconds newTime = renderState_.firstTime;
    if (renderState_.currentFrame >= 0) {
        const double progress = double(renderState_.currentFrame) / double(renderState_.numFrames - 1);
        newTime += progress * (renderState_.lastTime - renderState_.firstTime);
    }

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

