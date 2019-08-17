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

#ifndef IVW_KEYFRAME_CONSTANT_INTERPOLATION_H
#define IVW_KEYFRAME_CONSTANT_INTERPOLATION_H

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/animation/interpolation/interpolation.h>

#include <algorithm>

namespace inviwo {

namespace animation {

/** \class ConstantInterpolation
 * Interpolation function for key frames.
 * Returns left keyframe value until reaching right keyframe.
 *
 */
template <typename Key>
class ConstantInterpolation : public InterpolationTyped<Key> {
public:
    ConstantInterpolation() = default;
    virtual ~ConstantInterpolation() = default;

    virtual ConstantInterpolation* clone() const override;

    virtual std::string getName() const override;

    static std::string classIdentifier();
    virtual std::string getClassIdentifier() const override;

    virtual bool equal(const Interpolation& other) const override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    // keys should be sorted by time
    virtual auto operator()(const std::vector<std::unique_ptr<Key>>& keys, Seconds from, Seconds to,
                            easing::EasingType) const -> typename Key::value_type override;
};

template <typename Key>
ConstantInterpolation<Key>* ConstantInterpolation<Key>::clone() const {
    return new ConstantInterpolation<Key>(*this);
}

namespace detail {
template <typename T, typename std::enable_if<!std::is_enum<T>::value, int>::type = 0>
std::string getConstantInterpolationClassIdentifier() {
    return "org.inviwo.animation.constantinterpolation." + Defaultvalues<T>::getName();
}
template <typename T, typename std::enable_if<std::is_enum<T>::value, int>::type = 0>
std::string getConstantInterpolationClassIdentifier() {
    using ET = typename std::underlying_type<T>::type;
    return "org.inviwo.animation.constantinterpolation.enum." + Defaultvalues<ET>::getName();
}
}  // namespace detail

template <typename Key>
std::string ConstantInterpolation<Key>::classIdentifier() {
    return detail::getConstantInterpolationClassIdentifier<typename Key::value_type>();
}

template <typename Key>
std::string ConstantInterpolation<Key>::getClassIdentifier() const {
    return classIdentifier();
}

template <typename Key>
std::string ConstantInterpolation<Key>::getName() const {
    return "Constant";
}

template <typename Key>
bool ConstantInterpolation<Key>::equal(const Interpolation& other) const {
    return classIdentifier() == other.getClassIdentifier();
}
template <typename Key>
auto ConstantInterpolation<Key>::operator()(const std::vector<std::unique_ptr<Key>>& keys,
                                            Seconds from, Seconds to, easing::EasingType) const ->
    typename Key::value_type {

    if (to > from) {
        auto it = std::upper_bound(
            keys.begin(), keys.end(), to,
            [](const auto& time, const auto& key) { return time < key->getTime(); });

        if (it == keys.begin()) {
            return (*it)->getValue();
        } else {
            return (*std::prev(it))->getValue();
        }

    } else {
        auto it = std::upper_bound(
            keys.begin(), keys.end(), to,
            [](const auto& time, const auto& key) { return time < key->getTime(); });

        if (it == keys.end()) {
            return (*std::prev(it))->getValue();
        } else {
            return (*it)->getValue();
        }
    }
}

template <typename Key>
void ConstantInterpolation<Key>::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
}

template <typename Key>
void ConstantInterpolation<Key>::deserialize(Deserializer& d) {
    std::string className;
    d.deserialize("type", className, SerializationTarget::Attribute);
    if (className != getClassIdentifier()) {
        throw SerializationException(
            "Deserialized interpolation: " + getClassIdentifier() +
                " from a serialized interpolation with a different class identifier: " + className,
            IVW_CONTEXT);
    }
}

}  // namespace animation

}  // namespace inviwo

#endif  // IVW_KEYFRAME_CONSTANT_INTERPOLATION_H
