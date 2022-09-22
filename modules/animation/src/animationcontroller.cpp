/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2022 Inviwo Foundation
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

#include <inviwo/core/common/factoryutil.h>                       // for getDataWriterFactory
#include <inviwo/core/common/inviwoapplication.h>                 // for InviwoApplication
#include <inviwo/core/io/datawriterfactory.h>                     // for DataWriterFactory
#include <inviwo/core/io/serialization/deserializer.h>            // for Deserializer
#include <inviwo/core/io/serialization/serializationexception.h>  // for SerializationException
#include <inviwo/core/io/serialization/serializer.h>              // for Serializer
#include <inviwo/core/network/networklock.h>                      // for NetworkLock
#include <inviwo/core/network/processornetwork.h>                 // for ProcessorNetwork
#include <inviwo/core/processors/canvasprocessor.h>               // for CanvasProcessor
#include <inviwo/core/processors/processor.h>                     // for Processor
#include <inviwo/core/properties/boolproperty.h>                  // for BoolProperty
#include <inviwo/core/properties/buttonproperty.h>                // for ButtonProperty
#include <inviwo/core/properties/compositeproperty.h>             // for CompositeProperty
#include <inviwo/core/properties/constraintbehavior.h>            // for ConstraintBehavior, Con...
#include <inviwo/core/properties/directoryproperty.h>             // for DirectoryProperty
#include <inviwo/core/properties/invalidationlevel.h>             // for InvalidationLevel, Inva...
#include <inviwo/core/properties/minmaxproperty.h>                // for DoubleMinMaxProperty
#include <inviwo/core/properties/optionproperty.h>                // for OptionPropertyOption
#include <inviwo/core/properties/ordinalproperty.h>               // for IntVec2Property, IntSiz...
#include <inviwo/core/properties/ordinalrefproperty.h>            // for DoubleRefProperty
#include <inviwo/core/properties/propertyowner.h>                 // for PropertyOwner
#include <inviwo/core/properties/propertysemantics.h>             // for PropertySemantics, Prop...
#include <inviwo/core/properties/stringproperty.h>                // for StringProperty
#include <inviwo/core/properties/valuewrapper.h>                  // for PropertySerializationMode
#include <inviwo/core/util/assertion.h>                           // for ivwAssert
#include <inviwo/core/util/fileextension.h>                       // for FileExtension, operator<<
#include <inviwo/core/util/glmvec.h>                              // for ivec2, dvec2
#include <inviwo/core/util/staticstring.h>                        // for operator+
#include <inviwo/core/util/stdextensions.h>                       // for transform
#include <inviwo/core/util/stringconversion.h>                    // for toString
#include <inviwo/core/util/timer.h>                               // for Timer
#include <inviwo/core/util/utilities.h>                           // for saveAllCanvases
#include <modules/animation/datastructures/animation.h>           // for Animation
#include <modules/animation/datastructures/animationstate.h>      // for AnimationState, Playbac...
#include <modules/animation/datastructures/animationtime.h>       // for Seconds
#include <modules/animation/datastructures/controltrack.h>        // for ControlTrack
#include <modules/animation/datastructures/track.h>               // for Track

#include <algorithm>                                              // for max, copy_if, find_if, min
#include <chrono>                                                 // for milliseconds, duration
#include <cstdlib>                                                // for abs, size_t
#include <iomanip>                                                // for operator<<, setfill, setw
#include <iterator>                                               // for back_insert_iterator
#include <map>                                                    // for map
#include <memory>                                                 // for make_unique
#include <ratio>                                                  // for ratio
#include <sstream>                                                // for operator<<, basic_ostream
#include <string_view>                                            // for string_view, operator==
#include <unordered_map>                                          // for unordered_map
#include <utility>                                                // for move

#include <glm/vec2.hpp>                                           // for vec<>::(anonymous), ope...

namespace inviwo {
class Layer;

namespace animation {

namespace {

constexpr std::string_view defaultImageExt = "png";

std::vector<std::string> imageExts(InviwoApplication* app) {
    return util::transform(util::getDataWriterFactory(app)->getExtensionsForType<Layer>(),
                           [](const auto& i) -> std::string { return toString(i); });
}

size_t imageExtIndex(InviwoApplication* app, std::string_view ext) {
    auto exts = util::getDataWriterFactory(app)->getExtensionsForType<Layer>();
    auto it = std::find_if(exts.begin(), exts.end(), [&](auto& e) { return e.extension_ == ext; });
    if (it != exts.end()) {
        return std::distance(exts.begin(), it);
    } else {
        return 0;
    }
}

}  // namespace

AnimationController::AnimationController(Animation& animation, InviwoApplication* app)
    : deltaTime_(Seconds(1.0 / 25.0))
    , playOptions("PlayOptions", "Play Settings")
    , playWindowMode("PlayFirstLastTimeOption", "Time",
                     {{"FullTimeWindow", "Play full animation", 0},
                      {"UserTimeWindow", "Selected time window", 1}},
                     0)
    , playWindow("PlayFirstLastTime", "Window", 0, 10, 0, 1e5, 1, 0.0,
                 InvalidationLevel::InvalidOutput, PropertySemantics::Text)
    , framesPerSecond(
          "PlayFramesPerSecond", "Frames per Second",
          [this]() { return 1.0 / std::abs(deltaTime_.count()); },
          [this](const double& value) { deltaTime_ = Seconds(1.0 / value); },
          {0.001, ConstraintBehavior::Immutable}, {1000.0, ConstraintBehavior::Immutable}, 1.0,
          InvalidationLevel::InvalidOutput, PropertySemantics::Text)
    , playMode("PlayMode", "Mode",
               {{"Once", "Play once", PlaybackMode::Once},
                {"Loop", "Loop animation", PlaybackMode::Loop},
                {"Swing", "Swing animation", PlaybackMode::Swing}},
               0)
    , renderOptions("RenderOptions", "Render Animation")
    , renderWindowMode("RenderFirstLastTimeOption", "Time",
                       {{"FullTimeWindow", "Render full animation", 0},
                        {"UserTimeWindow", "Selected time window", 1}},
                       0)
    , renderWindow("RenderFirstLastTime", "Window", 0, 10, 0, 1e5, 1, 0.0,
                   InvalidationLevel::InvalidOutput, PropertySemantics::Text)
    , renderSizeMode("RenderSizeOptions", "Size",
                     {{"CurrentCanvas", "Use current settings of canvases", 0},
                      {"720p", "720p for all canvases", 1},
                      {"1080p", "1080p for all canvases", 2},
                      {"CustomSize", "User-defined resolution for all canvases", 3}},
                     0)
    , renderSize("RenderSize", "Pixels", ivec2(1024), ivec2(1), ivec2(20000), ivec2(256),
                 InvalidationLevel::InvalidOutput, PropertySemantics::Text)
    , renderAspectRatio("RenderSizeAspectRatio", "Aspect Ratio",
                        {{"Ignore", "Ignore aspect ratio", 0},
                         {"KeepInside", "Keep aspect ratio within given resolution", 1},
                         {"KeepEnlarge", "Keep aspect ratio exceeding given resolution", 2}},
                        1)
    , renderLocation("RenderLocationDir", "Directory")
    , renderBaseName("RenderLocationBaseName", "Base Name")
    , renderImageExtension("RenderImageExtension", "Type", imageExts(app),
                           imageExtIndex(app, defaultImageExt))
    , renderNumFrames("RenderNumFrames", "# Frames", 100, 2, 1000000, 1,
                      InvalidationLevel::InvalidOutput, PropertySemantics::Text)
    , renderAction("RenderAction", "Render")
    , renderActionStop("RenderActionStop", "Stop")
    , controlOptions("ControlOptions", "Control Track")
    , controlInsertPauseFrame("ControlInsertPauseFrame", "Add Control Track")
    , animation_(&animation)
    , app_(app)
    , state_(AnimationState::Paused)
    , currentTime_(0)
    , timer_{std::chrono::duration_cast<std::chrono::milliseconds>(deltaTime_), [this] {
                 if (state_ == AnimationState::Rendering)
                     tickRender();
                 else
                     tick();
             }} {

    // Play Settings
    playWindowMode.onChange([&]() { playWindow.setVisible(playWindowMode.get() == 1); });
    playWindow.setVisible(playWindowMode.get() == 1);

    playOptions.addProperties(playWindowMode, playWindow, framesPerSecond, playMode);
    playOptions.setCollapsed(true);
    addProperty(playOptions);

    framesPerSecond.onChange([this]() {
        timer_.setInterval(
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::abs(deltaTime_)));
    });

    // Rendering Settings
    renderWindowMode.onChange([&]() { renderWindow.setVisible(renderWindowMode.get() == 1); });
    renderWindow.setVisible(renderWindowMode.get() == 1);

    renderSizeMode.onChange([&]() {
        renderSize.setVisible(renderSizeMode.get() == 3);
        renderAspectRatio.setVisible(renderSizeMode.get() > 0);
    });
    renderSize.setVisible(renderSizeMode.get() == 3);
    renderAspectRatio.setVisible(renderSizeMode.get() > 0);

    renderAction.onChange([&]() { render(); });
    renderAction.setVisible(state_ != AnimationState::Rendering);

    renderActionStop.onChange([&]() { pause(); });
    renderActionStop.setVisible(state_ == AnimationState::Rendering);

    renderOptions.addProperty(renderWindowMode);
    renderOptions.addProperty(renderWindow);
    renderOptions.addProperty(renderSizeMode);
    renderOptions.addProperty(renderAspectRatio);
    renderOptions.addProperty(renderSize);
    renderOptions.addProperty(renderNumFrames);
    renderOptions.addProperty(renderLocation);
    renderOptions.addProperty(renderBaseName);
    renderOptions.addProperty(renderImageExtension);
    renderOptions.addProperty(renderAction);
    renderOptions.addProperty(renderActionStop);
    renderOptions.setCollapsed(true);
    addProperty(renderOptions);

    renderImageExtension.setSerializationMode(PropertySerializationMode::None);

    // Control Track
    controlInsertPauseFrame.onChange([this]() {
        auto ct = std::make_unique<ControlTrack>();
        animation_->add(std::move(ct));
    });

    controlOptions.addProperty(controlInsertPauseFrame);
    controlOptions.setCollapsed(true);
    addProperty(controlOptions);
}

AnimationController::~AnimationController() = default;

void AnimationController::setState(AnimationState newState) {
    if (state_ == newState) return;

    // We will be switching state: clean after rendering
    if (state_ == AnimationState::Rendering) afterRender();

    // Switch state
    auto oldState = state_;
    state_ = newState;

    // Act on new state
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

void AnimationController::setTime(Seconds time) {
    // No upper boundary check since you might want to set the time after the last keyframe of
    // animation when creating new ones
    if (currentTime_ != time) {
        auto oldTime = currentTime_;
        currentTime_ = std::max(Seconds(0), time);
        notifyTimeChanged(this, oldTime, currentTime_);
    }
}

void AnimationController::play() { setState(AnimationState::Playing); }

void AnimationController::render() {
    // Gather rendering info
    renderState_.firstTime =
        (renderWindowMode.get() == 0) ? animation_->getFirstTime() : Seconds(renderWindow.get()[0]);
    renderState_.lastTime =
        (renderWindowMode.get() == 0) ? animation_->getLastTime() : Seconds(renderWindow.get()[1]);
    renderState_.numFrames = renderNumFrames.get();
    if (renderState_.numFrames < 2) renderState_.numFrames = 2;
    renderState_.currentFrame = -1;  // first run, see below in tickRender()
    renderState_.baseFileName = renderLocation.get() + "/" + renderBaseName.get();
    // - digits of the frame counter
    renderState_.digits = 0;
    int number(renderState_.numFrames - 1);
    while (number) {
        number /= 10;
        renderState_.digits++;
    }
    // - use at least 4 digits, so we nicely overwrite the files from a previous test rendering with
    // less frames
    renderState_.digits = std::max(renderState_.digits, 4);

    // Get all active canvases
    auto network = app_->getProcessorNetwork();
    NetworkLock lock(network);
    auto allCanvases = network->getProcessorsByType<CanvasProcessor>();
    std::vector<CanvasProcessor*> activeCanvases;
    std::copy_if(allCanvases.begin(), allCanvases.end(), std::back_inserter(activeCanvases),
                 [](auto canvas) { return canvas->isSink(); });

    // Canvas indication replacement in filename
    renderState_.canvasIndicator = activeCanvases.size() > 1 ? "_UPN_" : "";

    // Alter the settings of the canvases, so we can shoot in the right resolution
    // - This will be restored later
    renderState_.origCanvasSettings.clear();
    if (renderSizeMode.get() != 0) {
        renderState_.origCanvasSettings.resize(activeCanvases.size());
        // For each active canvas
        for (auto canvas : activeCanvases) {

            // Save original state
            renderState_.origCanvasSettings.emplace_back();
            auto& settings = renderState_.origCanvasSettings.back();
            settings.enableCustomInputDimensions_ = canvas->enableCustomInputDimensions_.get();
            settings.customInputDimensions_ = canvas->customInputDimensions_.get();
            settings.keepAspectRatio_ = canvas->keepAspectRatio_.get();
            settings.canvasIdentifier = canvas->getIdentifier();

            // Calculate new dimensions
            // - dimensions of the canvas widget
            const dvec2& actualDims = canvas->dimensions_.get();
            // - basic dimensions desired
            ivec2 desiredDims{0};
            switch (renderSizeMode.get()) {
                case 1: {
                    desiredDims = ivec2(1280, 720);
                    break;
                }
                case 2: {
                    desiredDims = ivec2(1920, 1080);
                    break;
                }
                case 3: {
                    desiredDims = renderSize.get();
                    break;
                }
                default: {
                    ivwAssert(false, "Should not happen.");
                }
            }
            // - adjust basic dimensions to the aspect ratio
            if (renderAspectRatio.get() > 0) {
                const double widthFactor = double(desiredDims.x) / actualDims.x;
                const double heightFactor = double(desiredDims.y) / actualDims.y;
                // 1 - image is at most the given resolution, or smaller
                // 2 - image is at least the given resolution, or larger
                const double factor = (renderAspectRatio.get() == 1)
                                          ? std::min(widthFactor, heightFactor)
                                          : std::max(widthFactor, heightFactor);
                desiredDims = static_cast<ivec2>(factor * actualDims);
            }

            // Set new state
            canvas->enableCustomInputDimensions_.set(true);
            canvas->keepAspectRatio_.set(false);
            canvas->customInputDimensions_.set(desiredDims);
        }
    }

    // Switch Buttons
    renderAction.setVisible(false);
    renderActionStop.setVisible(true);

    // Go for it!
    setState(AnimationState::Rendering);
}

void AnimationController::afterRender() {
    // Switch Buttons
    renderActionStop.setVisible(false);
    renderAction.setVisible(true);

    // Restore original state of Canvases
    auto network = app_->getProcessorNetwork();
    NetworkLock lock(network);
    if (!network) return;
    for (auto& origSettings : renderState_.origCanvasSettings) {
        if (auto canvas = dynamic_cast<CanvasProcessor*>(
                network->getProcessorByIdentifier(origSettings.canvasIdentifier))) {

            canvas->enableCustomInputDimensions_.set(origSettings.enableCustomInputDimensions_);
            canvas->keepAspectRatio_.set(origSettings.keepAspectRatio_);
            canvas->customInputDimensions_.set(origSettings.customInputDimensions_);
        }
    }
}

void AnimationController::pause() { setState(AnimationState::Paused); }

void AnimationController::stop() {
    setState(AnimationState::Paused);
    deltaTime_ = std::chrono::abs(deltaTime_);  // Make sure we play forward.
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

    // Get active time window for playing
    // init with sub-window, overwrite with full window if necessary
    Seconds firstTime = Seconds(playWindow.get()[0]);
    Seconds lastTime = Seconds(playWindow.get()[1]);
    if (playWindowMode.get() == 0) {
        // Full animation window
        firstTime = animation_->getFirstTime();
        lastTime = animation_->getLastTime();
    }
    auto newState{state_};
    // Ping at the end of time
    if (newTime > lastTime) {
        switch (playMode.get()) {
            case PlaybackMode::Once: {
                newTime = lastTime;
                newState = AnimationState::Paused;
                break;
            }
            case PlaybackMode::Loop: {
                newTime = firstTime;
                break;
            }
            case PlaybackMode::Swing: {
                setPlaybackDirection(PlaybackDirection::Backward);
                newTime = lastTime + deltaTime_;
                break;
            }
            default:
                break;
        }
    }

    // Pong at the beginning of time
    if (newTime < firstTime) {
        switch (playMode.get()) {
            case PlaybackMode::Once: {
                newTime = firstTime;
                newState = AnimationState::Paused;
                break;
            }
            case PlaybackMode::Loop: {
                newTime = lastTime;
                break;
            }
            case PlaybackMode::Swing: {
                setPlaybackDirection(PlaybackDirection::Forward);
                newTime = firstTime + deltaTime_;
                break;
            }
            default:
                break;
        }
    }

    // Evaluate animation
    eval(currentTime_, newTime);

    // May be in paused state
    setState(newState);
}

void AnimationController::tickRender() {
    if (state_ != AnimationState::Rendering) {
        setState(AnimationState::Paused);
        return;
    }

    // Save all active canvases from last rendering tick:
    // Apparently, we need to be outside of tickRender() to let the system refresh its state
    // The first call to tickRender() is done with renderState_.currentFrame == -1 to bring the
    // system to a proper state
    // - generate filename pattern
    if (renderState_.currentFrame >= 0) {
        std::stringstream fileNamePattern;
        fileNamePattern << renderBaseName.get() << renderState_.canvasIndicator << std::setfill('0')
                        << std::setw(renderState_.digits) << renderState_.currentFrame;
        auto ext = FileExtension::createFileExtensionFromString(renderImageExtension.get());
        // - save active canvases
        util::saveAllCanvases(app_->getProcessorNetwork(), renderLocation.get(),
                              fileNamePattern.str(), ext.extension_, true);
    }

    // Next!
    renderState_.currentFrame++;

    // Are we finished?
    if (renderState_.currentFrame >= renderState_.numFrames) {
        setState(AnimationState::Paused);
        return;
    }

    // We render with equidistant steps
    Seconds newTime = renderState_.firstTime;
    if (renderState_.currentFrame >= 0) {
        const double progress =
            double(renderState_.currentFrame) / double(renderState_.numFrames - 1);
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

void AnimationController::setAnimation(Animation& animation) {
    auto oldAnim = animation_;
    animation_ = &animation;

    notifyAnimationChanged(this, oldAnim, animation_);
    setState(AnimationState::Paused);
    setTime(Seconds(0.0));
}

const Animation& AnimationController::getAnimation() const { return *animation_; }

Animation& AnimationController::getAnimation() { return *animation_; }

const AnimationState& AnimationController::getState() const { return state_; }

PlaybackDirection AnimationController::getPlaybackDirection() const {
    return deltaTime_ < Seconds(0) ? PlaybackDirection::Backward : PlaybackDirection::Forward;
}

void AnimationController::setPlaybackDirection(PlaybackDirection newDirection) {
    if (newDirection == PlaybackDirection::Forward) {
        deltaTime_ = std::chrono::abs(deltaTime_);
    } else {
        deltaTime_ = -std::chrono::abs(deltaTime_);
    }
}

Seconds AnimationController::getCurrentTime() const { return currentTime_; }

void AnimationController::resetAllPoperties() {

    const auto options = imageExts(app_);
    const auto selectedIndex = imageExtIndex(app_, defaultImageExt);

    renderImageExtension.replaceOptions(options);
    renderImageExtension.setSelectedIndex(selectedIndex);
    renderImageExtension.setCurrentStateAsDefault();

    PropertyOwner::resetAllPoperties();
}

void AnimationController::serialize(Serializer& s) const {
    PropertyOwner::serialize(s);
    s.serialize("framesPerSecond", framesPerSecond.get());
    if (renderImageExtension.size() > 0) {
        s.serialize("renderImageExtension", renderImageExtension.getSelectedValue());
    }
}

void AnimationController::deserialize(Deserializer& d) {
    PropertyOwner::deserialize(d);

    const auto options = imageExts(app_);
    std::string selectedValue{defaultImageExt};

    double fps = framesPerSecond;
    d.deserialize("framesPerSecond", fps);
    framesPerSecond.set(fps);
    d.deserialize("renderImageExtension", selectedValue);

    renderImageExtension.replaceOptions(options);
    renderImageExtension.setSelectedValue(selectedValue);
    renderImageExtension.setCurrentStateAsDefault();
}

}  // namespace animation

}  // namespace inviwo
