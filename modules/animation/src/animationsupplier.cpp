/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2023 Inviwo Foundation
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

#include <modules/animation/animationsupplier.h>

#include <inviwo/core/common/inviwoapplication.h>                    // for InviwoApplication
#include <inviwo/core/util/exception.h>                              // for Exception
#include <inviwo/core/util/sourcecontext.h>                          // for IVW_CONTEXT_CUSTOM
#include <modules/animation/animationmanager.h>                      // for AnimationManager
#include <modules/animation/animationmodule.h>                       // for AnimationModule
#include <modules/animation/factories/interpolationfactory.h>        // for InterpolationFactory
#include <modules/animation/factories/interpolationfactoryobject.h>  // for InterpolationFactory...
#include <modules/animation/factories/trackfactory.h>                // for TrackFactory
#include <modules/animation/factories/trackfactoryobject.h>          // for TrackFactoryObject

#include <string_view>  // for string_view
#include <utility>      // for move

namespace inviwo {

namespace animation {

AnimationManager& getAnimationManager(InviwoApplication* app) {
    if (app) {
        if (auto animationmodule = app->getModuleByType<AnimationModule>()) {
            return animationmodule->getAnimationManager();
        }
    }
    throw Exception("Was not able to find the animation manager",
                    IVW_CONTEXT_CUSTOM("AnimationSupplier"));
}

AnimationSupplier::AnimationSupplier(AnimationManager& manager) : manager_(manager) {}

AnimationSupplier::AnimationSupplier(InviwoApplication* app) : manager_(getAnimationManager(app)) {}

AnimationSupplier::~AnimationSupplier() { unRegisterAll(); }

void AnimationSupplier::registerPropertyTrackConnection(const std::string& propertyClassID,
                                                        const std::string& trackClassID) {
    manager_.registerPropertyTrackConnection(propertyClassID, trackClassID);
}

void AnimationSupplier::unRegisterAll() {
    for (auto& elem : tracks_) {
        manager_.getTrackFactory().unRegisterObject(elem.get());
    }
    tracks_.clear();

    for (auto& elem : interpolations_) {
        manager_.getInterpolationFactory().unRegisterObject(elem.get());
    }
    interpolations_.clear();
}

void AnimationSupplier::registerTrackObject(std::unique_ptr<TrackFactoryObject> obj) {
    if (manager_.getTrackFactory().registerObject(obj.get())) {
        tracks_.push_back(std::move(obj));
    }
}

void AnimationSupplier::registerInterpolationObject(
    std::unique_ptr<InterpolationFactoryObject> obj) {

    if (manager_.getInterpolationFactory().registerObject(obj.get())) {
        interpolations_.push_back(std::move(obj));
    }
}

}  // namespace animation

}  // namespace inviwo
