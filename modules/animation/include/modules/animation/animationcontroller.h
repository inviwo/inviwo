/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2026 Inviwo Foundation
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

#include <modules/animation/animationmoduledefine.h>

#include <inviwo/core/common/inviwoapplicationutil.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/ordinalrefproperty.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/staticstring.h>
#include <inviwo/core/util/timer.h>
#include <modules/animation/animationcontrollerobserver.h>
#include <modules/animation/datastructures/animationstate.h>
#include <modules/animation/datastructures/animationtime.h>

#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace inviwo {
class Deserializer;
class InviwoApplication;
class Serializer;

namespace animation {
class Animation;
class AnimationManager;
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
    AnimationController(Animation& animation, AnimationManager& manager,
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

    /*
     * Set current time to that of the previous keyframe, if there is one.
     * Equal to eval(getCurrentTime(), prevKeyframeTime).
     * Does not alter current PlaybackDirection.
     */
    void jumpToPrevKeyframe();
    /*
     * Set current time to that of the next keyframe, if there is one.
     * Equal to eval(getCurrentTime(), nextKeyframeTime).
     * Does not alter current PlaybackDirection.
     */
    void jumpToNextKeyframe();

    /*
     * Set current time to that of the previous ControlTrack keyframe, if there is one.
     * Equal to eval(getCurrentTime(), prevKeyframeTime).
     * Does not alter current PlaybackDirection.
     */
    void jumpToPrevControlKeyframe();
    /*
     * Set current time to that of the next ControlTrack keyframe, if there is one.
     * Equal to eval(getCurrentTime(), nextKeyframeTime).
     * Does not alter current PlaybackDirection.
     */
    void jumpToNextControlKeyframe();

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

    DoubleProperty renderFPS;
    ButtonProperty renderAction;
    ButtonProperty renderActionStop;

    BoolCompositeProperty exportOptions_;
    DirectoryProperty exportOutputDirectory_;
    StringProperty exportBaseName_;
    OptionProperty<FileExtension> exportWriter_;
    BoolProperty exportOverwrite_;

    CompositeProperty controlOptions;
    ButtonProperty insertControlTrack;
    ButtonProperty insertInvalidationTrack;

protected:
    /// Low-level setting of currentTime_. Use eval() to set time in the public interface.
    void setTime(Seconds time);

    /// The animation to control, non-owning reference.
    Animation* animation_;

    AnimationManager* manager_;

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
