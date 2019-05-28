/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#ifndef IVW_BASETRACK_H
#define IVW_BASETRACK_H

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/indirectiterator.h>

#include <modules/animation/datastructures/track.h>
#include <modules/animation/datastructures/keyframesequence.h>
#include <modules/animation/datastructures/keyframesequenceobserver.h>

namespace inviwo {

namespace animation {

template <typename Seq>
class BaseTrack : public Track, public KeyframeSequenceObserver {
public:
    using seq_type = Seq;
    using key_type = typename Seq::key_type;
    using value_type = typename Seq::value_type;
    using iterator = util::IndirectIterator<typename std::vector<std::unique_ptr<Seq>>::iterator>;
    using const_iterator =
        util::IndirectIterator<typename std::vector<std::unique_ptr<Seq>>::const_iterator>;

    static_assert(std::is_base_of<KeyframeSequence, Seq>::value,
                  "Seq has to derive from KeyframeSequence");

    BaseTrack(const std::string& identifier, const std::string& name, size_t priority = 0);
    virtual ~BaseTrack();

    virtual bool isEnabled() const override;
    virtual void setEnabled(bool enabled) override;

    virtual const std::string& getIdentifier() const override;
    virtual void setIdentifier(const std::string& identifier) override;

    virtual const std::string& getName() const override;
    virtual void setName(const std::string& name) override;

    virtual size_t getPriority() const override;
    virtual void setPriority(size_t priority) override;

    virtual Seconds getFirstTime() const override;
    virtual Seconds getLastTime() const override;
    virtual std::vector<Seconds> getAllTimes() const override;

    virtual size_t size() const override;
    virtual bool empty() const override;

    virtual Seq& operator[](size_t i) override;
    virtual const Seq& operator[](size_t i) const override;

    virtual const Seq& getFirst() const override;
    virtual Seq& getFirst() override;
    virtual const Seq& getLast() const override;
    virtual Seq& getLast() override;

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

    virtual void add(Seconds time, bool asNewSequence) override;
    virtual void add(std::unique_ptr<KeyframeSequence> sequence) override;
    virtual void add(std::unique_ptr<Seq> sequence);

    virtual std::unique_ptr<KeyframeSequence> remove(size_t i) override;
    virtual std::unique_ptr<Keyframe> remove(Keyframe* key) override;
    virtual std::unique_ptr<KeyframeSequence> remove(KeyframeSequence* seq) override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

protected:
    virtual void onKeyframeSequenceMoved(KeyframeSequence* seq) override;
    void addToClosestSequence(std::unique_ptr<key_type> key);

    bool enabled_{true};
    std::string identifier_;
    std::string name_;
    size_t priority_{0};

    // Sorted list of non-overlapping sequences of key frames
    std::vector<std::unique_ptr<Seq>> sequences_;
};

template <typename Seq>
BaseTrack<Seq>::BaseTrack(const std::string& identifier, const std::string& name, size_t priority)
    : identifier_{identifier}, name_{name}, priority_{priority} {}

template <typename Seq>
BaseTrack<Seq>::~BaseTrack() {
    while (size() > 0) {
        remove(size() - 1);
    }
}

template <typename Seq>
bool BaseTrack<Seq>::isEnabled() const {
    return enabled_;
}
template <typename Seq>
void BaseTrack<Seq>::setEnabled(bool enabled) {
    if (enabled_ != enabled) {
        enabled_ = enabled;
        this->notifyEnabledChanged(this);
    }
}

template <typename Seq>
const std::string& BaseTrack<Seq>::getIdentifier() const {
    return identifier_;
}
template <typename Seq>
void BaseTrack<Seq>::setIdentifier(const std::string& identifier) {
    if (identifier_ != identifier) {
        identifier_ = identifier;
        this->notifyIdentifierChanged(this);
    }
}

template <typename Seq>
const std::string& BaseTrack<Seq>::getName() const {
    return name_;
}
template <typename Seq>
void BaseTrack<Seq>::setName(const std::string& name) {
    if (name_ != name) {
        name_ = name;
        this->notifyNameChanged(this);
    }
}

template <typename Seq>
size_t BaseTrack<Seq>::getPriority() const {
    return priority_;
}
template <typename Seq>
void BaseTrack<Seq>::setPriority(size_t priority) {
    if (priority_ != priority) {
        priority_ = priority;
        this->notifyPriorityChanged(this);
    }
}

template <typename Seq>
Seconds BaseTrack<Seq>::getFirstTime() const {
    if (sequences_.empty()) {
        return Seconds{0.0};
    } else {
        return sequences_.front()->getFirstTime();
    }
}
template <typename Seq>
Seconds BaseTrack<Seq>::getLastTime() const {
    if (sequences_.empty()) {
        return Seconds{0.0};
    } else {
        return sequences_.back()->getLastTime();
    }
}
template <typename Seq>
std::vector<Seconds> BaseTrack<Seq>::getAllTimes() const {
    std::vector<Seconds> result;
    for (const auto& seq : sequences_) {
        for (size_t i = 0; i < seq->size(); i++) {
            result.push_back((*seq)[i].getTime());
        }
    }
    std::sort(result.begin(), result.end());
    return result;
}

template <typename Seq>
size_t BaseTrack<Seq>::size() const {
    return sequences_.size();
}
template <typename Seq>
bool BaseTrack<Seq>::empty() const {
    return sequences_.empty();
}

template <typename Seq>
Seq& BaseTrack<Seq>::operator[](size_t i) {
    return *sequences_[i];
}

template <typename Seq>
const Seq& BaseTrack<Seq>::operator[](size_t i) const {
    return *sequences_[i];
}

template <typename Seq>
const Seq& BaseTrack<Seq>::getFirst() const {
    return *sequences_.front();
}
template <typename Seq>
Seq& BaseTrack<Seq>::getFirst() {
    return *sequences_.front();
}
template <typename Seq>
const Seq& BaseTrack<Seq>::getLast() const {
    return *sequences_.back();
}
template <typename Seq>
Seq& BaseTrack<Seq>::getLast() {
    return *sequences_.back();
}

template <typename Seq>
auto BaseTrack<Seq>::begin() -> iterator {
    return util::makeIndirectIterator<true>(sequences_.begin());
}

template <typename Seq>
auto BaseTrack<Seq>::begin() const -> const_iterator {
    return util::makeIndirectIterator<true>(sequences_.begin());
}

template <typename Seq>
auto BaseTrack<Seq>::end() -> iterator {
    return util::makeIndirectIterator<true>(sequences_.end());
}

template <typename Seq>
auto BaseTrack<Seq>::end() const -> const_iterator {
    return util::makeIndirectIterator<true>(sequences_.end());
}

/**
 * Track of sequences
 * ----------X======X====X-----------------X=========X-------X=====X--------
 * |- case 1-|-case 2----------------------|-case 2----------|-case 2------|
 *           |-case 2a---|---------case 2b-|
 */
template <typename Seq>
void BaseTrack<Seq>::add(Seconds time, bool asNewSequence) {
    auto addNew = [this](std::unique_ptr<key_type> key) {
        std::vector<std::unique_ptr<key_type>> keys;
        keys.push_back(std::move(key));
        add(std::make_unique<Seq>(std::move(keys)));
    };

    auto key = std::make_unique<key_type>(time);
    if (sequences_.empty()) {
        addNew(std::move(key));
        return;
    }

    // 'it' will be the first seq. with a first time larger then 'to'.
    auto it = std::upper_bound(this->begin(), this->end(), time);
    if (it == this->begin()) {  // case 1
        if (asNewSequence) {
            addNew(std::move(key));
        } else {
            it->add(std::move(key));
        }
    } else {  // case 2
        auto& seq1 = *std::prev(it);
        if (time < seq1.getLastTime()) {  // case 2a
            seq1.add(std::move(key));
        } else {  // case 2b
            if (asNewSequence) {
                addNew(std::move(key));
            } else {
                addToClosestSequence(std::move(key));
            }
        }
    }
}

/**
 * Track of sequences
 * ----------X======X====X-----------------X=========X-------X=====X--------
 * |- case 1-|-case 2----------------------|-case 2----------|-case 3------|
 *           |-case 2a-----------|-case 2b-|
 */
template <typename Seq>
void BaseTrack<Seq>::addToClosestSequence(std::unique_ptr<key_type> key) {
    // 'it' will be the first seq. with a first time larger then 'to'.
    auto it = std::upper_bound(this->begin(), this->end(), key->getTime());

    if (it == this->begin()) {  // case 1
        it->add(std::move(key));
    } else if (it == this->end()) {  // case 3
        auto& seq1 = *std::prev(it);
        seq1.add(std::move(key));
    } else {  // case 2
        auto& seq1 = *std::prev(it);
        auto& seq2 = *it;
        if ((key->getTime() - seq1.getLastTime()) < (seq2.getFirstTime() - key->getTime())) {
            seq1.add(std::move(key));  // case 2a
        } else {
            seq2.add(std::move(key));  // case 2b
        }
    }
}

template <typename Seq>
void BaseTrack<Seq>::add(std::unique_ptr<KeyframeSequence> sequence) {
    if (auto s = util::dynamic_unique_ptr_cast<Seq>(std::move(sequence))) {
        add(std::move(s));
    } else {
        throw Exception("Invalid sequence type", IVW_CONTEXT);
    }
}

template <typename Seq>
void BaseTrack<Seq>::add(std::unique_ptr<Seq> sequence) {
    auto it = std::upper_bound(sequences_.begin(), sequences_.end(), sequence,
                               [](const auto& a, const auto& b) { return *a < *b; });

    if (it != sequences_.begin()) {
        if ((*std::prev(it))->getLastTime() > sequence->getFirstTime()) {
            throw Exception("Overlapping Sequence", IVW_CONTEXT);
        }
    }
    if (it != sequences_.end() && (*it)->getFirstTime() < sequence->getFirstTime()) {
        throw Exception("Overlapping Sequence", IVW_CONTEXT);
    }

    auto inserted = sequences_.insert(it, std::move(sequence));
    this->notifyKeyframeSequenceAdded(this, inserted->get());
    (*inserted)->KeyframeSequenceObserverble::addObserver(this);
}

template <typename Seq>
std::unique_ptr<KeyframeSequence> BaseTrack<Seq>::remove(size_t i) {
    if (i < sequences_.size()) {
        auto seq = std::move(sequences_[i]);
        sequences_.erase(sequences_.begin() + i);
        notifyKeyframeSequenceRemoved(this, seq.get());
        return std::move(seq);
    } else {
        return nullptr;
    }
}

template <typename Seq>
std::unique_ptr<Keyframe> BaseTrack<Seq>::remove(Keyframe* key) {
    for (auto& seq : sequences_) {
        if (auto res = seq->remove(key)) {
            if (seq->size() == 0) {
                remove(seq.get());
            }
            return std::move(res);
        }
    }
    return nullptr;
}
template <typename Seq>
std::unique_ptr<KeyframeSequence> BaseTrack<Seq>::remove(KeyframeSequence* seq) {
    auto it = std::find_if(sequences_.begin(), sequences_.end(),
                           [&](auto& elem) { return elem.get() == seq; });
    if (it != sequences_.end()) {
        auto res = std::move(*it);
        sequences_.erase(it);
        notifyKeyframeSequenceRemoved(this, res.get());
        return std::move(res);
    } else {
        return nullptr;
    }
}

template <typename Seq>
void BaseTrack<Seq>::onKeyframeSequenceMoved(KeyframeSequence* seq) {
    const bool atFront = sequences_.front().get() == seq;
    const bool atBack = sequences_.back().get() == seq;

    std::stable_sort(sequences_.begin(), sequences_.end(),
                     [](const auto& a, const auto& b) { return *a < *b; });

    // handle overlap?

    if (atFront || sequences_.front().get() == seq) {
        this->notifyFirstMoved(this);
    }

    if (atBack || sequences_.back().get() == seq) {
        this->notifyLastMoved(this);
    }
}

template <typename Seq>
void BaseTrack<Seq>::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
    s.serialize("identifier", identifier_, SerializationTarget::Attribute);
    s.serialize("name", name_);
    s.serialize("enabled", enabled_);
    s.serialize("priority", priority_);
    s.serialize("sequences", sequences_, "sequence");
}
template <typename Seq>
void BaseTrack<Seq>::deserialize(Deserializer& d) {
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

    using Elem = std::unique_ptr<Seq>;
    util::IndexedDeserializer<Elem>("sequences", "sequence")
        .onNew([&](Elem& seq) {
            this->notifyKeyframeSequenceAdded(this, seq.get());
            seq->KeyframeSequenceObserverble::addObserver(this);
        })
        .onRemove([&](Elem& seq) { this->notifyKeyframeSequenceRemoved(this, seq.get()); })(
            d, sequences_);
}

}  // namespace animation

}  // namespace inviwo

#endif  // IVW_BASETRACK_H
