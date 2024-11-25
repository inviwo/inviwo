/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>  // for InviwoApplication
#include <inviwo/core/io/serialization/deserializer.h>
#include <inviwo/core/network/workspacemanager.h>              // for WorkspaceManager
#include <modules/animation/datastructures/animation.h>        // for Animation
#include <modules/animation/factories/interpolationfactory.h>  // for InterpolationFactory
#include <modules/animation/factories/trackfactory.h>          // for TrackFactory

#include <functional>  // for __base
#include <string>      // for basic_string

namespace inviwo {

namespace animation {

AnimationManager::AnimationManager(InviwoApplication* app)
    : app_(app), trackFactory_{app_->getProcessorNetwork()}, interpolationFactory_{} {

    app_->getWorkspaceManager()->registerFactory(&trackFactory_);
    app_->getWorkspaceManager()->registerFactory(&interpolationFactory_);
}

TrackFactory& AnimationManager::getTrackFactory() { return trackFactory_; }

const TrackFactory& AnimationManager::getTrackFactory() const { return trackFactory_; }

InterpolationFactory& AnimationManager::getInterpolationFactory() { return interpolationFactory_; }

const InterpolationFactory& AnimationManager::getInterpolationFactory() const {
    return interpolationFactory_;
}

RecorderFactories& AnimationManager::getRecorderFactories() { return recorders_; }

const RecorderFactories& AnimationManager::getRecorderFactories() const { return recorders_; }

void AnimationManager::registerPropertyTrackConnection(std::string_view propertyClassID,
                                                       std::string_view trackClassID) {
    trackFactory_.registerPropertyTrackConnection(propertyClassID, trackClassID);
}

std::vector<Animation> AnimationManager::import(Deserializer& d) {
    std::vector<Animation> animations;
    // Must pass AnimationManager to Animation constructor
    d.deserialize("Animations", animations, "Animation",
                  deserializer::IndexFunctions{.makeNew = [&]() { return Animation(this); }});

    return animations;
}

}  // namespace animation

}  // namespace inviwo
