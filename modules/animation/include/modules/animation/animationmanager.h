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
#pragma once

#include <modules/animation/animationmoduledefine.h>  // for IVW_MODULE_ANIMATION_API

#include <modules/animation/factories/interpolationfactory.h>  // for InterpolationFactory
#include <modules/animation/factories/trackfactory.h>          // for TrackFactory
#include <modules/animation/factories/recorderfactories.h>

#include <string_view>  // for string_view
#include <vector>       // for vector

namespace inviwo {

class Deserializer;
class InviwoApplication;

namespace animation {

class Animation;

/**
 * The AnimationManager is responsible for managing the factories related to animations.
 *
 * The modules that wish to extend the Animation with a new functionality ( Track or Interpolation )
 * will do so through the AnimationSuppliers and will register those with the factories here.
 *
 * @see Animation
 * @see AnimationController
 * @see Track
 */
class IVW_MODULE_ANIMATION_API AnimationManager {
public:
    AnimationManager(InviwoApplication* app);
    virtual ~AnimationManager() = default;

    TrackFactory& getTrackFactory();
    const TrackFactory& getTrackFactory() const;

    InterpolationFactory& getInterpolationFactory();
    const InterpolationFactory& getInterpolationFactory() const;

    RecorderFactories& getRecorderFactories();
    const RecorderFactories& getRecorderFactories() const;

    /**
     * Register connection between a property and a track.
     * Used to create typed tracks for a property.
     * @param propertyClassID Property::getClassIdentifier
     * @param trackClassID PropertyTrack::getIdentifier()
     */
    void registerPropertyTrackConnection(std::string_view propertyClassID,
                                         std::string_view trackClassID);

    /**
     * Extracts each "Animation" in "Animations" in the Deserializer stream.
     * This AnimationManager is passed to the Animation constructor to enable its Property-based
     * adding convenience functions.
     * @note The animations are often tightly coupled to the processors in the workspace.
     * @param d stream
     * @return All animations found, empty .
     */
    std::vector<Animation> import(Deserializer& d);

private:
    InviwoApplication* app_;

    TrackFactory trackFactory_;
    InterpolationFactory interpolationFactory_;
    RecorderFactories recorders_;
};

}  // namespace animation

}  // namespace inviwo
