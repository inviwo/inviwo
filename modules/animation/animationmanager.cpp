/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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
#include <inviwo/core/network/processornetwork.h>

#include <modules/animation/datastructures/propertytrack.h>

namespace inviwo {

namespace animation {

AnimationManager::AnimationManager(InviwoApplication* app, AnimationModule* animationModule)
    : app_(app)
    , trackFactory_{}
    , interpolationFactory_{}
    , animation_{}
    , controller_{&animation_, app} {

    {
        auto callbackAction = new ModuleCallbackAction("Add Key Frame", animationModule,
                                                       ModuleCallBackActionState::Enabled);

        callbackAction->getCallBack().addMemberFunction(this,
                                                        &AnimationManager::addKeyframeCallback);
        app->addCallbackAction(callbackAction);
    }
    {
        auto callbackAction = new ModuleCallbackAction("Add Sequence", animationModule,
                                                       ModuleCallBackActionState::Enabled);
        callbackAction->getCallBack().addMemberFunction(this,
                                                        &AnimationManager::addSequenceCallback);
        app->addCallbackAction(callbackAction);
    }

    app_->getWorkspaceManager()->registerFactory(&trackFactory_);
    app_->getWorkspaceManager()->registerFactory(&interpolationFactory_);

    app_->getProcessorNetwork()->addObserver(this);
    animation_.addObserver(this);

    animationClearHandle_ = app_->getWorkspaceManager()->onClear([&]() { animation_.clear(); });
    animationSerializationHandle_ = app_->getWorkspaceManager()->onSave(
        [&](Serializer& s) { s.serialize("Animation", animation_); });
    animationDeserializationHandle_ = app_->getWorkspaceManager()->onLoad(
        [&](Deserializer& d) { d.deserialize("Animation", animation_); });
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

void AnimationManager::addKeyframeCallback(Property* property) {
    auto it = trackMap_.find(property);
    if (it != trackMap_.end()) {
        it->second->addKeyFrameUsingPropertyValue(controller_.getCurrentTime());
    } else if (auto basePropertyTrack = addNewTrack(property)) {
        basePropertyTrack->addKeyFrameUsingPropertyValue(controller_.getCurrentTime());
    } else {
        LogWarn("No matching Track found for property \"" + property->getIdentifier() + "\"");
    }
}

void AnimationManager::addSequenceCallback(Property* property) {
    auto it = trackMap_.find(property);
    if (it != trackMap_.end()) {
        it->second->addSequenceUsingPropertyValue(controller_.getCurrentTime());
    } else if (auto basePropertyTrack = addNewTrack(property)) {
        basePropertyTrack->addKeyFrameUsingPropertyValue(controller_.getCurrentTime());
    } else {
        LogWarn("No matching Track found for property \"" + property->getIdentifier() + "\"");
    }
}

BasePropertyTrack* AnimationManager::addNewTrack(Property* property) {
    auto it = propertyToTrackMap_.find(property->getClassIdentifier());
    if (it != propertyToTrackMap_.end()) {
        if (auto track = trackFactory_.create(it->second)) {
            if (auto basePropertyTrack = dynamic_cast<BasePropertyTrack*>(track.get())) {
                basePropertyTrack->setProperty(const_cast<Property*>(property));
                animation_.add(std::move(track)); // Callback will add track to trackMap_           
                property->getOwner()->addObserver(this);
                return basePropertyTrack; 
            }
        }
    }
    return nullptr;
}

void AnimationManager::onWillRemoveProperty(Property* property, size_t index) {
    auto it = trackMap_.find(property);
    if (it != trackMap_.end()) {
        animation_.removeTrack(it->second->getIdentifier());
    }
}

void AnimationManager::onTrackRemoved(Track* track) {
    util::map_erase_remove_if(trackMap_, [&](const auto& elem) {
        return elem.second->toTrack() == track;
    });
}

void AnimationManager::onProcessorNetworkWillRemoveProcessor(Processor* processor) {
    std::vector<std::string> toRemove;  // Save id to remove to avoid invalidating iterators.
    util::map_erase_remove_if(trackMap_, [&](const auto& elem) {
        if (elem.first->getOwner()->getProcessor() == processor) {
            toRemove.push_back(elem.second->getIdentifier());
            return true;
        } else {
            return false;
        }
    });

    for (const auto& item : toRemove) {
        animation_.removeTrack(item);
    }
}

void AnimationManager::onTrackAdded(Track* track) {
    if (auto basePropertyTrack = dynamic_cast<BasePropertyTrack*>(track)) {
        trackMap_[basePropertyTrack->getProperty()] = basePropertyTrack;
    }
}

}  // namespace

}  // namespace
