/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <modules/animation/animationmoduledefine.h>
#include <modules/animation/interpolation/interpolation.h>

#include <algorithm>

#include <inviwo/core/util/glm.h>

namespace inviwo {

namespace animation {

/** \class LinearInterpolation
 * Linear interpolation function for key frames.
 * Perfoms linear interpolation between two neighboring key frames.
 */
template <typename Key, typename Result = typename Key::value_type>
class LinearInterpolation : public InterpolationTyped<Key, Result> {
public:
    LinearInterpolation() = default;
    virtual ~LinearInterpolation() = default;

    virtual LinearInterpolation<Key, Result>* clone() const override;

    virtual std::string getName() const override;

    static std::string classIdentifier();
    virtual std::string getClassIdentifier() const override;

    virtual bool equal(const Interpolation& other) const override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    /*
     * Returns linear interpolation of keyframe values at time t.
     */
    virtual void operator()(const std::vector<std::unique_ptr<Key>>& keys, Seconds from, Seconds to,
                            easing::EasingType easing, Result& out) const override;
};

template <typename Key, typename Result>
LinearInterpolation<Key, Result>* LinearInterpolation<Key, Result>::clone() const {
    return new LinearInterpolation<Key, Result>(*this);
}

template <typename Key, typename Result>
std::string LinearInterpolation<Key, Result>::getName() const {
    return "Linear";
}

template <typename Key, typename Result>
std::string LinearInterpolation<Key, Result>::getClassIdentifier() const {
    return classIdentifier();
}

template <typename Key, typename Result>
bool LinearInterpolation<Key, Result>::equal(const Interpolation& other) const {
    return classIdentifier() == other.getClassIdentifier();
}

template <typename Key, typename Result>
std::string LinearInterpolation<Key, Result>::classIdentifier() {
    return "org.inviwo.animation.linearinterpolation." +
           Defaultvalues<typename Key::value_type>::getName();
}

template <typename Key, typename Result>
void LinearInterpolation<Key, Result>::operator()(const std::vector<std::unique_ptr<Key>>& keys,
                                                  Seconds /*from*/, Seconds to,
                                                  easing::EasingType easing, Result& out) const {

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
    const auto& dv1 = static_cast<DT>(v1);
    const auto& dv2 = static_cast<DT>(v2);

    out = static_cast<VT>(glm::mix(dv1, dv2, easing::ease((to - t1) / (t2 - t1), easing)));
}

template <typename Key, typename Result>
void LinearInterpolation<Key, Result>::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
}

template <typename Key, typename Result>
void LinearInterpolation<Key, Result>::deserialize(Deserializer&) {}

}  // namespace animation

}  // namespace inviwo
