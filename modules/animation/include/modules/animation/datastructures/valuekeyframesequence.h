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

#ifndef IVW_VALUEKEYFRAMESEQUENCE_H
#define IVW_VALUEKEYFRAMESEQUENCE_H

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/observer.h>

#include <modules/animation/datastructures/basekeyframesequence.h>
#include <modules/animation/interpolation/interpolation.h>
#include <modules/animation/interpolation/constantinterpolation.h>

namespace inviwo {

namespace animation {

class ValueKeyframeSequence;

class IVW_MODULE_ANIMATION_API ValueKeyframeSequenceObserver : public Observer {
public:
    virtual void onValueKeyframeSequenceEasingChanged(ValueKeyframeSequence*){};
    virtual void onValueKeyframeSequenceInterpolationChanged(ValueKeyframeSequence*){};
};

class IVW_MODULE_ANIMATION_API ValueKeyframeSequenceObserverble
    : public Observable<ValueKeyframeSequenceObserver> {
protected:
    void notifyValueKeyframeSequenceEasingChanged(ValueKeyframeSequence* seq);
    void notifyValueKeyframeSequenceInterpolationChanged(ValueKeyframeSequence* seq);
};

class IVW_MODULE_ANIMATION_API ValueKeyframeSequence : public ValueKeyframeSequenceObserverble {
public:
    ValueKeyframeSequence() = default;
    virtual ~ValueKeyframeSequence() = default;

    virtual const Interpolation& getInterpolation() const = 0;
    virtual void setInterpolation(std::unique_ptr<Interpolation> interpolation) = 0;

    virtual easing::EasingType getEasingType() const = 0;
    virtual void setEasingType(easing::EasingType easing) = 0;
};

/** \class KeyframeSequenceTyped
 * KeyframeSequence for a given type of KeyFames.
 * @see KeyframeSequence
 */
template <typename Key>
class KeyframeSequenceTyped : public BaseKeyframeSequence<Key>, public ValueKeyframeSequence {
public:
    using key_type = Key;
    using value_type = typename Key::value_type;
    static_assert(std::is_base_of<Keyframe, Key>::value, "Key has to derive from Keyframe");

    KeyframeSequenceTyped();
    KeyframeSequenceTyped(std::vector<std::unique_ptr<Key>> keyframes);
    KeyframeSequenceTyped(std::vector<std::unique_ptr<Key>> keyframes,
                          std::unique_ptr<InterpolationTyped<Key>> interpolation);

    KeyframeSequenceTyped(const KeyframeSequenceTyped& rhs);
    KeyframeSequenceTyped& operator=(const KeyframeSequenceTyped& that);

    /**
     * Remove all keyframes and call KeyframeObserver::notifyKeyframeRemoved
     */
    virtual ~KeyframeSequenceTyped();

    virtual KeyframeSequenceTyped<Key>* clone() const override;

    virtual auto operator()(Seconds from, Seconds to) const -> value_type;

    virtual const InterpolationTyped<Key>& getInterpolation() const override;
    virtual void setInterpolation(std::unique_ptr<Interpolation> interpolation) override;
    void setInterpolation(std::unique_ptr<InterpolationTyped<Key>> interpolation);

    virtual easing::EasingType getEasingType() const override;
    virtual void setEasingType(easing::EasingType easing) override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

private:
    easing::EasingType easing_{easing::EasingType::Linear};
    std::unique_ptr<InterpolationTyped<Key>> interpolation_;
};

template <typename Key>
bool operator==(const KeyframeSequenceTyped<Key>& a, const KeyframeSequenceTyped<Key>& b) {
    return a.getEasingType() == b.getEasingType() && a.getInterpolation() == b.getInterpolation() &&
           std::equal(a.begin(), a.end(), b.begin(), b.end(),
                      [](const auto& a, const auto& b) { return a == b; });
}
template <typename Key>
bool operator!=(const KeyframeSequenceTyped<Key>& a, const KeyframeSequenceTyped<Key>& b) {
    return !(a == b);
}

template <typename Key>
KeyframeSequenceTyped<Key>::KeyframeSequenceTyped()
    : BaseKeyframeSequence<Key>{}
    , ValueKeyframeSequence{}
    , interpolation_{std::make_unique<ConstantInterpolation<Key>>()} {}

template <typename Key>
KeyframeSequenceTyped<Key>::KeyframeSequenceTyped(std::vector<std::unique_ptr<Key>> keyframes)
    : BaseKeyframeSequence<Key>{std::move(keyframes)}
    , ValueKeyframeSequence()
    , interpolation_{std::make_unique<ConstantInterpolation<Key>>()} {}

template <typename Key>
KeyframeSequenceTyped<Key>::KeyframeSequenceTyped(
    std::vector<std::unique_ptr<Key>> keyframes,
    std::unique_ptr<InterpolationTyped<Key>> interpolation)
    : BaseKeyframeSequence<Key>{std::move(keyframes)}
    , ValueKeyframeSequence()
    , interpolation_{std::move(interpolation)} {}
template <typename Key>
KeyframeSequenceTyped<Key>::KeyframeSequenceTyped(const KeyframeSequenceTyped<Key>& rhs)
    : BaseKeyframeSequence<Key>(rhs)
    , ValueKeyframeSequence(rhs)
    , interpolation_(std::unique_ptr<InterpolationTyped<Key>>(rhs.interpolation_->clone())) {}

template <typename Key>
KeyframeSequenceTyped<Key>& KeyframeSequenceTyped<Key>::operator=(
    const KeyframeSequenceTyped<Key>& that) {
    if (this != &that) {
        BaseKeyframeSequence<Key>::operator=(that);
        ValueKeyframeSequence::operator=(that);
        setInterpolation(that.interpolation_->clone());
        setEasingType(that.easing_);
    }
    return *this;
}

template <typename Key>
KeyframeSequenceTyped<Key>::~KeyframeSequenceTyped() = default;

template <typename Key>
KeyframeSequenceTyped<Key>* KeyframeSequenceTyped<Key>::clone() const {
    return new KeyframeSequenceTyped<Key>(*this);
}

template <typename Key>
auto KeyframeSequenceTyped<Key>::operator()(Seconds from, Seconds to) const ->
    typename Key::value_type {
    if (interpolation_) {
        return (*interpolation_)(this->keyframes_, from, to, easing_);
    } else {
        return this->keyframes_.front()->getValue();
    }
}

template <typename Key>
const InterpolationTyped<Key>& KeyframeSequenceTyped<Key>::getInterpolation() const {
    return *interpolation_;
}

template <typename Key>
void KeyframeSequenceTyped<Key>::setInterpolation(
    std::unique_ptr<InterpolationTyped<Key>> interpolation) {
    if (interpolation && !interpolation_->equal(*interpolation)) {
        interpolation_ = std::move(interpolation);
        notifyValueKeyframeSequenceInterpolationChanged(this);
    }
}

template <typename Key>
void KeyframeSequenceTyped<Key>::setInterpolation(std::unique_ptr<Interpolation> interpolation) {
    if (auto inter =
            util::dynamic_unique_ptr_cast<InterpolationTyped<Key>>(std::move(interpolation))) {
        setInterpolation(std::move(inter));
    } else {
        throw Exception("Interpolation type does not match key", IVW_CONTEXT);
    }
}

template <typename Key>
easing::EasingType KeyframeSequenceTyped<Key>::getEasingType() const {
    return easing_;
}

template <typename Key>
void KeyframeSequenceTyped<Key>::setEasingType(easing::EasingType easing) {
    if (easing_ != easing) {
        easing_ = easing;
        notifyValueKeyframeSequenceEasingChanged(this);
    }
}

template <typename Key>
void KeyframeSequenceTyped<Key>::serialize(Serializer& s) const {
    BaseKeyframeSequence<Key>::serialize(s);
    s.serialize("easing", easing_);
    s.serialize("interpolation", interpolation_);
}

template <typename Key>
void KeyframeSequenceTyped<Key>::deserialize(Deserializer& d) {
    BaseKeyframeSequence<Key>::deserialize(d);
    {
        easing::EasingType easing = easing_;
        d.deserialize("easing", easing);
        setEasingType(easing);
    }
    {
        std::unique_ptr<InterpolationTyped<Key>> interpolation;
        d.deserializeAs<Interpolation>("interpolation", interpolation);
        setInterpolation(std::move(interpolation));
    }
}

}  // namespace animation

}  // namespace inviwo

#endif  // IVW_VALUEKEYFRAMESEQUENCE_H
