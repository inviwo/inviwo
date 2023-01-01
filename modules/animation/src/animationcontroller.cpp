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
#include <inviwo/core/processors/exporter.h>                      // for exportAllFiles
#include <modules/animation/datastructures/animation.h>           // for Animation
#include <modules/animation/datastructures/animationstate.h>      // for AnimationState, Playbac...
#include <modules/animation/datastructures/animationtime.h>       // for Seconds
#include <modules/animation/datastructures/controltrack.h>        // for ControlTrack
#include <modules/animation/datastructures/invalidationtrack.h>   // for InvalidationTrack
#include <modules/animation/datastructures/track.h>               // for Track

#include <algorithm>      // for max, copy_if, find_if, min
#include <chrono>         // for milliseconds, duration
#include <cstdlib>        // for abs, size_t
#include <iomanip>        // for operator<<, setfill, setw
#include <iterator>       // for back_insert_iterator
#include <map>            // for map
#include <memory>         // for make_unique
#include <ratio>          // for ratio
#include <sstream>        // for operator<<, basic_ostream
#include <string_view>    // for string_view, operator==
#include <unordered_map>  // for unordered_map
#include <utility>        // for move

#include <glm/vec2.hpp>  // for vec<>::(anonymous), ope...

namespace inviwo {
class Layer;

namespace animation {

AnimationController::AnimationController(Animation& animation, InviwoApplication* app)
    : playOptions("PlayOptions", "Play Settings")
    , playWindowMode("PlayFirstLastTimeOption", "Time",
                     {
                         {"FullTimeWindow", "Play full animation",  0},
                         {"UserTimeWindow", "Selected time window", 1}
},
                     0),
    playWindow("PlayFirstLastTime", "Window", 0, 10, 0, 1e5, 1, 0.0,
               InvalidationLevel::InvalidOutput, PropertySemantics::Text),
    framesPerSecond("PlayFramesPerSecond", "Frames per Second", 24.0,
                    {0.001, ConstraintBehavior::Immutable}, {1000.0, ConstraintBehavior::Immutable},
                    1.0, InvalidationLevel::InvalidOutput, PropertySemantics::Text),
    playMode("PlayMode", "Mode",
             {{"Once", "Play once", PlaybackMode::Once},
              {"Loop", "Loop animation", PlaybackMode::Loop},
              {"Swing", "Swing animation", PlaybackMode::Swing}},
             0),
    renderOptions("RenderOptions", "Render Animation"),
    renderWindowMode("RenderFirstLastTimeOption", "Time",
                     {{"FullTimeWindow", "Render full animation", 0},
                      {"UserTimeWindow", "Selected time window", 1}},
                     0),
    renderWindow("RenderFirstLastTime", "Window", 0, 10, 0, 1e5, 1, 0.0,
                 InvalidationLevel::InvalidOutput, PropertySemantics::Text),
    renderLocation("RenderLocationDir", "Directory"),
    renderBaseName("RenderLocationBaseName", "Base Name"),
    writer("writer", "Type",
           [&]() {
               OptionPropertyState<FileExtension> state;
               const auto exts = util::getDataWriterFactory(app)->getExtensionsForType<Layer>();
               std::transform(
                   exts.begin(), exts.end(), std::back_inserter(state.options),
                   [](const auto& ext) -> OptionPropertyOption<FileExtension> { return ext; });
               auto it = std::find_if(exts.begin(), exts.end(),
                                      [&](auto& e) { return e.extension_ == "png"; });
               if (it != exts.end()) {
                   state.selectedIndex = std::distance(exts.begin(), it);
               }
               return state;
           }()),
    renderFPS("renderFPS", "Frames per Second", 24.0, {0.001, ConstraintBehavior::Immutable},
              {1000.0, ConstraintBehavior::Immutable}, 1.0, InvalidationLevel::InvalidOutput,
              PropertySemantics::Text),
    renderAction("renderAction", "Render"), renderActionStop("renderActionStop", "Stop"),
    controlOptions("controlOptions", "Special Tracks"),
    insertControlTrack("insertControlTrack", "Add Control Track",
                       [this]() { animation_->add(std::make_unique<ControlTrack>()); }),
    insertInvalidationTrack(
        "insertInvalidationTrack", "Add Invalidation Track",
        [this]() {
            animation_->add(std::make_unique<InvalidationTrack>(app_->getProcessorNetwork()));
        }),
    animation_(&animation), app_(app), state_(AnimationState::Paused), currentTime_(0),
    direction_(PlaybackDirection::Forward),
    timer_{std::chrono::duration_cast<std::chrono::milliseconds>(deltaTime()), [this] {
               if (state_ != AnimationState::Rendering) {
                   tick();
               }
           }} {

    //Play Settings
    playWindow.readonlyDependsOn(playWindowMode, [](auto& prop) { return prop.get() == 0; });

    playOptions.addProperties(playWindowMode, playWindow, framesPerSecond, playMode);
    playOptions.setCollapsed(true);

    framesPerSecond.onChange([this]() {
        timer_.setInterval(
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::abs(deltaTime())));
    });

    //Rendering Settings
    renderWindow.readonlyDependsOn(renderWindowMode, [](auto& prop) { return prop.get() == 0; });

    renderAction.onChange([&]() { render(); });
    renderActionStop.onChange([&]() { pause(); });
    renderAction.setReadOnly(state_ == AnimationState::Rendering);
    renderActionStop.setReadOnly(state_ != AnimationState::Rendering);

    renderOptions.addProperties(renderWindowMode, renderWindow, renderFPS, renderLocation,
                                renderBaseName, writer, renderAction, renderActionStop);
    renderOptions.setCollapsed(true);

    controlOptions.addProperties(insertControlTrack, insertInvalidationTrack);
    controlOptions.setCollapsed(true);

    addProperties(playOptions, renderOptions, controlOptions);
}

AnimationController::~AnimationController() = default;

void AnimationController::setState(AnimationState newState) {
    if (state_ == newState) return;

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
            timer_.stop();
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
    //No upper boundary check since you might want to set the time after the last keyframe of
    //animation when creating new ones
    if (currentTime_ != time) {
        auto oldTime = currentTime_;
        currentTime_ = std::max(Seconds(0), time);
        notifyTimeChanged(this, oldTime, currentTime_);
    }
}

void AnimationController::play() { setState(AnimationState::Playing); }

void AnimationController::pause() { setState(AnimationState::Paused); }

void AnimationController::stop() {
    setState(AnimationState::Paused);
    direction_ = PlaybackDirection::Forward;
    eval(currentTime_, Seconds(0));
}

void AnimationController::tick() {
    if (state_ != AnimationState::Playing) {
        setState(AnimationState::Paused);
        return;
    }

    //TODO: Implement fully working solution for this.
    //What to do when network cannot be evaluated in the speed that is given by deltaTime?
    //Initial solution: Don't care about that, and let it evaluate fully in the speed that it can
    //muster.
    auto newTime = currentTime_ + deltaTime();

    //Get active time window for playing
    //init with sub-window, overwrite with full window if necessary
    Seconds firstTime = Seconds(playWindow.get()[0]);
    Seconds lastTime = Seconds(playWindow.get()[1]);
    if (playWindowMode.get() == 0) {
        //Full animation window
        firstTime = animation_->getFirstTime();
        lastTime = animation_->getLastTime();
    }
    auto newState{state_};
    //Ping at the end of time
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
                newTime = lastTime + deltaTime();
                break;
            }
            default:
                break;
        }
    }

    //Pong at the beginning of time
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
                newTime = firstTime + deltaTime();
                break;
            }
            default:
                break;
        }
    }

    //Evaluate animation
    eval(currentTime_, newTime);

    //May be in paused state
    setState(newState);
}

void AnimationController::render() {
    auto start = std::chrono::high_resolution_clock::now();

    auto network = app_->getProcessorNetwork();
    std::optional<NetworkLock> lock{network};

    setState(AnimationState::Rendering);

    //Gather rendering info
    const Seconds firstTime =
        (renderWindowMode.get() == 0) ? animation_->getFirstTime() : Seconds(renderWindow.get()[0]);
    const Seconds lastTime =
        (renderWindowMode.get() == 0) ? animation_->getLastTime() : Seconds(renderWindow.get()[1]);
    const int numFrames =
        std::max(2, static_cast<int>((lastTime - firstTime) / Seconds{1.0 / renderFPS.get()}));

    //digits of the frame counter
    const int digits = [&]() {
        int d = 0;
        int number(numFrames - 1);
        while (number) {
            number /= 10;
            d++;
        }
        //use at least 4 digits, so we nicely overwrite the files from a previous test rendering
        //with less frames
        return std::max(d, 4);
    }();

    //Get all active canvases
    std::vector<std::pair<Exporter*, std::string>> exporters;
    network->forEachProcessor([&](Processor* p) {
        if (p->isSink()) {
            if (auto exporter = dynamic_cast<Exporter*>(p)) {
                auto base = renderBaseName.get();
                replaceInString(base, "UPN", p->getIdentifier());
                exporters.emplace_back(exporter, base);
            }
        }
    });

    //Switch Buttons
    renderAction.setReadOnly(true);
    renderActionStop.setReadOnly(false);

    util::OnScopeExit reset{[&]() {
        setState(AnimationState::Paused);
        renderAction.setReadOnly(false);
        renderActionStop.setReadOnly(true);
    }};

    //render frames
    for (int currentFrame = 0; currentFrame < numFrames; ++currentFrame) {
        //Evaluate animation
        Seconds newTime = firstTime + (lastTime - firstTime) / (numFrames - 1) * currentFrame;
        eval(currentTime_, newTime);

        lock.reset();

        while (app_->getProcessorNetwork()->runningBackgroundJobs() > 0) {
            app_->processFront();
            app_->processEvents();
            std::this_thread::yield();
        }
        app_->processEvents();

        for (auto&& [exporter, base] : exporters) {
            const auto file = fmt::format("{}{:0{}}", base, currentFrame, digits);
            exporter->exportFile(renderLocation.get(), file, {writer.get()}, Overwrite::Yes);
        }

        lock.emplace(network);

        if (state_ != AnimationState::Rendering) break;
    }

    using duration_double = std::chrono::duration<double, std::ratio<1>>;
    auto seconds = std::chrono::duration_cast<duration_double>(
                       std::chrono::high_resolution_clock::now() - start)
                       .count();
    LogInfo(fmt::format("Rendered {} frames in {:.3f} seconds, {:.3f} per frame", numFrames,
                        seconds, seconds / numFrames));
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

PlaybackDirection AnimationController::getPlaybackDirection() const { return direction_; }

void AnimationController::setPlaybackDirection(PlaybackDirection newDirection) {
    direction_ = newDirection;
}

Seconds AnimationController::deltaTime() const {
    if (direction_ == PlaybackDirection::Forward) {
        return Seconds{1.0 / framesPerSecond.get()};
    } else {
        return Seconds{-1.0 / framesPerSecond.get()};
    }
}

Seconds AnimationController::getCurrentTime() const { return currentTime_; }

}  //namespace animation

}  //namespace inviwo
