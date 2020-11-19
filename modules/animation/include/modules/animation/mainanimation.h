/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <modules/animation/datastructures/animation.h>
#include <modules/animation/animationcontroller.h>
#include <modules/animation/animationmanager.h>

#include <inviwo/core/network/workspacemanager.h>

namespace inviwo {

class AnimationModule;

namespace animation {

/**
 * \brief Responsible for the main Animation and AnimationController and saving it in the workspace.
 * It is responsible for clearing, saving, and loading the animation and its controller when
 * the workspace is cleared, saved, or loaded.
 * The MainAnimation also manages the ModuleCallback actions used to
 * create property tracks from the context menu of properties.
 *
 * This object is not intended to be used from within the Network, e.g. in a Processor, but rather
 * from the outside. See the for AnimationQt module for examples.
 *
 * @see Animation
 * @see AnimationController
 * @see Track
 */
class IVW_MODULE_ANIMATION_API MainAnimation {
public:
    MainAnimation(InviwoApplication* app, AnimationModule* animationModule,
                  AnimationManager& manager);
    ~MainAnimation() = default;

    Animation& getAnimation();
    const Animation& getAnimation() const;
    AnimationController& getAnimationController();
    const AnimationController& getAnimationController() const;

private:
    /**
     * Module callbacks must return void
     * @see addKeyframe(Property* property, Seconds time)
     */
    void addKeyframeCallback(Property* property);
    /**
     * Module callbacks must return void
     * @see addSequence(Property* property, Seconds time)
     */
    void addKeyframeSequenceCallback(Property* property);

    Animation animation_;
    AnimationController controller_;

    WorkspaceManager::ClearHandle animationClearHandle_;
    WorkspaceManager::SerializationHandle animationSerializationHandle_;
    WorkspaceManager::DeserializationHandle animationDeserializationHandle_;

    WorkspaceManager::ClearHandle animationControllerClearHandle_;
    WorkspaceManager::SerializationHandle animationControllerSerializationHandle_;
    WorkspaceManager::DeserializationHandle animationControllerDeserializationHandle_;
};

}  // namespace animation

}  // namespace inviwo
