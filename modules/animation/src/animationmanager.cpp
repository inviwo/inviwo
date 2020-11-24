/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2020 Inviwo Foundation
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
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/properties/property.h>

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

void AnimationManager::registerPropertyTrackConnection(const std::string& propertyClassID,
                                                       const std::string& trackClassID) {
    trackFactory_.registerPropertyTrackConnection(propertyClassID, trackClassID);
}

}  // namespace animation

}  // namespace inviwo
