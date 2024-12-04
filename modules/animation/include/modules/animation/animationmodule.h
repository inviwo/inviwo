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

#include <inviwo/core/common/inviwomodule.h>                // for InviwoModule
#include <inviwo/core/io/serialization/ticpp.h>             // for TxElement
#include <inviwo/core/io/serialization/versionconverter.h>  // for VersionConverter
#include <modules/animation/animationmanager.h>             // for AnimationManager
#include <modules/animation/animationsupplier.h>            // for AnimationSupplier
#include <modules/animation/demo/democontroller.h>          // for DemoController
#include <modules/animation/workspaceanimations.h>          // for WorkspaceAnimations
#include <modules/animation/datastructures/propertytrack.h>

#include <memory>  // for unique_ptr

namespace inviwo {
class InviwoApplication;
namespace animation {
class MainAnimation;
}  // namespace animation

class IVW_MODULE_ANIMATION_API AnimationModule : public InviwoModule,
                                                 public animation::AnimationSupplier {
public:
    AnimationModule(InviwoApplication* app);
    virtual ~AnimationModule();

    virtual int getVersion() const override;
    virtual std::unique_ptr<VersionConverter> getConverter(int version) const override;
    /*
     * \brief Returns WorkspaceAnimations, which keeps track of animations stored in the workspace.
     * One of the animations is set to be the MainAnimation, which can be changed from the GUI (see
     * AnimationQtModule).
     */
    animation::WorkspaceAnimations& getWorkspaceAnimations();
    const animation::WorkspaceAnimations& getWorkspaceAnimations() const;
    /*
     * Returns the AnimationController and its associated Animation intended to be used outside of
     * the Network, e.g. by the AnimationQtModule. The main Animation can only be changed through
     * WorkspaceAnimations.
     * @see getWorkspaceAnimations
     */
    animation::MainAnimation& getMainAnimation();
    const animation::MainAnimation& getMainAnimation() const;

    animation::AnimationManager& getAnimationManager();
    const animation::AnimationManager& getAnimationManager() const;

    animation::DemoController& getDemoController();
    const animation::DemoController& getDemoController() const;

private:
    template <typename PropertyType, typename Interpolation>
    void interpolationHelper() {
        // No need to add existing interpolation method. Will produce a warning if adding a
        // duplicate
        if (!manager_.getInterpolationFactory().hasKey(Interpolation::classIdentifier())) {
            registerInterpolation<Interpolation>();
        }
    }
    template <typename PropertyType,
              typename Keyframe = animation::ValueKeyframe<typename PropertyType::value_type>,
              typename KeyframeSeq = animation::KeyframeSequenceTyped<Keyframe>>
    void propertyHelper() {
        using namespace animation;
        // Register PropertyTrack and the KeyFrame it should use
        registerTrack<PropertyTrack<PropertyType, Keyframe, KeyframeSeq>>();
        registerPropertyTrackConnection(
            PropertyTraits<PropertyType>::classIdentifier(),
            PropertyTrack<PropertyType, Keyframe, KeyframeSeq>::classIdentifier());
    }

    class Converter : public VersionConverter {
    public:
        Converter(int version) : version_(version) {}
        virtual bool convert(TxElement* root) override;

    private:
        int version_;
    };
    animation::AnimationManager manager_;
    animation::WorkspaceAnimations
        animations_;  /// Used by Animation Editor and stored with workspace.
    animation::DemoController demoController_;
};

}  // namespace inviwo
