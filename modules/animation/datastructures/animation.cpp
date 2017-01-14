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

#include <modules/animation/datastructures/animation.h>

namespace inviwo {

namespace animation {

Animation::Animation() = default;

AniamtionTimeState Animation::operator()(Seconds from, Seconds to, AnimationState state) const {
    AniamtionTimeState ts{to, state};
    for (const auto& track : priorityTracks_) {
        ts = (*track)(from, ts.time, ts.state);
    }
    return ts;
}

bool Animation::empty() const {
    return tracks_.empty();
}

size_t Animation::size() const { return tracks_.size(); }

const Track& Animation::operator[](size_t i) const { return *tracks_[i]; }

Track& Animation::operator[](size_t i) { return *tracks_[i]; }

void Animation::add(std::unique_ptr<Track> track) {
    tracks_.push_back(std::move(track));
    priorityTracks_.push_back(tracks_.back().get());
    doPrioritySort();
    tracks_.back()->addObserver(this);
    notifyTrackAdded(tracks_.back().get());
}

void Animation::removeTrack(size_t i) {
    auto track = std::move(tracks_[i]);
    tracks_.erase(tracks_.begin() + i);
    util::erase_remove(priorityTracks_, track.get());
    notifyTrackRemoved(track.get());
}

void Animation::removeTrack(const std::string& id) {
    auto it = std::find_if(tracks_.begin(), tracks_.end(),
                           [&](const auto& track) { return track->getIdentifier() == id; });
    if (it != tracks_.end()) {
        removeTrack(std::distance(tracks_.begin(), it));
    }
}

void Animation::removeKeyframe(Keyframe* key) {
    for (size_t t = 0; t < tracks_.size(); ++t) {
        auto& track = *tracks_[t];
        for (size_t s = 0; s < track.size(); ++s) {
            auto& seq = track[s];
            for (size_t k = 0; k < seq.size(); ++k) {
                if (&seq[k] == key) {
                    if (seq.size() == 1) {
                        track.remove(s);
                    } else {
                        seq.remove(k);
                    }
                    return;
                }
            }
        }
    }
}

void Animation::clear() {
    while (!empty()) {
        removeTrack(tracks_.size() - 1);
    }
}

std::vector<Seconds> Animation::getAllTimes() const {
    
    std::vector<Seconds> result;

    for (size_t t = 0; t < tracks_.size(); ++t) {
        auto& track = *tracks_[t];
        for (size_t s = 0; s < track.size(); ++s) {
            auto& seq = track[s];
            for (size_t k = 0; k < seq.size(); ++k) {
                result.push_back(seq[k].getTime());
            }
        }
    }
    std::sort(result.begin(), result.end());
    return result;
}

Seconds Animation::firstTime() const {
    auto it = std::min_element(tracks_.begin(), tracks_.end(), [](const auto& a, const auto& b) {
        return a->firstTime() < b->firstTime();
    });
    if (it != tracks_.end()) {
        return (*it)->firstTime();
    } else {
        return Seconds{0.0};
    }
}

Seconds Animation::lastTime() const {
    auto it = std::max_element(tracks_.begin(), tracks_.end(), [](const auto& a, const auto& b) {
        return a->lastTime() < b->lastTime();
    });
    if (it != tracks_.end()) {
        return (*it)->lastTime();
    } else {
        return Seconds{0.0};
    }
}

void Animation::serialize(Serializer& s) const { s.serialize("tracks", tracks_, "track"); }

void Animation::deserialize(Deserializer& d) {
    util::IdentifiedDeserializer<std::string, std::unique_ptr<Track>>("tracks", "track")
        .setGetId([](const std::unique_ptr<Track>& t) { return t->getIdentifier(); })
        .setMakeNew([]() { return std::unique_ptr<Track>(); })
        .onNew([&](std::unique_ptr<Track>& t) { add(std::move(t)); })
        .onRemove([&](const std::string& id) { removeTrack(id); })(d, tracks_);
}

void Animation::onPriorityChanged(Track* t) { doPrioritySort(); }

void Animation::doPrioritySort() {
    std::stable_sort(
        priorityTracks_.begin(), priorityTracks_.end(),
        [](const auto& a, const auto& b) { return a->getPriority() > b->getPriority(); });
}

void Animation::onFirstMoved(Track* t) { notifyFirstMoved(); }

void Animation::onLastMoved(Track* t) { notifyLastMoved(); }

} // namespace

} // namespace

