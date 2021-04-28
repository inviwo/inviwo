/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2021 Inviwo Foundation
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

#include <modules/animation/datastructures/animation.h>
#include <modules/animation/datastructures/propertytrack.h>
#include <modules/animation/animationmanager.h>

#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

namespace animation {

Animation::Animation(AnimationManager* am, std::string_view name)
    : AnimationObservable(), TrackObserver(), PropertyOwnerObserver(), name_(name), am_(am) {}

Animation::Animation(const Animation& other)
    : AnimationObservable(other)
    , TrackObserver(other)
    , PropertyOwnerObserver(other)
    , name_{other.name_}
    , am_{other.am_} {
    for (const auto& tr : other.tracks_) {
        add(std::unique_ptr<Track>(tr->clone()));
    }
}

Animation& Animation::operator=(const Animation& that) {
    if (this != &that) {
        while (!tracks_.empty()) {
            remove(tracks_.back().get());
        }
        name_ = that.name_;
        am_ = that.am_;
        for (const auto& tr : that.tracks_) {
            add(std::unique_ptr<Track>(tr->clone()));
        }
    }
    return *this;
}

AnimationTimeState Animation::operator()(Seconds from, Seconds to, AnimationState state) const {
    AnimationTimeState ts{to, state};
    for (const auto& track : priorityTracks_) {
        ts = (*track)(from, ts.time, ts.state);
    }
    return ts;
}

bool Animation::empty() const { return tracks_.empty(); }

size_t Animation::size() const { return tracks_.size(); }

const Track& Animation::operator[](size_t i) const { return *tracks_[i]; }

Track& Animation::operator[](size_t i) { return *tracks_[i]; }

auto Animation::begin() -> iterator { return util::makeIndirectIterator<true>(tracks_.begin()); }

auto Animation::begin() const -> const_iterator {
    return util::makeIndirectIterator<true>(tracks_.begin());
}

auto Animation::end() -> iterator { return util::makeIndirectIterator<true>(tracks_.end()); }

auto Animation::end() const -> const_iterator {
    return util::makeIndirectIterator<true>(tracks_.end());
}

void Animation::add(std::unique_ptr<Track> track) {
    if (!track) {
        return;
    }
    tracks_.push_back(std::move(track));
    trackAddedInternal(tracks_.back().get());
}

Keyframe* Animation::addKeyframe(Property* property, Seconds time) {
    auto it = findTrack(property);
    try {
        if (it != end()) {
            return dynamic_cast<BasePropertyTrack*>(&(*it))->addKeyFrameUsingPropertyValue(time);
        } else if (auto basePropertyTrack = add(property)) {
            return basePropertyTrack->addKeyFrameUsingPropertyValue(time);
        } else {
            LogWarn("No matching Track found for property \"" << property->getIdentifier() << "\"");
        }
    } catch (const Exception& ex) {
        LogError(ex.getMessage());
    }
    return nullptr;
}

KeyframeSequence* Animation::addKeyframeSequence(Property* property, Seconds time) {
    auto it = findTrack(property);
    std::string interpolationErrMsg;
    try {
        if (it != end()) {
            return dynamic_cast<BasePropertyTrack*>(&(*it))->addSequenceUsingPropertyValue(time);
        } else if (auto basePropertyTrack = add(property)) {
            basePropertyTrack->addKeyFrameUsingPropertyValue(time);
            return &basePropertyTrack->toTrack()->getFirst();
        } else {
            LogWarn("No matching Track found for property \"" << property->getIdentifier() << "\"");
        }
    } catch (const Exception& ex) {
        LogError(ex.getMessage());
    }
    return nullptr;
}

Animation::iterator Animation::findTrack(Property* property) {
    return std::find_if(begin(), end(), [property](auto& track) {
        if (auto pTrack = dynamic_cast<BasePropertyTrack*>(&track)) {
            return pTrack->getProperty() == property;
        } else {
            return false;
        }
    });
}

BasePropertyTrack* Animation::add(Property* property) {
    auto it = findTrack(property);
    if (it != end()) {
        return dynamic_cast<BasePropertyTrack*>(&(*it));
    } else {
        if (auto track = getManager()->getTrackFactory().create(property)) {
            if (auto basePropertyTrack = dynamic_cast<BasePropertyTrack*>(track.get())) {
                try {
                    basePropertyTrack->setProperty(property);
                } catch (const Exception& e) {
                    LogWarn(e.getMessage() << " Invalid property class identified?") return nullptr;
                }
                add(std::move(track));
                return basePropertyTrack;
            }
        }
        return nullptr;
    }
}

std::unique_ptr<Track> Animation::remove(size_t i) {
    auto track = std::move(tracks_[i]);
    tracks_.erase(tracks_.begin() + i);
    trackRemovedInternal(track.get());
    return track;
}

std::unique_ptr<Track> Animation::remove(Track* track) {
    auto it = std::find_if(tracks_.begin(), tracks_.end(),
                           [&](const auto& t) { return t.get() == track; });
    if (it != tracks_.end()) {
        return remove(std::distance(tracks_.begin(), it));
    } else {
        return nullptr;
    }
}

std::unique_ptr<Keyframe> Animation::remove(Keyframe* key) {
    for (auto& track : tracks_) {
        if (auto res = track->remove(key)) {
            return res;
        }
    }
    return nullptr;
}

std::unique_ptr<KeyframeSequence> Animation::remove(KeyframeSequence* seq) {
    for (auto& track : tracks_) {
        if (auto res = track->remove(seq)) {
            return res;
        }
    }
    return nullptr;
}

void Animation::clear() {
    while (!empty()) {
        remove(tracks_.size() - 1);
    }
}

std::vector<Seconds> Animation::getAllTimes() const {
    std::vector<Seconds> result;

    for (auto& track : tracks_) {
        auto times = track->getAllTimes();
        result.insert(result.end(), times.begin(), times.end());
    }
    std::sort(result.begin(), result.end());
    return result;
}

Seconds Animation::getFirstTime() const {
    Seconds time{0};

    auto it = std::min_element(tracks_.begin(), tracks_.end(), [](const auto& a, const auto& b) {
        return a->getFirstTime() < b->getFirstTime();
    });
    if (it != tracks_.end()) {
        time = (*it)->getFirstTime();
    }
    return time;
}

Seconds Animation::getLastTime() const {
    Seconds time{0};
    auto it = std::max_element(tracks_.begin(), tracks_.end(), [](const auto& a, const auto& b) {
        return a->getLastTime() < b->getLastTime();
    });
    if (it != tracks_.end()) {
        time = (*it)->getLastTime();
    }
    return time;
}

std::string_view inviwo::animation::Animation::getName() const { return name_; }

void inviwo::animation::Animation::setName(std::string_view name) {
    if (name != name_) {
        name_ = name;
        notifyNameChanged(this);
    }
}

void Animation::serialize(Serializer& s) const {
    s.serialize("name", name_);
    s.serialize("tracks", tracks_, "track");
}

void Animation::deserialize(Deserializer& d) {
    d.deserialize("name", name_);
    util::IndexedDeserializer<std::unique_ptr<Track>>("tracks", "track")
        .onNew([&](std::unique_ptr<Track>& track) { trackAddedInternal(track.get()); })
        .onRemove([&](std::unique_ptr<Track>& track) { trackRemovedInternal(track.get()); })(
            d, tracks_);
}

void Animation::onWillRemoveProperty(Property* property, size_t) {
    auto it = findTrack(property);
    if (it != end()) {
        remove(std::distance(tracks_.begin(), it.base()));
    }
}

void Animation::trackAddedInternal(Track* track) {
    priorityTracks_.push_back(track);
    doPrioritySort();
    track->addObserver(this);
    if (auto propertyTrack = dynamic_cast<BasePropertyTrack*>(track)) {
        if (auto propertyOwner = propertyTrack->getProperty()->getOwner()) {
            propertyOwner->addObserver(this);
        }
    }
    notifyTrackAdded(track);
}

void Animation::trackRemovedInternal(Track* track) {
    util::erase_remove(priorityTracks_, track);
    if (auto propertyTrack = dynamic_cast<BasePropertyTrack*>(track);
        auto owner = propertyTrack->getProperty()->getOwner()) {
        // Only stop observing property owner if no other track needs it
        auto sameOwnerIt = std::find_if(tracks_.begin(), tracks_.end(), [owner](const auto& elem) {
            if (auto ptrck = dynamic_cast<BasePropertyTrack*>(elem.get())) {
                return ptrck->getProperty()->getOwner() == owner;
            } else {
                return false;
            }
        });
        if (sameOwnerIt == tracks_.end()) {
            owner->removeObserver(this);
        }
    }
    notifyTrackRemoved(track);
}

AnimationManager* Animation::getManager() {
    if (am_) {
        return am_;
    } else {
        throw Exception("AnimationManager must be set to use this functionality", IVW_CONTEXT);
    }
}

void Animation::onPriorityChanged(Track*) { doPrioritySort(); }

void Animation::doPrioritySort() {
    std::stable_sort(
        priorityTracks_.begin(), priorityTracks_.end(),
        [](const auto& a, const auto& b) { return a->getPriority() > b->getPriority(); });
}

void Animation::onFirstMoved(Track*) { notifyFirstMoved(); }

void Animation::onLastMoved(Track*) { notifyLastMoved(); }

}  // namespace animation

}  // namespace inviwo
