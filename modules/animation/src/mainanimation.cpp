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

#include <modules/animation/mainanimation.h>
#include <modules/animation/animationmodule.h>
#include <modules/animation/workspaceanimations.h>
#include <inviwo/core/common/modulecallback.h>
#include <inviwo/core/common/moduleaction.h>

namespace inviwo {

namespace animation {

MainAnimation::MainAnimation(InviwoApplication* app,
                             Animation& animation)
    : controller_{animation, app} {
    AnimationModule* animationModule = app->getModuleByType<AnimationModule>();
    {
        auto callbackAction = new ModuleCallbackAction("Add Key Frame", animationModule,
                                                       ModuleCallBackActionState::Enabled);

        callbackAction->getCallBack().addMemberFunction(this, &MainAnimation::addKeyframeCallback);
        app->addCallbackAction(callbackAction);
    }
    {
        auto callbackAction = new ModuleCallbackAction("Add Sequence", animationModule,
                                                       ModuleCallBackActionState::Enabled);
        callbackAction->getCallBack().addMemberFunction(
            this, &MainAnimation::addKeyframeSequenceCallback);
        app->addCallbackAction(callbackAction);
    }

    animationControllerClearHandle_ =
        app->getWorkspaceManager()->onClear([&]() { controller_.resetAllPoperties(); });
    animationControllerSerializationHandle_ = app->getWorkspaceManager()->onSave(
        [&](Serializer& s) { s.serialize("AnimationController", controller_); });
    animationControllerDeserializationHandle_ = app->getWorkspaceManager()->onLoad(
        [&](Deserializer& d) { d.deserialize("AnimationController", controller_); });
}

void MainAnimation::set(Animation& animation) { controller_.setAnimation(animation); }

Animation& MainAnimation::get() { return controller_.getAnimation(); }

const Animation& MainAnimation::get() const { return controller_.getAnimation(); }

AnimationController& MainAnimation::getController() { return controller_; }

const AnimationController& MainAnimation::getController() const { return controller_; }

void MainAnimation::addKeyframeCallback(Property* property) {
    controller_.getAnimation().addKeyframe(property, controller_.getCurrentTime());
}

void MainAnimation::addKeyframeSequenceCallback(Property* property) {
    controller_.getAnimation().addKeyframeSequence(property, controller_.getCurrentTime());
}

}  // namespace animation

}  // namespace inviwo
