/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#include <modules/animation/animationmanager.h>
#include <modules/animation/animationmodule.h>

#include <inviwo/core/common/modulecallback.h>
#include <inviwo/core/common/moduleaction.h>

#include <modules/animation/datastructures/propertytrack.h>

namespace inviwo {

namespace animation {

AnimationManager::AnimationManager(InviwoApplication* app, AnimationModule* animationModule)
    : app_(app)
    , trackFactory_{}
    , interpolationFactory_{}
    , animation_{}
    , controller_{&animation_, app} {

    auto callbackAction = new ModuleCallbackAction("Add Key Frame", animationModule);

    callbackAction->getCallBack()->addMemberFunction(this, &AnimationManager::addTrackCallback);
    callbackAction->setActionState(ModuleCallBackActionState::Enabled);

    app->addCallbackAction(callbackAction);
}

TrackFactory& AnimationManager::getTrackFactory() { return trackFactory_; }

const TrackFactory& AnimationManager::getTrackFactory() const { return trackFactory_; }

InterpolationFactory& AnimationManager::getInterpolationFactory() { return interpolationFactory_; }

const InterpolationFactory& AnimationManager::getInterpolationFactory() const {
    return interpolationFactory_;
}

void AnimationManager::registerPropertyTrackConnection(const std::string& propertyClassID,
                                                       const std::string& trackClassID) {
    propertyToTrackMap_[propertyClassID] = trackClassID;
}

Animation& AnimationManager::getAnimation() { return animation_; }

const Animation& AnimationManager::getAnimation() const { return animation_; }

AnimationController& AnimationManager::getAnimationController() { return controller_; }

const AnimationController& AnimationManager::getAnimationController() const { return controller_; }

void AnimationManager::addTrackCallback(const Property* property) {
    auto tIt = trackMap_.find(property);
    if (tIt != trackMap_.end()) {
        tIt->second->addKeyFrameUsingPropertyValue(controller_.getCurrentTime());
    } else {

        auto it = propertyToTrackMap_.find(property->getClassIdentifier());
        if (it != propertyToTrackMap_.end()) {
            if (auto track = trackFactory_.create(it->second)) {
                if (auto baseTrackProperty = dynamic_cast<BasePropertyTrack*>(track.get())) {
                    baseTrackProperty->setProperty(const_cast<Property*>(property));
                    baseTrackProperty->addKeyFrameUsingPropertyValue(controller_.getCurrentTime());
                    animation_.add(std::move(track));
                    trackMap_[property] = baseTrackProperty;
                    return;
                }
            }
        }
        LogWarn("No matching Track found for property");
    }
}

}  // namespace

}  // namespace
