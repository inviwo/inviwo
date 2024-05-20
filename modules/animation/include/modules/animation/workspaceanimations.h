/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2024 Inviwo Foundation
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

#include <modules/animation/animationmoduledefine.h>  // for IVW_MODULE_ANIMAT...

#include <inviwo/core/network/workspacemanager.h>  // for WorkspaceManager
#include <inviwo/core/util/dispatcher.h>           // for Dispatcher
#include <inviwo/core/util/indirectiterator.h>
#include <modules/animation/animationcontrollerobserver.h>              // for AnimationControll...
#include <modules/animation/datastructures/animation.h>                 // for Animation
#include <modules/animation/datastructures/animationobserver.h>         // for AnimationObserver
#include <modules/animation/datastructures/animationtime.h>             // for Seconds
#include <modules/animation/datastructures/keyframeobserver.h>          // for KeyframeObserver
#include <modules/animation/datastructures/keyframesequenceobserver.h>  // for KeyframeSequenceO...
#include <modules/animation/datastructures/trackobserver.h>             // for TrackObserver
#include <modules/animation/mainanimation.h>                            // for MainAnimation

#include <cstddef>      // for size_t
#include <string_view>  // for string_view
#include <vector>       // for vector, vector<>:...

namespace inviwo {

class AnimationModule;
class InviwoApplication;

namespace animation {
class AnimationController;
class AnimationManager;
class Keyframe;
class KeyframeSequence;
class Track;

/**
 * \brief Responsible for animations saved in the workspace and ensuring that there always is at
 * least one main Animation.
 *
 * One of its animations is set to be the MainAnimation. The Animation used by MainAnimation must
 * be set through WorkspaceAnimations.
 *
 *
 * WorkspaceAnimations is responsible for clearing, saving, and loading animations when the
 * workspace is cleared, saved, or loaded. It will also call notifyObserversProcessorNetworkChanged
 * whenever its animations change to enable undo/redo.
 *
 */
class IVW_MODULE_ANIMATION_API WorkspaceAnimations : public AnimationControllerObserver,
                                                     public AnimationObserver,
                                                     public TrackObserver,
                                                     public KeyframeSequenceObserver,
                                                     public KeyframeObserver {
public:
    /**
     * Called when an Animation is added or removed.
     * In case of removal: The supplied Animation will be removed from WorkspaceAnimation before
     * this function is called, but it will not be deleted until after this call.
     * @param Index of changed Animation (as it was before removal)
     * @param Reference to the added/removed Animation.
     */
    using OnChangedDispatcher = Dispatcher<void(size_t, Animation&)>;
    WorkspaceAnimations(InviwoApplication* app, AnimationManager& manager, AnimationModule& module);
    WorkspaceAnimations(const WorkspaceAnimations&) = delete;
    WorkspaceAnimations(WorkspaceAnimations&&) = delete;
    WorkspaceAnimations& operator=(const WorkspaceAnimations&) = delete;
    WorkspaceAnimations& operator=(WorkspaceAnimations&&) = delete;
    virtual ~WorkspaceAnimations();

    Animation& get(size_t index);
    /**
     * @return all animations matching name
     */
    std::vector<Animation*> get(std::string_view name);

    Animation& operator[](size_t i);
    const Animation& operator[](size_t i) const;

    using const_iterator =
        util::IndirectIterator<typename std::vector<std::unique_ptr<Animation>>::const_iterator>;
    using iterator =
        util::IndirectIterator<typename std::vector<std::unique_ptr<Animation>>::iterator>;

    const_iterator begin() const;
    const_iterator end() const;
    iterator begin();
    iterator end();

    /**
     * Add an empty Animation with specified name.
     */
    Animation& add(std::string_view name);
    Animation& add(Animation anim);

    Animation& insert(size_t index, std::string_view name);
    Animation& insert(const_iterator position, std::string_view name);
    /**
     * Removes  the animation and adjusts the main animation to the next available
     * index if needed.
     * An empty animation will be added if all animations are erased.
     */
    void erase(size_t index);
    void erase(const_iterator position);
    /**
     * Removes all animations leaving an empty Animation, which is set to be the MainAnimation.
     * The size() will be 1 after clearing.
     */
    void clear();

    size_t size() const { return animations_.size(); }

    /**
     * @brief Set specified Animation to be the MainAnimation.
     * A copy of the Animation will be added in case the it has not been added before.
     * The AnimationController in MainAnimation will notify its observers of the animation change.
     */
    void setMainAnimation(Animation& anim);
    MainAnimation& getMainAnimation();
    const MainAnimation& getMainAnimation() const;

    const_iterator find(const Animation* anim) const;
    iterator find(const Animation* anim);

    OnChangedDispatcher onChanged_;  // Fired when animations are added/removed

private:
    void onAnyChange();

    size_t getMainAnimationIndex() const;
    // MainAnimation changed
    virtual void onAnimationChanged(AnimationController* controller, Animation* oldAnim,
                                    Animation* newAnim) override;

    // Add observers and fire onChanged_
    void addInternal(typename std::vector<std::unique_ptr<Animation>>::iterator animation);

    // AnimationObserver overloads
    virtual void onTrackAdded(Track*) override;
    virtual void onTrackRemoved(Track*) override;
    virtual void onNameChanged(Animation*) override;

    // TrackObserver overloads, notify network that they it is invalid for undo/redo
    virtual void onKeyframeSequenceAdded(Track*, KeyframeSequence*) override;
    virtual void onKeyframeSequenceRemoved(Track*, KeyframeSequence*) override;

    virtual void onEnabledChanged(Track*) override;
    virtual void onNameChanged(Track*) override;
    virtual void onPriorityChanged(Track*) override;

    // KeyframeSequenceObserver overloads
    virtual void onKeyframeAdded(Keyframe*, KeyframeSequence*) override;
    virtual void onKeyframeRemoved(Keyframe*, KeyframeSequence*) override;
    virtual void onKeyframeSequenceMoved(KeyframeSequence*) override;
    virtual void onKeyframeSequenceSelectionChanged(KeyframeSequence*) override;

    // KeyframeObserver
    virtual void onKeyframeTimeChanged(Keyframe*, Seconds) override;
    virtual void onKeyframeSelectionChanged(Keyframe*) override;

    AnimationManager& animationManager_;

    std::vector<std::unique_ptr<Animation>> animations_;
    MainAnimation mainAnimation_;

    InviwoApplication* app_;

    WorkspaceManager::ClearHandle animationClearHandle_;
    WorkspaceManager::SerializationHandle animationSerializationHandle_;
    WorkspaceManager::DeserializationHandle animationDeserializationHandle_;

    WorkspaceManager::ClearHandle animationControllerClearHandle_;
    WorkspaceManager::SerializationHandle animationControllerSerializationHandle_;
    WorkspaceManager::DeserializationHandle animationControllerDeserializationHandle_;
};

}  // namespace animation

}  // namespace inviwo
