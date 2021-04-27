/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <modules/animation/animationmoduledefine.h>
#include <modules/animation/datastructures/animation.h>
#include <modules/animation/animationcontroller.h>
#include <modules/animation/animationmanager.h>
#include <modules/animation/mainanimation.h>

#include <inviwo/core/network/workspacemanager.h>

namespace inviwo {

namespace animation {
/**
 * \brief Responsible for animations saved in the workspace and ensuring that there always is at
 * least one main Animation.
 *
 * One of its animations is set to be the MainAnimation.
 * Each Animation is associated with a name, but the name is not enforced to be unique.
 *
 * It is responsible for clearing, saving, and loading animations when the workspace is cleared,
 * saved, or loaded.
 *
 */
class IVW_MODULE_ANIMATION_API WorkspaceAnimations {
public:
    // Indices of changed elements [from, to], inclusive range
    using OnChangedDispatcher = Dispatcher<void(size_t, size_t)>;
    WorkspaceAnimations(InviwoApplication* app, AnimationManager& manager);
    virtual ~WorkspaceAnimations() = default;

    Animation& get(size_t index);
    /**
     * @return all animations matching name
     */
    std::vector<Animation*> get(std::string_view name);
    const std::vector<Animation>& get() const { return animations_; }

    Animation& operator[](size_t i);
    const Animation& operator[](size_t i) const;

    std::vector<Animation>::iterator begin();
    std::vector<Animation>::const_iterator begin() const;
    std::vector<Animation>::iterator end();
    std::vector<Animation>::const_iterator end() const;

    std::string_view getName(size_t index);
    Animation& add(std::string_view name);
    Animation& add(Animation anim);
    Animation& insert(size_t index, std::string_view name);
    /**
     * Removes  the animation and adjusts the main animation to the next available
     * index if needed.
     * An empty animation will be added if all animations are erased.
     */
    void erase(size_t index);
    /**
     * Removes all animations leaving an empty Animation, which is set to be the MainAnimation.
     * The size() will be 1 after clearing.
     */
    void clear();
    void setName(size_t index, std::string_view newName);

    size_t size() const { return animations_.size(); }

    /**
     * Appends found animations in the Deserializer stream.
     * @return the number of added animations.
     */
    size_t import(Deserializer& d);

    /**
     * @brief Set which of the added animations that should be MainAnimation.
     * The AnimationController in MainAnimation will notify its observers of the animation change.
     *
     * @throws std::out_of_range exception if index is greater than or equal to
     * WorkspaceAnimations::size
     */
    void setMainAnimationIndex(size_t index);
    size_t getMainAnimationIndex() const;
    MainAnimation& getMainAnimation();
    const MainAnimation& getMainAnimation() const;

    OnChangedDispatcher onChanged_;  // Fired when Animation is added/removed or name changed

private:
    AnimationManager& animationManager_;

    std::vector<Animation> animations_;
    MainAnimation mainAnimation_;
    size_t mainAnimationIdx_;

    WorkspaceManager::ClearHandle animationClearHandle_;
    WorkspaceManager::SerializationHandle animationSerializationHandle_;
    WorkspaceManager::DeserializationHandle animationDeserializationHandle_;

    WorkspaceManager::ClearHandle animationControllerClearHandle_;
    WorkspaceManager::SerializationHandle animationControllerSerializationHandle_;
    WorkspaceManager::DeserializationHandle animationControllerDeserializationHandle_;
};

}  // namespace animation

}  // namespace inviwo
