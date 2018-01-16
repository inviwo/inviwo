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

#include <modules/animation/datastructures/controltrack.h>

namespace inviwo {

namespace animation {

ControlTrack::ControlTrack() : identifier_("control-track"), name_("Control Track") {}

ControlTrack::~ControlTrack() {
    while (size() > 0) {
        remove(size() - 1);
    }
}

std::string ControlTrack::classIdentifier() { return "org.inviwo.animation.controltrack"; }

std::string ControlTrack::getClassIdentifier() const { return classIdentifier(); }

size_t ControlTrack::getPriority() const { return priority_; }

void ControlTrack::setPriority(size_t priority) {
    if (priority_ != priority) {
        priority_ = priority;
        this->notifyPriorityChanged(this);
    }
}

const std::string& ControlTrack::getName() const { return name_; }

void ControlTrack::setName(const std::string& name) {
    if (name_ != name) {
        name_ = name;
        this->notifyNameChanged(this);
    }
}

const std::string& ControlTrack::getIdentifier() const { return identifier_; }

void ControlTrack::setIdentifier(const std::string& identifier) {
    if (identifier_ != identifier) {
        identifier_ = identifier;
        this->notifyIdentifierChanged(this);
    }
}

bool ControlTrack::isEnabled() const { return enabled_; }

void ControlTrack::setEnabled(bool enabled) { enabled_ = enabled; }

const ControlKeyframeSequence& ControlTrack::operator[](size_t i) const {
    return *sequences_[i];
}

ControlKeyframeSequence& ControlTrack::operator[](size_t i) {
    return *sequences_[i];
}

size_t ControlTrack::size() const { return sequences_.size(); }

/**
* Track of sequences
* ----------X======X====X-----------X=========X-------X=====X--------
* |- case 1-|-case 2----------------|-case 2----------|-case 2------|
*           |-case 2a---|-case 2b---|
*/
AnimationTimeState ControlTrack::operator()(Seconds from, Seconds to, AnimationState state) const {
    if (!enabled_ || sequences_.empty()) return {to, state};

	// We only consider keyframes if they are passed over whilst playing.
	if (state == AnimationState::Playing) {
		// 'it' will be the first seq. with a first time larger then 'to'.
		auto it = std::upper_bound(
			sequences_.begin(), sequences_.end(), to,
			[](const auto& time, const auto& seq) { return time < seq->getFirst().getTime(); });

		if (it != sequences_.begin()) {
			auto& seq1 = *std::prev(it);

			if (to < seq1->getLast().getTime()) {  // case 2a
				return (*seq1)(from, to, state);
			}
		}
	}

    return {to, state};
}

void ControlTrack::add(const KeyframeSequence& sequence) {
    addTyped(dynamic_cast<const ControlKeyframeSequence&>(sequence));
}

void ControlTrack::addTyped(const ControlKeyframeSequence& sequence) {
    auto it = std::upper_bound(
        sequences_.begin(), sequences_.end(), sequence.getFirst().getTime(),
        [](const auto& time, const auto& seq) { return time < seq->getFirst().getTime(); });

    if (it != sequences_.begin()) {
        if ((*std::prev(it))->getLast().getTime() > sequence.getFirst().getTime()) {
            throw Exception("Overlapping Sequence", IvwContext);
        }
    }
    if (it != sequences_.end() && (*it)->getFirst().getTime() < sequence.getLast().getTime()) {
        throw Exception("Overlapping Sequence", IvwContext);
    }

    auto inserted =
        sequences_.insert(it, std::make_unique<ControlKeyframeSequence>(sequence));
    this->notifyKeyframeSequenceAdded(this, inserted->get());
    (*inserted)->addObserver(this);
}

void ControlTrack::remove(size_t i) {
    auto seq = std::move(sequences_[i]);
    sequences_.erase(sequences_.begin() + i);
    this->notifyKeyframeSequenceRemoved(this, seq.get());
}

Seconds ControlTrack::lastTime() const {
    if (sequences_.empty()) {
        return Seconds{0.0};
    } else {
        return sequences_.back()->getLast().getTime();
    }
}

Seconds ControlTrack::firstTime() const {
    if (sequences_.empty()) {
        return Seconds{0.0};
    } else {
        return sequences_.front()->getFirst().getTime();
    }
}


std::vector<Seconds> ControlTrack::getAllTimes() const
{
	std::vector<Seconds> result;
	for (const auto& seq : sequences_) {
		for (size_t i = 0; i < seq->size(); i++) {
			result.push_back((*seq)[i].getTime());
		}
	}
	std::sort(result.begin(), result.end());
	return result;
}


void ControlTrack::onKeyframeSequenceMoved(KeyframeSequence* key) {
    std::stable_sort(sequences_.begin(), sequences_.end(), [](const auto& a, const auto& b) {
        return a->getFirst().getTime() < b->getFirst().getTime();
    });
    /// Do validation?

    auto it = std::find_if(sequences_.begin(), sequences_.end(),
                           [&](const auto& item) { return item.get() == key; });
    if (it != sequences_.end()) {
        if (it == sequences_.begin()) {
            this->notifyFirstMoved(this);
        } else if (it == std::prev(sequences_.end())) {
            this->notifyLastMoved(this);
        }
    }
}

void ControlTrack::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
    s.serialize("identifier", identifier_, SerializationTarget::Attribute);
    s.serialize("name", name_);
    s.serialize("enabled", enabled_);
    s.serialize("priority", priority_);
    s.serialize("sequences", sequences_, "sequence");
}

void ControlTrack::deserialize(Deserializer& d) {
    std::string className;
    d.deserialize("type", className, SerializationTarget::Attribute);
    if (className != getClassIdentifier()) {
        throw SerializationException(
            "Deserialized animation track: " + getClassIdentifier() +
                " from a serialized track with a different class identifier: " + className,
            IvwContext);
    }
    {
        auto old = identifier_;
        d.deserialize("identifier", identifier_, SerializationTarget::Attribute);
        if (old != identifier_) this->notifyIdentifierChanged(this);
    }
    {
        auto old = name_;
        d.deserialize("name", name_);
        if (old != name_) this->notifyNameChanged(this);
    }
    {
        auto old = enabled_;
        d.deserialize("enabled", enabled_);
        if (old != enabled_) this->notifyEnabledChanged(this);
    }
    {
        auto old = priority_;
        d.deserialize("priority", priority_);
        if (old != priority_) this->notifyPriorityChanged(this);
    }

    using Elem = std::unique_ptr<ControlKeyframeSequence>;
    util::IndexedDeserializer<Elem>("sequences", "sequence")
        .onNew([&](Elem& seq) {
            this->notifyKeyframeSequenceAdded(this, seq.get());
            seq->addObserver(this);
        })
        .onRemove([&](Elem& seq) { this->notifyKeyframeSequenceRemoved(this, seq.get()); })(
            d, sequences_);
}

}  // namespace animation

}  // namespace inviwo