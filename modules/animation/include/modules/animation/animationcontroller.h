/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>

namespace inviwo {

namespace animation {

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
    AnimationController(Animation& animation, InviwoApplication* app = InviwoApplication::getPtr());
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

    /// Advances the animation to the next time step in rendering state.
    void tickRender();

    /// Asks the animation to update the network to reflect the new time.
    void eval(Seconds oldTime, Seconds newTime);

    void setAnimation(Animation& animation);
    void setPlaySpeed(double framesPerSecond);

    /// Returns mutable controlled animation.
    Animation& getAnimation();

    /// Returns controlled animation.
    const Animation& getAnimation() const;

    /// Returns the current state of the controller, whether it is playing, or pausing, and such.
    const AnimationState& getState() const;

    /// Returns playback mode such as loop or swing and such.
    const AnimationPlaySettings& getPlaybackSettings() const { return settingsPlay_; }
    AnimationPlaySettings& getPlaybackSettings() { return settingsPlay_; }
    void setPlaybackSettings(const AnimationPlaySettings& newSettings);

    const AnimationPlaySettings& getRenderingSettings() const { return settingsRendering_; }
    Seconds getCurrentTime() const;

    InviwoApplication* getInviwoApplication() { return app_; }

    CompositeProperty playOptions;
    OptionPropertyInt playWindowMode;
    DoubleMinMaxProperty playWindow;
    DoubleProperty framesPerSecond;
    TemplateOptionProperty<PlaybackMode> playMode;

    CompositeProperty renderOptions;
    OptionPropertyInt renderWindowMode;
    DoubleMinMaxProperty renderWindow;
    OptionPropertyInt renderSizeMode;
    IntVec2Property renderSize;
    OptionPropertyInt renderAspectRatio;
    DirectoryProperty renderLocation;
    StringProperty renderBaseName;
    OptionPropertyString renderImageExtension;
    IntProperty renderNumFrames;
    ButtonProperty renderAction;
    ButtonProperty renderActionStop;

    CompositeProperty controlOptions;
    ButtonProperty controlInsertPauseFrame;

protected:
    /// Low-level setting of currentTime_. Use eval() to set time in the public interface.
    void setTime(Seconds time);

    /// Called to cleanup after rendering
    void afterRender();

    /// The animation to control, non-owning reference.
    Animation* animation_;

    /// Host application
    InviwoApplication* app_;

    /// State of the animation, such as paused or playing or rendering
    AnimationState state_;

    /// If in playback state, how fast we play the animation and whether we loop, or swing, etc.
    AnimationPlaySettings settingsPlay_;

    /// If in rendering state, how many frames we render.
    AnimationPlaySettings settingsRendering_;

    /// Current time of the animation. This is an important variable to keep consistent!
    Seconds currentTime_;

    /// Time span between two frames.
    Seconds deltaTime_;

    /// Timer for calling the tick function is regular intervals.
    Timer timer_;

    struct RenderCanvasSize {
        RenderCanvasSize() = default;
        std::string canvasIdentifier;
        bool enableCustomInputDimensions_{false};
        ivec2 customInputDimensions_{0};
        bool keepAspectRatio_{true};
    };

    /// Data structure for the state needed during rendering
    struct RenderState {
        Seconds firstTime{0};
        Seconds lastTime{0};
        int numFrames{0};
        int currentFrame{0};
        int digits{0};
        std::string baseFileName;
        std::vector<RenderCanvasSize> origCanvasSettings;
        std::string canvasIndicator;
    };

    /// State needed during rendering
    RenderState renderState_;
};

}  // namespace animation

}  // namespace inviwo

#endif  // IVW_ANIMATIONCONTROLLER_H
