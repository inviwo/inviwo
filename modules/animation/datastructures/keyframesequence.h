/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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
 * \class KeyframeSequence
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS
 */
class IVW_MODULE_ANIMATION_API KeyframeSequence : public Serializable,
                                                  public KeyframeSequenceObserverble,
                                                  public KeyframeObserver {
        
public:
    KeyframeSequence() = default;
    virtual ~KeyframeSequence() = default;

    virtual size_t getNumberOfKeyframes() = 0;

    virtual void serialize(Serializer& s) const override = 0;
    virtual void deserialize(Deserializer& d) override = 0;
};

// A sequence should always have at least to keyframes.
template <typename Key>
class KeyframeSequenceTyped : public KeyframeSequence {
public:
    KeyframeSequenceTyped(const std::vector<Key>& keyframes,
                          std::unique_ptr<InterpolationTyped<Key>> interpolation)
        : KeyframeSequence(), keyframes_(), interpolation_{std::move(interpolation)} {

        for (const auto& key : keyframes) {
            keyframes_.push_back(util::make_unique<Key>(key));
        }
    }
    KeyframeSequenceTyped(const KeyframeSequenceTyped& rhs)
        : KeyframeSequence(rhs), interpolation_(rhs.interpolation_->clone()) {
        for (const auto& key : rhs.keyframes_) {
            addKeyFrame(util::make_unique<Key>(*key));
        }
    }
    KeyframeSequenceTyped& operator=(const KeyframeSequenceTyped& that) {
        if (this != &that) {
            KeyframeSequence::operator=(that);
            interpolation_.reset(that.interpolation_->clone());

            for (size_t i = 0; i < std::min(keyframes_.size(), that.keyframes_.size()); i++) {
                *keyframes_[i] = *that.keyframes_[i];
            }
            for (size_t i = std::min(keyframes_.size(), that.keyframes_.size());
                 i < that.keyframes_.size(); i++) {
                keyframes_.push_back(util::make_unique<Key>(*that.keyframes_[i]));
                notifyKeyframeAdded(keyframes_.back().get());
            }
            while (keyframes_.size() > that.keyframes_.size()) {
                auto key = std::move(keyframes_.back());
                keyframes_.pop_back();
                notifyKeyframeRemoved(key.get());
            }
        }
        return *this;
    }

    virtual ~KeyframeSequenceTyped() = default;

    virtual size_t getNumberOfKeyframes() { return keyframes_.size(); }
    virtual auto evaluate(Time from, Time to) const -> typename Key::value_type {
        return interpolation_->evaluate(keyframes_, to);
    }

    void setInterpolation(std::shared_ptr<const InterpolationTyped<Key>> interpolation) {
        interpolation_ = interpolation;
    }

    const Key& getFirst() const { return *keyframes_.front(); }
    Key& getFirst() { return *keyframes_.front(); }
    const Key& getLast() const { return *keyframes_.back(); }
    Key& getLast() { return *keyframes_.back(); }

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

private:
    void addKeyFrame(std::unique_ptr<Key> key) {
        keyframes_.push_back(std::move(key));
        keyframes_.back()->addObserver(this);
        notifyKeyframeAdded(keyframes_.back().get());
    }

    virtual void onKeyframeTimeChanged(Keyframe* key, Time oldTime) override {
        std::stable_sort(keyframes_.begin(), keyframes_.end(),
                         [](const auto& a, const auto& b) { return a->getTime() < b->getTime(); });
    }

    std::vector<std::unique_ptr<Key>> keyframes_;
    std::unique_ptr<InterpolationTyped<Key>> interpolation_;
};

template <typename Key>
void KeyframeSequenceTyped<Key>::serialize(Serializer& s) const {
    s.serialize("keyframes", keyframes_);
    s.serialize("interpolation", interpolation_);
}


template <typename Key>
void KeyframeSequenceTyped<Key>::deserialize(Deserializer& d) {
    d.deserialize("keyframes", keyframes_);
    d.deserialize("interpolation", interpolation_);
}


} // namespace

} // namespace

#endif // IVW_KEYFRAMESEQUENCE_H

