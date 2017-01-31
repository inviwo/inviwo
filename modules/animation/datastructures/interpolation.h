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

#ifndef IVW_KEYFRAME_INTERPOLATION_H
#define IVW_KEYFRAME_INTERPOLATION_H

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/animation/datastructures/keyframe.h>
#include <modules/animation/datastructures/easing.h>
#include <inviwo/core/io/serialization/serializable.h>

#include <algorithm>

namespace inviwo {

namespace animation {


/**
 *	Base class for keyframe interpolations.
 */
class IVW_MODULE_ANIMATION_API Interpolation : public Serializable {
public:
    Interpolation() = default;
    virtual ~Interpolation() = default;

    virtual Interpolation* clone() const = 0;

    virtual std::string getClassIdentifier() const = 0;

    virtual void serialize(Serializer& s) const override = 0;
    virtual void deserialize(Deserializer& d) override = 0;
};

template <typename Key>
class InterpolationTyped : public Interpolation {
public:
    InterpolationTyped() = default;
    virtual ~InterpolationTyped() = default;

    virtual InterpolationTyped* clone() const override = 0;

    virtual std::string getClassIdentifier() const override = 0;
    virtual void serialize(Serializer& s) const override = 0;
    virtual void deserialize(Deserializer& d) override = 0;

    // keys should be sorted by time
    virtual auto operator()(const std::vector<std::unique_ptr<Key>>& keys, Seconds t) const ->
        typename Key::value_type = 0;
};

template <typename Key>
class LinearInterpolation : public InterpolationTyped<Key> {
public:
    LinearInterpolation() = default;
    virtual ~LinearInterpolation() = default;

    virtual LinearInterpolation* clone() const override { return new LinearInterpolation(*this); };

    static std::string classIdentifier() {
        auto keyid = Key::classIdentifier();
        std::string id = "org.inviwo.animation.linearinterpolation.";
        auto res = std::mismatch(id.begin(), id.end(), keyid.begin(), keyid.end());
        id.append(res.second, keyid.end());
        return id;
    };
    virtual std::string getClassIdentifier() const override { return classIdentifier(); }

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    // keys should be sorted by time
    virtual auto operator()(const std::vector<std::unique_ptr<Key>>& keys, Seconds t) const ->
        typename Key::value_type override {
        auto it = std::upper_bound(
            keys.begin(), keys.end(), t,
            [](const auto& time, const auto& key) { return time < key->getTime(); });

        const auto& v1 = (*std::prev(it))->getValue();
        const auto& t1 = (*std::prev(it))->getTime();

        const auto& v2 = (*it)->getValue();
        const auto& t2 = (*it)->getTime();

        return glm::mix(v1, v2,
                        Easing::Ease((t - t1) / (t2 - t1), Easing::EEasingType::InOutCubic));
    }
};

template <typename Key>
void LinearInterpolation<Key>::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
}

template <typename Key>
void LinearInterpolation<Key>::deserialize(Deserializer& d) {
    std::string className;
    d.deserialize("type", className, SerializationTarget::Attribute);
    if (className != getClassIdentifier()) {
        throw SerializationException(
            "Deserialized interpolation: " + getClassIdentifier() +
                " from a serialized interpolation with a different class identifier: " + className,
            IvwContext);
    }
}

} // namespace

} // namespace

#endif // IVW_KEYFRAME_INTERPOLATION_H

