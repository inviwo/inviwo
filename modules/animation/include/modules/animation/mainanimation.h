/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2021 Inviwo Foundation
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

#include <inviwo/core/network/workspacemanager.h>

namespace inviwo {

namespace animation {
class WorkspaceAnimations;
/**
 * \brief Responsible for the main AnimationController and saving it in the workspace.
 *
 * This class is intented to be owned by WorkspaceAnimations, which is responsible for animations
 * stored in the workspace. The main Animation can only be changed through WorkspaceAnimations.
 *
 * The controller is cleared, saved, and loaded when the workspace is cleared, saved, or loaded.
 *
 * The MainAnimation also manages the ModuleCallback actions used to
 * create property tracks from the context menu of properties.
 *
 * This object is not intended to be used from within the Network, e.g. in a Processor, but rather
 * from the outside. See the for AnimationQt module for examples.
 *
 * @see AnimationWorkspaces
 * @see Animation
 * @see AnimationController
 * @see Track
 */
class IVW_MODULE_ANIMATION_API MainAnimation {
public:
    MainAnimation(InviwoApplication* app, Animation& animation);
    ~MainAnimation() = default;

    Animation& get();
    const Animation& get() const;
    AnimationController& getController();
    const AnimationController& getController() const;

private:
    // Only allow changes of main Animation from WorkspaceAnimations
    friend class WorkspaceAnimations;
    void set(Animation& animation);
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

    AnimationController controller_;

    WorkspaceManager::ClearHandle animationControllerClearHandle_;
    WorkspaceManager::SerializationHandle animationControllerSerializationHandle_;
    WorkspaceManager::DeserializationHandle animationControllerDeserializationHandle_;
};

}  // namespace animation

}  // namespace inviwo
