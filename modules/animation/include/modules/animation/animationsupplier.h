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

#ifndef IVW_ANIMATIONSUPPLIER_H
#define IVW_ANIMATIONSUPPLIER_H

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/animation/animationmanager.h>

#include <modules/animation/factories/interpolationfactory.h>
#include <modules/animation/factories/interpolationfactoryobject.h>
#include <modules/animation/factories/trackfactory.h>
#include <modules/animation/factories/trackfactoryobject.h>

namespace inviwo {

class InviwoApplication;

namespace animation {

/**
 * \class AnimationSupplier
 * Base class to derive your module from if your module should add to the animation framework
 */
class IVW_MODULE_ANIMATION_API AnimationSupplier {
public:
    AnimationSupplier(AnimationManager& manager);
    AnimationSupplier(InviwoApplication* app);
    AnimationSupplier(const AnimationSupplier&) = delete;
    AnimationSupplier& operator=(const AnimationSupplier&) = delete;
    virtual ~AnimationSupplier();

    /**
     * Register a Track with the Track Factory
     */
    template <typename T>
    void registerTrack();

    /**
     * Register a Interpolation with the Interpolation Factory
     */
    template <typename T>
    void registerInterpolation();

    /**
     * Register connection between a property and a track.
     * Used to create typed tracks for a property.
     * @param propertyClassID Property::getClassIdentifier
     * @param trackClassID PropertyTrack::getIdentifier()
     * @see AnimationManager
     */
    void registerPropertyTrackConnection(const std::string& propertyClassID,
                                         const std::string& trackClassID);

    /**
     * Register connection between a property and an interpolation.
     * Used to get the preferred interpolation method for a property.
     * @param propertyClassID Property::getClassIdentifier
     * @param interpolationClassID Interpolation::getIdentifier()
     * @see AnimationManager
     */
    void registerPropertyInterpolationConnection(const std::string& propertyClassID,
                                                 const std::string& interpolationClassID);

    void unRegisterAll();

private:
    AnimationManager& manager_;
    std::vector<std::unique_ptr<TrackFactoryObject>> tracks_;
    std::vector<std::unique_ptr<InterpolationFactoryObject>> interpolations_;
};

template <typename T>
void AnimationSupplier::registerInterpolation() {
    auto interpolation = std::make_unique<InterpolationFactoryObjectTemplate<T>>();
    if (manager_.getInterpolationFactory().registerObject(interpolation.get())) {
        interpolations_.push_back(std::move(interpolation));
    }
}

template <typename T>
void AnimationSupplier::registerTrack() {
    auto track = std::make_unique<TrackFactoryObjectTemplate<T>>();
    if (manager_.getTrackFactory().registerObject(track.get())) {
        tracks_.push_back(std::move(track));
    }
}

}  // namespace animation

}  // namespace inviwo

#endif  // IVW_ANIMATIONSUPPLIER_H
