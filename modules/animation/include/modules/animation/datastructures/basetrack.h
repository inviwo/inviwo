/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2025 Inviwo Foundation
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

#include <inviwo/core/io/serialization/deserializer.h>                  // for Deserializer, Ind...
#include <inviwo/core/io/serialization/serializebase.h>                 // for SerializationTarget
#include <inviwo/core/io/serialization/serializer.h>                    // for Serializer
#include <inviwo/core/util/exception.h>                                 // for Exception
#include <inviwo/core/util/indirectiterator.h>                          // for makeIndirectIterator
#include <inviwo/core/util/stdextensions.h>                             // for dynamic_unique_pt...
#include <modules/animation/datastructures/animationtime.h>             // for Seconds
#include <modules/animation/datastructures/keyframesequenceobserver.h>  // for KeyframeSequenceO...
#include <modules/animation/datastructures/track.h>                     // for Track

#include <algorithm>    // for sort, upper_bound
#include <cstddef>      // for size_t
#include <iterator>     // for prev
#include <memory>       // for unique_ptr, make_...
#include <string>       // for string, operator!=
#include <string_view>  // for string_view
#include <type_traits>  // for is_base_of
#include <utility>      // for move
#include <vector>       // for vector

namespace inviwo {

namespace animation {
class Keyframe;
class KeyframeSequence;

namespace detail {

/*
 * Creates a KeyframeSequence using the provided keyframes.
 * Provide a template specialization to add custom KeyframeSequence creation behavior.
 */
template <typename Seq>
struct DefaultSequenceCreator {
    using key_type = typename Seq::key_type;
    static std::unique_ptr<Seq> create(std::vector<std::unique_ptr<key_type>> keys);
};
template <typename Seq>
std::unique_ptr<Seq> DefaultSequenceCreator<Seq>::create(
    std::vector<std::unique_ptr<key_type>> keys) {
    return std::make_unique<Seq>(std::move(keys));
}

}  // namespace detail

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

    BaseTrack(const std::string& name, size_t priority = 0);

    virtual ~BaseTrack();

    virtual bool isEnabled() const override;
    virtual void setEnabled(bool enabled) override;

    virtual const std::string& getName() const override;
    virtual void setName(const std::string& name) override;

    virtual size_t getPriority() const override;
    virtual void setPriority(size_t priority) override;

    virtual Seconds getFirstTime() const override;
    virtual Seconds getLastTime() const override;

    virtual std::optional<Seconds> getPrevTime(Seconds at) const final;
    virtual std::optional<Seconds> getNextTime(Seconds at) const final;
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
    /**
     * Add a Keyframe at time using default values and return the added keyframe.
     * The Keyframe is added to a new KeyframeSequence if asNewSequence is true,
     * otherwise it is added to the KeyframeSequence at, or before, time.
     * A KeyframeSequence is added to the Track if none exists.
     *
     * Tracks should override createKeyframe(Seconds time) to customize Keyframe creation.
     */
    virtual key_type* add(Seconds time, bool asNewSequence) override;
    /**
     * Add KeyframeSequence and call KeyframeSequenceObserverble::notifyKeyframeSequenceAdded
     * @throw Exception if KeyframeSequence is not compatible with BaseTrack<Seq>
     * @throw Exception if KeyframeSequence overlaps existing sequences
     * @see BaseTrack::add(std::unique_ptr<Seq> sequence)
     */
    virtual Seq* add(std::unique_ptr<KeyframeSequence> sequence) override;
    /**
     * Add KeyframeSequence and call KeyframeSequenceObserverble::notifyKeyframeSequenceAdded
     * @throw Exception if KeyframeSequence overlaps existing sequences
     */
    virtual Seq* add(std::unique_ptr<Seq> sequence);

    virtual std::unique_ptr<KeyframeSequence> remove(size_t i) override;
    virtual std::unique_ptr<Keyframe> remove(Keyframe* key) override;
    virtual std::unique_ptr<KeyframeSequence> remove(KeyframeSequence* seq) override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    /*
     * Creates a Seq::key_type using default constructor.
     * Provide a template specialization or override this function to add custom Keyframe creation
     * behavior, e.g. based on Property value, instead of add(Seconds time, bool asNewSequence).
     */
    virtual std::unique_ptr<key_type> createKeyframe(Seconds time) const;

protected:
    BaseTrack(const BaseTrack& other);
    BaseTrack(BaseTrack&& other) = default;
    BaseTrack& operator=(const BaseTrack&);
    BaseTrack& operator=(BaseTrack&& other) = default;
    virtual void onKeyframeSequenceMoved(KeyframeSequence* seq) override;
    key_type* addToClosestSequence(std::unique_ptr<key_type> key);

private:
    bool enabled_{true};
    std::string name_;
    size_t priority_{0};

    // Sorted list of non-overlapping sequences of key frames
    std::vector<std::unique_ptr<Seq>> sequences_;
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS

template <typename Seq>
BaseTrack<Seq>::BaseTrack(const std::string& name, size_t priority)
    : name_{name}, priority_{priority} {}

template <typename Seq>
BaseTrack<Seq>::BaseTrack(const BaseTrack<Seq>& other)
    : KeyframeSequenceObserver(other)
    , enabled_{other.enabled_}
    , name_{other.name_}
    , priority_{other.priority_} {
    for (auto& seq : other.sequences_) {
        add(std::make_unique<Seq>(*seq));
    }
}

template <typename Seq>
BaseTrack<Seq>& BaseTrack<Seq>::operator=(const BaseTrack<Seq>& that) {
    if (this != &that) {
        while (!sequences_.empty()) {
            remove(sequences_.back().get());
        }
        enabled_ = that.enabled_;
        name_ = that.name_;
        priority_ = that.priority_;
        for (const auto& seq : that.sequences_) {
            add(std::make_unique<Seq>(*seq));
        }
    }
    return *this;
}

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
std::optional<Seconds> BaseTrack<Seq>::getPrevTime(Seconds at) const {
    std::optional<Seconds> prevKeyframeTime = std::nullopt;
    for (auto& sequence : sequences_) {
        if (auto t = sequence->getPrevTime(at)) {
            if (!prevKeyframeTime) {
                prevKeyframeTime = t;
            } else {
                prevKeyframeTime = std::max(prevKeyframeTime, t);
            }
        }
    }
    return prevKeyframeTime;
}

template <typename Seq>
std::optional<Seconds> BaseTrack<Seq>::getNextTime(Seconds at) const {
    std::optional<Seconds> nextKeyframeTime = std::nullopt;
    for (auto& sequence : sequences_) {
        if (auto t = sequence->getNextTime(at)) {
            if (!nextKeyframeTime) {
                nextKeyframeTime = t;
            } else {
                nextKeyframeTime = std::min(nextKeyframeTime, t);
            }
        }
    }
    return nextKeyframeTime;
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
typename BaseTrack<Seq>::key_type* BaseTrack<Seq>::add(Seconds time, bool asNewSequence) {
    auto addNew = [this](std::unique_ptr<key_type> key) -> key_type* {
        std::vector<std::unique_ptr<key_type>> keys;
        keys.push_back(std::move(key));
        auto seq = add(detail::DefaultSequenceCreator<Seq>::create(std::move(keys)));
        return &seq->getLast();
    };

    auto key = createKeyframe(time);
    if (sequences_.empty()) {
        return addNew(std::move(key));
    }

    // 'it' will be the first seq. with a first time larger then 'to'.
    auto it = std::upper_bound(this->begin(), this->end(), time);
    if (it == this->begin()) {  // case 1
        if (asNewSequence) {
            return addNew(std::move(key));
        } else {
            it->add(std::move(key));
            return &it->getLast();
        }
    } else {  // case 2
        auto& seq1 = *std::prev(it);
        if (time < seq1.getLastTime()) {  // case 2a
            return seq1.add(std::move(key));
        } else {  // case 2b
            if (asNewSequence) {
                return addNew(std::move(key));
            } else {
                return addToClosestSequence(std::move(key));
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
auto BaseTrack<Seq>::addToClosestSequence(std::unique_ptr<key_type> key) -> key_type* {
    // 'it' will be the first seq. with a first time larger then 'to'.
    auto it = std::upper_bound(this->begin(), this->end(), key->getTime());

    if (it == this->begin()) {  // case 1
        return it->add(std::move(key));
    } else if (it == this->end()) {  // case 3
        auto& seq1 = *std::prev(it);
        return seq1.add(std::move(key));
    } else {  // case 2
        auto& seq1 = *std::prev(it);
        auto& seq2 = *it;
        if ((key->getTime() - seq1.getLastTime()) < (seq2.getFirstTime() - key->getTime())) {
            return seq1.add(std::move(key));  // case 2a
        } else {
            return seq2.add(std::move(key));  // case 2b
        }
    }
}

template <typename Seq>
Seq* BaseTrack<Seq>::add(std::unique_ptr<KeyframeSequence> sequence) {
    if (auto s = util::dynamic_unique_ptr_cast<Seq>(std::move(sequence))) {
        return add(std::move(s));
    } else {
        throw Exception("Invalid sequence type");
    }
    return nullptr;
}

template <typename Seq>
Seq* BaseTrack<Seq>::add(std::unique_ptr<Seq> sequence) {
    auto it = std::upper_bound(sequences_.begin(), sequences_.end(), sequence,
                               [](const auto& a, const auto& b) { return *a < *b; });

    if (it != sequences_.begin()) {
        if ((*std::prev(it))->getLastTime() > sequence->getFirstTime()) {
            throw Exception("Overlapping Sequence");
        }
    }
    if (it != sequences_.end() && (*it)->getFirstTime() < sequence->getFirstTime()) {
        throw Exception("Overlapping Sequence");
    }

    auto inserted = sequences_.insert(it, std::move(sequence));
    this->notifyKeyframeSequenceAdded(this, inserted->get());
    (*inserted)->KeyframeSequenceObserverble::addObserver(this);
    return inserted->get();
}

template <typename Seq>
std::unique_ptr<KeyframeSequence> BaseTrack<Seq>::remove(size_t i) {
    if (i < sequences_.size()) {
        auto seq = std::move(sequences_[i]);
        sequences_.erase(sequences_.begin() + i);
        notifyKeyframeSequenceRemoved(this, seq.get());
        return seq;
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
            return res;
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
        return res;
    } else {
        return nullptr;
    }
}

template <typename Seq>
std::unique_ptr<typename BaseTrack<Seq>::key_type> BaseTrack<Seq>::createKeyframe(
    Seconds time) const {
    return std::make_unique<key_type>(time);
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
    s.serialize("name", name_);
    s.serialize("enabled", enabled_);
    s.serialize("priority", priority_);
    s.serialize("sequences", sequences_, "sequence");
}

template <typename Seq>
void BaseTrack<Seq>::deserialize(Deserializer& d) {
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

    d.deserialize(
        "sequences", sequences_, "sequence",
        deserializer::IndexFunctions{.makeNew = []() { return std::unique_ptr<Seq>{}; },
                                     .onNew =
                                         [&](std::unique_ptr<Seq>& seq, size_t) {
                                             this->notifyKeyframeSequenceAdded(this, seq.get());
                                             seq->KeyframeSequenceObserverble::addObserver(this);
                                         },
                                     .onRemove =
                                         [&](std::unique_ptr<Seq>& seq) {
                                             this->notifyKeyframeSequenceRemoved(this, seq.get());
                                         }});
}

#endif

}  // namespace animation

}  // namespace inviwo
