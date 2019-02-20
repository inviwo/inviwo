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
    , controller_{animation_, app} {

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

    animationControllerClearHandle_ =
        app_->getWorkspaceManager()->onClear([&]() { controller_.resetAllPoperties(); });
    animationControllerSerializationHandle_ = app_->getWorkspaceManager()->onSave(
        [&](Serializer& s) { s.serialize("AnimationController", controller_); });
    animationControllerDeserializationHandle_ = app_->getWorkspaceManager()->onLoad(
        [&](Deserializer& d) { d.deserialize("AnimationController", controller_); });
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

void AnimationManager::registerPropertyInterpolationConnection(
    const std::string& propertyClassID, const std::string& interpolationClassID) {
    propertyToInterpolationMap_.emplace(propertyClassID, interpolationClassID);
}

Animation& AnimationManager::getAnimation() { return animation_; }

const Animation& AnimationManager::getAnimation() const { return animation_; }

AnimationController& AnimationManager::getAnimationController() { return controller_; }

const AnimationController& AnimationManager::getAnimationController() const { return controller_; }

void AnimationManager::addKeyframeCallback(Property* property) {
    addKeyframeCallback(property, controller_.getCurrentTime());
}

void AnimationManager::addKeyframeCallback(Property* property, Seconds time) {
    auto it = trackMap_.find(property);
    try {
        auto interpolation = getDefaultInterpolation(property);
        if (it != trackMap_.end()) {
            // Note: interpolation will only be used if a new sequence is created.
            it->second->addKeyFrameUsingPropertyValue(time, std::move(interpolation));
        } else if (auto basePropertyTrack = addNewTrack(property)) {
            basePropertyTrack->addKeyFrameUsingPropertyValue(time, std::move(interpolation));
        } else {
            LogWarn("No matching Track found for property \"" + property->getIdentifier() + "\"");
        }
    } catch (const Exception& ex) {
        // No interpolation method registered?
        LogError(ex.getMessage());
    }
}

void AnimationManager::addSequenceCallback(Property* property) {
    addSequenceCallback(property, controller_.getCurrentTime());
}

void AnimationManager::addSequenceCallback(Property* property, Seconds time) {
    auto it = trackMap_.find(property);
    try {
        auto interpolation = getDefaultInterpolation(property);
        if (it != trackMap_.end()) {
            it->second->addSequenceUsingPropertyValue(time, std::move(interpolation));
        } else if (auto basePropertyTrack = addNewTrack(property)) {
            basePropertyTrack->addKeyFrameUsingPropertyValue(time, std::move(interpolation));
        } else {
            LogWarn("No matching Track found for property \"" + property->getIdentifier() + "\"");
        }
    } catch (const Exception& ex) {
        // No interpolation method registered?
        LogError(ex.getMessage());
    }
}

BasePropertyTrack* AnimationManager::addNewTrack(Property* property) {
    auto it = propertyToTrackMap_.find(property->getClassIdentifier());
    if (it != propertyToTrackMap_.end()) {
        if (auto track = trackFactory_.create(it->second)) {
            if (auto basePropertyTrack = dynamic_cast<BasePropertyTrack*>(track.get())) {
                try {
                    basePropertyTrack->setProperty(const_cast<Property*>(property));
                } catch (const Exception& e) {
                    LogWarn(e.getMessage() << " Invalid property class identified?") return nullptr;
                }
                animation_.add(std::move(track));  // Callback will add track to trackMap_
                property->getOwner()->addObserver(this);
                return basePropertyTrack;
            }
        }
    }
    return nullptr;
}

std::unique_ptr<Interpolation> AnimationManager::getDefaultInterpolation(Property* property) {
    // Check if there is an interpolation associated with this property
    auto interpolationIt = propertyToInterpolationMap_.find(property->getClassIdentifier());
    std::unique_ptr<Interpolation> interpolation(nullptr);
    if (interpolationIt != propertyToInterpolationMap_.end()) {
        interpolation = interpolationFactory_.create(interpolationIt->second);
        if (!interpolation) {
            throw Exception(
                "Default interpolation method for " + property->getClassIdentifier() +
                " was registered but the interpolation method was not added to the "
                "interpolation factory. @Developer: Please follow examples in animationmodule.cpp");
        }
    } else {
        throw Exception(
            "No interpolation method for " + property->getClassIdentifier() +
            " was registered. @Developer: Please follow examples in animationmodule.cpp");
    }
    return interpolation;
}

const std::unordered_multimap<std::string, std::string>& AnimationManager::getInterpolationMapping()
    const {
    return propertyToInterpolationMap_;
}

void AnimationManager::onWillRemoveProperty(Property* property, size_t) {
    auto it = trackMap_.find(property);
    if (it != trackMap_.end()) {
        animation_.remove(it->second->getIdentifier());
    }
}

void AnimationManager::onTrackRemoved(Track* track) {
    util::map_erase_remove_if(trackMap_,
                              [&](const auto& elem) { return elem.second->toTrack() == track; });
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
        animation_.remove(item);
    }
}

void AnimationManager::onTrackAdded(Track* track) {
    if (auto basePropertyTrack = dynamic_cast<BasePropertyTrack*>(track)) {
        trackMap_[basePropertyTrack->getProperty()] = basePropertyTrack;
    }
}

}  // namespace animation

}  // namespace inviwo
