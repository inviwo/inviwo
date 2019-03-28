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

#include <modules/animation/datastructures/animation.h>

namespace inviwo {

namespace animation {

Animation::Animation() = default;

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
    tracks_.push_back(std::move(track));
    priorityTracks_.push_back(tracks_.back().get());
    doPrioritySort();
    tracks_.back()->addObserver(this);
    notifyTrackAdded(tracks_.back().get());
}

std::unique_ptr<Track> Animation::remove(size_t i) {
    auto track = std::move(tracks_[i]);
    tracks_.erase(tracks_.begin() + i);
    util::erase_remove(priorityTracks_, track.get());
    notifyTrackRemoved(track.get());
    return track;
}

std::unique_ptr<Track> Animation::remove(const std::string& id) {
    auto it = std::find_if(tracks_.begin(), tracks_.end(),
                           [&](const auto& track) { return track->getIdentifier() == id; });
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

void Animation::serialize(Serializer& s) const { s.serialize("tracks", tracks_, "track"); }

void Animation::deserialize(Deserializer& d) {
    util::IdentifiedDeserializer<std::string, std::unique_ptr<Track>>("tracks", "track")
        .setGetId([](const std::unique_ptr<Track>& t) { return t->getIdentifier(); })
        .setMakeNew([]() { return std::unique_ptr<Track>(); })
        .onNew([&](std::unique_ptr<Track>& t) { add(std::move(t)); })
        .onRemove([&](const std::string& id) { remove(id); })(d, tracks_);
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
