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

#ifndef IVW_ANIMATIONMANAGER_H
#define IVW_ANIMATIONMANAGER_H

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/propertyownerobserver.h>

#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/core/network/processornetworkobserver.h>

#include <modules/animation/datastructures/animation.h>
#include <modules/animation/datastructures/animationobserver.h>
#include <modules/animation/animationcontroller.h>

#include <modules/animation/factories/interpolationfactory.h>
#include <modules/animation/factories/trackfactory.h>

namespace inviwo {

class InviwoApplication;
class AnimationModule;

namespace animation {

class BasePropertyTrack;

/**
 * The AnimationManager is responsible for managing the factories related to animations as well as
 * owning the currently used animation and controller. The AnumationSuppliers will register
 * FactoryObjects with the factories here.
 * The AnimationManager owns the current Animation and a AnimationController for that Animation, it 
 * is also responsible for clearing, saving, and loading the animation when ever the workspace is 
 * cleared, saved, or loaded. 
 * The AnimationManager also manages the ModuleCallback actions that are used to facilitate
 * the creation of property track from the context menu of properties. 
 * To be able to do this is has a map of track class identifiers to map to property class 
 * identifiers.
 */
class IVW_MODULE_ANIMATION_API AnimationManager : public AnimationObserver, 
                                                  public PropertyOwnerObserver,
                                                  public ProcessorNetworkObserver {
public:
    AnimationManager(InviwoApplication* app, AnimationModule* animationModule);
    virtual ~AnimationManager() = default;

    TrackFactory& getTrackFactory();
    const TrackFactory& getTrackFactory() const;

    InterpolationFactory& getInterpolationFactory();
    const InterpolationFactory& getInterpolationFactory() const;

    Animation& getAnimation();
    const Animation& getAnimation() const;
    AnimationController& getAnimationController();
    const AnimationController& getAnimationController() const;



    /**
     *	Register a Property Track class identifier for a property class identifier.
     */
    void registerPropertyTrackConnection(const std::string& propertyClassID,
                                         const std::string& trackClassID);


    /**
     *	Callback for the module action callbacks. 
     */
    void addKeyframeCallback(Property* property);
    /**
     *	Callback for the module action callbacks. 
     */
    void addSequenceCallback(Property* property);

private:
    BasePropertyTrack* addNewTrack(Property* property);


    // PropertyOwnerObserver overload
    virtual void onWillRemoveProperty(Property* property, size_t index) override;

    // AnimationObserver overload
    virtual void onTrackRemoved(Track* track) override;

    // ProcessorNetworkObserver overload
    virtual void onProcessorNetworkWillRemoveProcessor(Processor* processor) override;

    virtual void onTrackAdded(Track* track) override;

    InviwoApplication* app_;

    TrackFactory trackFactory_;
    InterpolationFactory interpolationFactory_;

    std::unordered_map<std::string, std::string> propertyToTrackMap_;
    std::unordered_map<const Property*, BasePropertyTrack*> trackMap_;

    Animation animation_;
    AnimationController controller_;

    WorkspaceManager::ClearHandle animationClearHandle_;
    WorkspaceManager::SerializationHandle animationSerializationHandle_;
    WorkspaceManager::DeserializationHandle animationDeserializationHandle_;
};

} // namespace

} // namespace

#endif // IVW_ANIMATIONMANAGER_H

