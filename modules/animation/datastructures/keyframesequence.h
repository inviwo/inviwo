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

#ifndef IVW_KEYFRAMESEQUENCE_H
#define IVW_KEYFRAMESEQUENCE_H

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/io/serialization/serializable.h>

#include <modules/animation/datastructures/interpolation.h>
#include <modules/animation/datastructures/keyframe.h>
#include <modules/animation/datastructures/keyframeobserver.h>
#include <modules/animation/datastructures/keyframesequenceobserver.h>

namespace inviwo {

namespace animation {

/**
 * The KeyframeSequence is a part of a Track and owns Keyframes. KeyframeSequence provides the
 * base interface giving access to a list of Keyframes. And a interpolation method used to
 * interpolate between Keyframes.
 */
class IVW_MODULE_ANIMATION_API KeyframeSequence : public Serializable,
                                                  public KeyframeSequenceObserverble,
                                                  public KeyframeObserver {

public:
    KeyframeSequence() = default;
    virtual ~KeyframeSequence() = default;

    virtual size_t size() const = 0;

    virtual const Keyframe& operator[](size_t i) const = 0;
    virtual Keyframe& operator[](size_t i) = 0;

    virtual const Keyframe& getFirst() const = 0;
    virtual Keyframe& getFirst() = 0;
    virtual const Keyframe& getLast() const = 0;
    virtual Keyframe& getLast() = 0;

    virtual void remove(size_t i) = 0;
    virtual void add(const Keyframe& key) = 0;

    virtual void setInterpolation(std::unique_ptr<Interpolation> interpolation) = 0;

    virtual void serialize(Serializer& s) const override = 0;
    virtual void deserialize(Deserializer& d) override = 0;
};

// A sequence should always have at least two keyframes.
template <typename Key>
class KeyframeSequenceTyped : public KeyframeSequence {
public:
    static_assert(std::is_base_of<Keyframe, Key>::value, "Key has to derive from Keyframe");

    KeyframeSequenceTyped();

    KeyframeSequenceTyped(const std::vector<Key>& keyframes,
                          std::unique_ptr<InterpolationTyped<Key>> interpolation);
    KeyframeSequenceTyped(const KeyframeSequenceTyped& rhs);
    KeyframeSequenceTyped& operator=(const KeyframeSequenceTyped& that);

    virtual ~KeyframeSequenceTyped() = default;

    virtual size_t size() const override;

    virtual const Key& operator[](size_t i) const override;
    virtual Key& operator[](size_t i) override;

    virtual const Key& getFirst() const override;
    virtual Key& getFirst() override;
    virtual const Key& getLast() const override;
    virtual Key& getLast() override;

    virtual void remove(size_t i) override;
    virtual void add(const Keyframe& key) override;
    void add(const Key& key);

    virtual auto operator()(Seconds from, Seconds to) const -> typename Key::value_type;

    virtual void setInterpolation(std::unique_ptr<Interpolation> interpolation) override;
    void setInterpolation(std::unique_ptr<InterpolationTyped<Key>> interpolation);

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

private:
    void addKeyFrame(std::unique_ptr<Key> key);

    virtual void onKeyframeTimeChanged(Keyframe* key, Seconds oldTime) override;

    std::vector<std::unique_ptr<Key>> keyframes_;
    std::unique_ptr<InterpolationTyped<Key>> interpolation_;
};

template <typename Key>
bool operator==(const KeyframeSequenceTyped<Key>& a, const KeyframeSequenceTyped<Key>& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i]) return false;
    }
    return true;
}
template <typename Key>
bool operator!=(const KeyframeSequenceTyped<Key>& a, const KeyframeSequenceTyped<Key>& b) {
    return !(a == b);
}

template <typename Key>
KeyframeSequenceTyped<Key>::KeyframeSequenceTyped()
    : KeyframeSequence(), keyframes_(), interpolation_() {
}

template <typename Key>
KeyframeSequenceTyped<Key>::KeyframeSequenceTyped(
    const std::vector<Key>& keyframes, std::unique_ptr<InterpolationTyped<Key>> interpolation)
    : KeyframeSequence(), keyframes_(), interpolation_{std::move(interpolation)} {
    for (const auto& key : keyframes) {
        keyframes_.push_back(std::make_unique<Key>(key));
    }
}

template <typename Key>
KeyframeSequenceTyped<Key>::KeyframeSequenceTyped(const KeyframeSequenceTyped<Key>& rhs)
    : KeyframeSequence(rhs), interpolation_(rhs.interpolation_->clone()) {
    for (const auto& key : rhs.keyframes_) {
        addKeyFrame(std::make_unique<Key>(*key));
    }
}

template <typename Key>
KeyframeSequenceTyped<Key>& KeyframeSequenceTyped<Key>::operator=(
    const KeyframeSequenceTyped<Key>& that) {
    if (this != &that) {
        KeyframeSequence::operator=(that);
        interpolation_.reset(that.interpolation_->clone());

        for (size_t i = 0; i < std::min(keyframes_.size(), that.keyframes_.size()); i++) {
            *keyframes_[i] = *that.keyframes_[i];
        }
        for (size_t i = std::min(keyframes_.size(), that.keyframes_.size());
             i < that.keyframes_.size(); i++) {
            keyframes_.push_back(std::make_unique<Key>(*that.keyframes_[i]));
            notifyKeyframeAdded(keyframes_.back().get(), this);
        }
        while (keyframes_.size() > that.keyframes_.size()) {
            auto key = std::move(keyframes_.back());
            keyframes_.pop_back();
            notifyKeyframeRemoved(key.get(), this);
        }
    }
    return *this;
}

template <typename Key>
void KeyframeSequenceTyped<Key>::onKeyframeTimeChanged(Keyframe* key, Seconds oldTime) {
    const auto startTime = keyframes_.front()->getTime();
    const auto endTime = keyframes_.back()->getTime();

    std::stable_sort(keyframes_.begin(), keyframes_.end(),
                     [](const auto& a, const auto& b) { return a->getTime() < b->getTime(); });
    if (startTime != keyframes_.front()->getTime() || endTime != keyframes_.back()->getTime()) {
        notifyKeyframeSequenceMoved(this);
    }
}

template <typename Key>
void KeyframeSequenceTyped<Key>::setInterpolation(
    std::unique_ptr<InterpolationTyped<Key>> interpolation) {
    interpolation_ = std::move(interpolation);
}

template <typename Key>
void KeyframeSequenceTyped<Key>::setInterpolation(std::unique_ptr<Interpolation> interpolation) {
    if (auto inter =
            util::dynamic_unique_ptr_cast<InterpolationTyped<Key>>(std::move(interpolation))) {
        setInterpolation(std::move(inter));
    } else {
        throw Exception("Interpolation type does not match key", IvwContext);
    }
}

template <typename Key>
auto KeyframeSequenceTyped<Key>::operator()(Seconds from, Seconds to) const ->
    typename Key::value_type {
    if (interpolation_) {
        return (*interpolation_)(keyframes_, to);
    } else {
        return keyframes_.front()->getValue();
    }
}

template <typename Key>
void KeyframeSequenceTyped<Key>::add(const Keyframe& key) {
    add(dynamic_cast<const Key&>(key));
}

template <typename Key>
void KeyframeSequenceTyped<Key>::add(const Key& key) {
    addKeyFrame(std::make_unique<Key>(key));
}

template <typename Key>
void KeyframeSequenceTyped<Key>::addKeyFrame(std::unique_ptr<Key> key) {
    auto it = keyframes_.insert(std::upper_bound(keyframes_.begin(), keyframes_.end(), key,
                                                 [&key](const auto& a, const auto& b) {
                                                     return a->getTime() < b->getTime();
                                                 }),
                                std::move(key));

    (*it)->addObserver(this);
    notifyKeyframeAdded(it->get(), this);
}

template <typename Key>
void KeyframeSequenceTyped<Key>::remove(size_t i) {

    auto key = std::move(keyframes_[i]);
    keyframes_.erase(keyframes_.begin() + i);
    notifyKeyframeRemoved(key.get(), this);
}

template <typename Key>
Key& KeyframeSequenceTyped<Key>::getLast() {
    return *keyframes_.back();
}

template <typename Key>
const Key& KeyframeSequenceTyped<Key>::getLast() const {
    return *keyframes_.back();
}

template <typename Key>
Key& KeyframeSequenceTyped<Key>::getFirst() {
    return *keyframes_.front();
}

template <typename Key>
const Key& animation::KeyframeSequenceTyped<Key>::getFirst() const {
    return *keyframes_.front();
}

template <typename Key>
Key& KeyframeSequenceTyped<Key>::operator[](size_t i) {
    return *keyframes_[i];
}

template <typename Key>
const Key& KeyframeSequenceTyped<Key>::operator[](size_t i) const {
    return *keyframes_[i];
}

template <typename Key>
size_t KeyframeSequenceTyped<Key>::size() const {
    return keyframes_.size();
}

template <typename Key>
void KeyframeSequenceTyped<Key>::serialize(Serializer& s) const {
    s.serialize("keyframes", keyframes_, "keyframe");
    s.serialize("interpolation", interpolation_);
}

template <typename Key>
void KeyframeSequenceTyped<Key>::deserialize(Deserializer& d) {
    using Elem = std::unique_ptr<Key>;
    util::IndexedDeserializer<Elem>("keyframes", "keyframe")
        .onNew([&](Elem& key) {
            notifyKeyframeAdded(key.get(), this);
            key->addObserver(this);
        })
        .onRemove([&](Elem& key) { notifyKeyframeRemoved(key.get(), this); })(d, keyframes_);

    d.deserializeAs<Interpolation>("interpolation", interpolation_);
}

}  // namespace

}  // namespace

#endif  // IVW_KEYFRAMESEQUENCE_H
