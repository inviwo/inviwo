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

#ifndef IVW_LINEAR_INTERPOLATION_H
#define IVW_LINEAR_INTERPOLATION_H

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/animation/interpolation/interpolation.h>

#include <algorithm>

namespace inviwo {

namespace animation {

/** \class LinearInterpolation
 * Linear interpolation function for key frames.
 * Perfoms linear interpolation between two neighboring key frames.
 */
template <typename Key>
class LinearInterpolation : public InterpolationTyped<Key> {
public:
    LinearInterpolation() = default;
    virtual ~LinearInterpolation() = default;

    virtual LinearInterpolation<Key>* clone() const override;

    virtual std::string getName() const override;

    static std::string classIdentifier();
    virtual std::string getClassIdentifier() const override;

    virtual bool equal(const Interpolation& other) const override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    /*
     * Returns linear interpolation of keyframe values at time t.
     */
    virtual auto operator()(const std::vector<std::unique_ptr<Key>>& keys, Seconds from, Seconds to,
                            easing::EasingType easing) const -> typename Key::value_type override;
};

template <typename Key>
LinearInterpolation<Key>* LinearInterpolation<Key>::clone() const {
    return new LinearInterpolation<Key>(*this);
}

template <typename Key>
std::string LinearInterpolation<Key>::getName() const {
    return "Linear";
}

template <typename Key>
std::string LinearInterpolation<Key>::getClassIdentifier() const {
    return classIdentifier();
}

template <typename Key>
bool LinearInterpolation<Key>::equal(const Interpolation& other) const {
    return classIdentifier() == other.getClassIdentifier();
}

template <typename Key>
std::string LinearInterpolation<Key>::classIdentifier() {
    return "org.inviwo.animation.linearinterpolation." +
           Defaultvalues<typename Key::value_type>::getName();
}

template <typename Key>
auto LinearInterpolation<Key>::operator()(const std::vector<std::unique_ptr<Key>>& keys,
                                          Seconds /*from*/, Seconds to,
                                          easing::EasingType easing) const ->
    typename Key::value_type {

    using VT = typename Key::value_type;
    using DT = typename util::same_extent<VT, double>::type;

    auto it = std::upper_bound(keys.begin(), keys.end(), to, [](const auto& time, const auto& key) {
        return time < key->getTime();
    });

    const auto& v1 = (*std::prev(it))->getValue();
    const auto& t1 = (*std::prev(it))->getTime();

    const auto& v2 = (*it)->getValue();
    const auto& t2 = (*it)->getTime();

    // We have to take special care here since we might have unsigned types.
    // Lets just convert everything to doubles.
    const auto dv1 = static_cast<DT>(v1);
    const auto dv2 = static_cast<DT>(v2);

    return static_cast<VT>(glm::mix(dv1, dv2, easing::ease((to - t1) / (t2 - t1), easing)));
}

template <typename Key>
void LinearInterpolation<Key>::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
}

template <typename Key>
void LinearInterpolation<Key>::deserialize(Deserializer&) {}

}  // namespace animation

}  // namespace inviwo

#endif  // IVW_LINEAR_INTERPOLATION_H
