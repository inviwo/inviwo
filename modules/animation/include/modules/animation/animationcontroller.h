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
#pragma once

#include <modules/animation/animationmoduledefine.h>  // for IVW_MODULE_ANIMATION_API

#include <inviwo/core/common/inviwoapplicationutil.h>         // for getInviwoApplication
#include <inviwo/core/properties/buttonproperty.h>            // for ButtonProperty
#include <inviwo/core/properties/compositeproperty.h>         // for CompositeProperty
#include <inviwo/core/properties/directoryproperty.h>         // for DirectoryProperty
#include <inviwo/core/properties/minmaxproperty.h>            // for DoubleMinMaxProperty
#include <inviwo/core/properties/optionproperty.h>            // for OptionPropertyInt, OptionPr...
#include <inviwo/core/properties/ordinalproperty.h>           // for IntProperty, IntVec2Property
#include <inviwo/core/properties/ordinalrefproperty.h>        // for DoubleRefProperty
#include <inviwo/core/properties/propertyowner.h>             // for PropertyOwner
#include <inviwo/core/properties/stringproperty.h>            // for StringProperty
#include <inviwo/core/util/glmvec.h>                          // for ivec2
#include <inviwo/core/util/staticstring.h>                    // for operator+
#include <inviwo/core/util/timer.h>                           // for Timer
#include <modules/animation/animationcontrollerobserver.h>    // for AnimationControllerObservable
#include <modules/animation/datastructures/animationstate.h>  // for PlaybackMode, AnimationState
#include <modules/animation/datastructures/animationtime.h>   // for Seconds

#include <functional>   // for __base
#include <string>       // for operator==, string
#include <string_view>  // for operator==
#include <vector>       // for operator!=, vector, operator==

namespace inviwo {
class Deserializer;
class InviwoApplication;
class Serializer;

namespace animation {
class Animation;

/** The AnimationController is responsible for steering the animation.
 *
 *   It keeps track of the animation time and state.
 *   When playing, it should adjust the step sizes to maintain a certain playback speed (frames per
 *  second).
 *
 *   Furthermore, it allows to render the animation into an image sequence.
 */
class IVW_MODULE_ANIMATION_API AnimationController : public AnimationControllerObservable,
                                                     public PropertyOwner {
public:
    AnimationController(Animation& animation,
                        InviwoApplication* app = util::getInviwoApplication());
    virtual ~AnimationController();

    /// Play animation
    void play();
    /// Pause animation
    void pause();
    /// Render the animation into an image sequence
    void render();
    // Pause and reset to start
    void stop();

    void setState(AnimationState newState);

    /// Advances the animation to the next time step in playing state.
    void tick();

    /// Asks the animation to update the network to reflect the new time.
    void eval(Seconds oldTime, Seconds newTime);

    void setAnimation(Animation& animation);

    /// Returns mutable controlled animation.
    Animation& getAnimation();

    /// Returns controlled animation.
    const Animation& getAnimation() const;

    /// Returns the current state of the controller, whether it is playing, or pausing, and such.
    const AnimationState& getState() const;

    /// Returns the playback direction used during tick.
    PlaybackDirection getPlaybackDirection() const;
    void setPlaybackDirection(PlaybackDirection newDirection);

    Seconds getCurrentTime() const;

    Seconds deltaTime() const;

    InviwoApplication* getInviwoApplication() override { return app_; }

public:
    CompositeProperty playOptions;
    OptionPropertyInt playWindowMode;
    DoubleMinMaxProperty playWindow;
    DoubleProperty framesPerSecond;
    OptionProperty<PlaybackMode> playMode;

    CompositeProperty renderOptions;
    OptionPropertyInt renderWindowMode;
    DoubleMinMaxProperty renderWindow;
    DirectoryProperty renderLocation;
    StringProperty renderBaseName;
    OptionProperty<FileExtension> writer;
    DoubleProperty renderFPS;
    ButtonProperty renderAction;
    ButtonProperty renderActionStop;

    CompositeProperty controlOptions;
    ButtonProperty insertControlTrack;
    ButtonProperty insertInvalidationTrack;

protected:
    /// Low-level setting of currentTime_. Use eval() to set time in the public interface.
    void setTime(Seconds time);

    /// The animation to control, non-owning reference.
    Animation* animation_;

    /// Host application
    InviwoApplication* app_;

    /// State of the animation, such as paused or playing or rendering
    AnimationState state_;

    /// Current time of the animation. This is an important variable to keep consistent!
    Seconds currentTime_;

    PlaybackDirection direction_;

    /// Timer for calling the tick function is regular intervals.
    Timer timer_;
};

}  // namespace animation

}  // namespace inviwo
