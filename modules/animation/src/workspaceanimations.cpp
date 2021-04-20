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

#include <modules/animation/workspaceanimations.h>

#include <inviwo/core/util/zip.h>

namespace inviwo {

namespace animation {

WorkspaceAnimations::WorkspaceAnimations(InviwoApplication* app, AnimationManager& animationManager)
    : animationManager_{animationManager}
    , animations_{{Animation(&animationManager_)}}
    , names_{{"Animation 1"}}
    , mainAnimation_(app, animations_.front())
    , mainAnimationIdx_{0} {

    animationClearHandle_ =
        app->getWorkspaceManager()->onClear([&]() { erase(0, animations_.size()); });
    animationSerializationHandle_ = app->getWorkspaceManager()->onSave([&](Serializer& s) {
        s.serialize("MainAnimationIndex", mainAnimationIdx_);
        s.serialize("Animations", animations_, "Animation");
        s.serialize("AnimationNames", names_);
    });
    animationDeserializationHandle_ = app->getWorkspaceManager()->onLoad([&](Deserializer& d) {
        size_t mainAnimation = 0;
        erase(0, animations_.size());
        d.deserialize("MainAnimationIndex", mainAnimation);
        d.deserialize("AnimationNames", names_);

        // Must pass AnimationManager to Animation constructor
        util::IndexedDeserializer<Animation>("Animations", "Animation").setMakeNew([&]() {
            return Animation(&animationManager_);
        })(d, animations_);

        // Failsafe
        if (animations_.empty()) {
            names_.clear();
            add("Animation 1");
            setMainAnimationIndex(0);
        } else {
            setMainAnimationIndex(mainAnimation);
            auto idx = static_cast<size_t>(animations_.size() - 1);
            onChanged_.invoke(0, idx);
        }
    });
}

Animation& animation::WorkspaceAnimations::get(size_t index) { return animations_.at(index); }

std::vector<Animation*> WorkspaceAnimations::get(std::string_view name) {
    std::vector<Animation*> animations;
    for (auto&& [ind, elem] : util::enumerate(names_)) {
        if (elem == name) {
            animations.push_back(&animations_[ind]);
        }
    }
    return animations;
}

std::string_view WorkspaceAnimations::getName(size_t index) { return names_.at(index); }

Animation& WorkspaceAnimations::add(std::string_view name) {
    animations_.emplace_back(Animation(&animationManager_));
    names_.emplace_back(name);
    auto idx = static_cast<size_t>(animations_.size() - 1);
    onChanged_.invoke(idx, idx);
    return animations_.back();
}

Animation& WorkspaceAnimations::add(std::string_view name, Animation anim) {
    animations_.emplace_back(anim);
    names_.emplace_back(name);
    auto idx = static_cast<size_t>(animations_.size() - 1);
    onChanged_.invoke(idx, idx);
    return animations_.back();
}

Animation& WorkspaceAnimations::insert(size_t index, std::string_view name) {
    if (index <= getMainAnimationIndex()) {
        mainAnimationIdx_ = index + 1;
    }
    animations_.insert(animations_.begin() + index, Animation(&animationManager_));
    names_.insert(names_.begin() + index, std::string(name));
    onChanged_.invoke(index, index);
    return animations_[index];
}

void WorkspaceAnimations::erase(size_t from, size_t to) {
    auto numToRemove = to - from;

    // First ensure that the MainAnimation is valid
    if (numToRemove == animations_.size()) {
        add("Animation 1");
        mainAnimation_.set(animations_.back());
        mainAnimationIdx_ = 0;  // all before will be removed
    } else if (getMainAnimationIndex() >= to) {
        // numToRemove before will be removed, but the animation itself will not change
        mainAnimationIdx_ -= numToRemove;
    } else if (getMainAnimationIndex() >= from) {
        // The selected one will be removed, select next after
        auto indexBeforeRemoval = to < animations_.size() ? to : from - 1;
        mainAnimation_.set(animations_[indexBeforeRemoval]);
        mainAnimationIdx_ = static_cast<size_t>(
            std::clamp(static_cast<int>(getMainAnimationIndex()) - static_cast<int>(numToRemove), 0,
                       static_cast<int>(animations_.size()) - 1));
    }

    animations_.erase(animations_.begin() + from, animations_.begin() + to);
    names_.erase(names_.begin() + from, names_.begin() + to);
    onChanged_.invoke(from, to - 1);
}

void WorkspaceAnimations::setName(size_t index, std::string_view newName) {
    if (index < names_.size()) {
        names_[index] = newName;
        onChanged_.invoke(index, index);
    }
}

void WorkspaceAnimations::setMainAnimationIndex(size_t index) {
    Animation& anim = animations_.at(mainAnimationIdx_);
    mainAnimationIdx_ = index;
    mainAnimation_.set(anim);
}

size_t WorkspaceAnimations::getMainAnimationIndex() const { return mainAnimationIdx_; }

MainAnimation& WorkspaceAnimations::getMainAnimation() { return mainAnimation_; }

const MainAnimation& WorkspaceAnimations::getMainAnimation() const { return mainAnimation_; }

size_t WorkspaceAnimations::import(Deserializer& d) {
    std::vector<Animation> animations;
    std::vector<std::string> names;
    d.deserialize("AnimationNames", names);

    // Must pass AnimationManager to Animation constructor
    util::IndexedDeserializer<Animation>("Animations", "Animation").setMakeNew([&]() {
        return Animation(&animationManager_);
    })(d, animations);
    for (auto&& [name, anim] : util::zip(names, animations)) {
        add(name, anim);
    }
    return animations.size();
}

}  // namespace animation

}  // namespace inviwo
