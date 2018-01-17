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

#include <modules/animation/datastructures/controlkeyframesequence.h>

namespace inviwo {

namespace animation {

bool operator==(const ControlKeyframeSequence& a, const ControlKeyframeSequence& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

bool operator!=(const ControlKeyframeSequence& a, const ControlKeyframeSequence& b) {
    return !(a == b);
}

ControlKeyframeSequence::ControlKeyframeSequence() : KeyframeSequence(), keyframes_() {}

ControlKeyframeSequence::ControlKeyframeSequence(const std::vector<ControlKeyframe>& keyframes)
    : KeyframeSequence(), keyframes_() {
    for (const auto& key : keyframes) {
        keyframes_.push_back(std::make_unique<ControlKeyframe>(key));
    }
}

ControlKeyframeSequence::ControlKeyframeSequence(const ControlKeyframeSequence& rhs)
    : KeyframeSequence(rhs) {
    for (const auto& key : rhs.keyframes_) {
        addKeyFrame(std::make_unique<ControlKeyframe>(*key));
    }
}

ControlKeyframeSequence& ControlKeyframeSequence::operator=(const ControlKeyframeSequence& that) {
    if (this != &that) {
        KeyframeSequence::operator=(that);

        while (keyframes_.size() > 0) {
            auto key = std::move(keyframes_.back());
            keyframes_.pop_back();
            notifyKeyframeRemoved(key.get(), this);
        }

        for (const auto& key : that.keyframes_) {
            keyframes_.push_back(std::make_unique<ControlKeyframe>(*key));
            notifyKeyframeAdded(keyframes_.back().get(), this);
        }
    }
    return *this;
}

ControlKeyframeSequence::~ControlKeyframeSequence() {
    while (size() > 0) {
        // Remove and notify that keyframe is removed.
        remove(size() - 1);
    }
}

void ControlKeyframeSequence::onKeyframeTimeChanged(Keyframe* key, Seconds oldTime) {
    const auto startTime = keyframes_.front()->getTime();
    const auto endTime = keyframes_.back()->getTime();

    std::stable_sort(keyframes_.begin(), keyframes_.end(),
                     [](const auto& a, const auto& b) { return a->getTime() < b->getTime(); });
    if (startTime != keyframes_.front()->getTime() || endTime != keyframes_.back()->getTime()) {
        notifyKeyframeSequenceMoved(this);
    }
}

AnimationTimeState ControlKeyframeSequence::operator()(Seconds from, Seconds to,
                                                       AnimationState state) const {
	AnimationTimeState timeState{ to, state };
	
	if (state == AnimationState::Playing) {
		for (const auto& key : keyframes_) {
			auto t = key->getTime();
			if (from < t && t < to || to < t && t < from) {
				timeState = (*key)(from, to, state);
			}
		}
	}

	return timeState;
}

void ControlKeyframeSequence::add(const Keyframe& key) {
    add(dynamic_cast<const ControlKeyframe&>(key));
}

void ControlKeyframeSequence::add(const ControlKeyframe& key) {
    addKeyFrame(std::make_unique<ControlKeyframe>(key));
}

void ControlKeyframeSequence::addKeyFrame(std::unique_ptr<ControlKeyframe> key) {
    auto it = keyframes_.insert(std::upper_bound(keyframes_.begin(), keyframes_.end(), key,
                                                 [&key](const auto& a, const auto& b) {
                                                     return a->getTime() < b->getTime();
                                                 }),
                                std::move(key));

    (*it)->addObserver(this);
    notifyKeyframeAdded(it->get(), this);
}

void ControlKeyframeSequence::remove(size_t i) {

    auto key = std::move(keyframes_[i]);
    keyframes_.erase(keyframes_.begin() + i);
    notifyKeyframeRemoved(key.get(), this);
}

ControlKeyframe& ControlKeyframeSequence::getLast() { return *keyframes_.back(); }

const ControlKeyframe& ControlKeyframeSequence::getLast() const { return *keyframes_.back(); }

ControlKeyframe& ControlKeyframeSequence::getFirst() { return *keyframes_.front(); }

const ControlKeyframe& ControlKeyframeSequence::getFirst() const { return *keyframes_.front(); }

ControlKeyframe& ControlKeyframeSequence::operator[](size_t i) { return *keyframes_[i]; }

const ControlKeyframe& ControlKeyframeSequence::operator[](size_t i) const {
    return *keyframes_[i];
}

void ControlKeyframeSequence::serialize(Serializer& s) const {
    s.serialize("keyframes", keyframes_, "keyframe");
}

void ControlKeyframeSequence::deserialize(Deserializer& d) {
    //static_assert(false);
    /*
using Elem = std::unique_ptr<Key>;
util::IndexedDeserializer<Elem>("keyframes", "keyframe")
    .onNew([&](Elem& key) {
        notifyKeyframeAdded(key.get(), this);
        key->addObserver(this);
    })
    .onRemove([&](Elem& key) { notifyKeyframeRemoved(key.get(), this); })(d, keyframes_);
            */
}

}  // namespace animation

}  // namespace inviwo
