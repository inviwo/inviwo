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

#include <modules/animation/animationcontrollerobserver.h>
#include <modules/animation/animationcontroller.h>
#include <modules/animation/datastructures/animation.h>

namespace inviwo {

namespace animation {

void AnimationControllerObservable::notifyStateChanged(AnimationController* controller,
                                                       AnimationState oldState,
                                                       AnimationState newState) {
    forEachObserver(
        [&](AnimationControllerObserver* o) { o->onStateChanged(controller, oldState, newState); });
}

void AnimationControllerObservable::notifyPlaybackSettingsChanged(
    AnimationController* controller, AnimationPlaySettings prevSettings,
    AnimationPlaySettings newSettings) {
    forEachObserver([&](AnimationControllerObserver* o) {
        o->onPlaybackSettingsChanged(controller, prevSettings, newSettings);
    });
}

void AnimationControllerObservable::notifyTimeChanged(AnimationController* controller,
                                                      Seconds oldtime, Seconds newTime) {
    forEachObserver(
        [&](AnimationControllerObserver* o) { o->onTimeChanged(controller, oldtime, newTime); });
}

void AnimationControllerObservable::notifyAnimationChanged(AnimationController* controller,
                                                           Animation* oldAnim, Animation* newAnim) {
    forEachObserver([&](AnimationControllerObserver* o) {
        o->onAnimationChanged(controller, oldAnim, newAnim);
    });
}

}  // namespace animation

}  // namespace inviwo
