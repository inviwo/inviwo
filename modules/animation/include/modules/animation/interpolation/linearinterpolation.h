/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2026 Inviwo Foundation
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
#include <inviwo/core/io/serialization/serialization.h>
#include <inviwo/core/algorithm/easing.h>
#include <inviwo/core/properties/optionproperty.h>

#include <algorithm>

#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/defaultvalues.h>

namespace inviwo {

class InviwoApplication;

namespace animation {

/**
 * Linear interpolation function for key frames.
 * Performs linear interpolation between two neighboring key frames.
 * Easing is controlled via the `easingType` and `easingMode` properties.
 */
template <typename Key, typename Result = typename Key::value_type>
class LinearInterpolation : public InterpolationTyped<Key, Result> {
public:
    explicit LinearInterpolation(InviwoApplication* app = nullptr);
    virtual ~LinearInterpolation() = default;

    LinearInterpolation(const LinearInterpolation&);
    LinearInterpolation& operator=(const LinearInterpolation&) = delete;

    virtual LinearInterpolation<Key, Result>* clone() const override;

    virtual std::string getName() const override;

    static std::string_view classIdentifier();
    virtual std::string_view getClassIdentifier() const override;

    virtual bool equal(const Interpolation& other) const override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    /*
     * Returns linear interpolation of keyframe values at time t.
     */
    virtual void operator()(const std::vector<std::unique_ptr<Key>>& keys, Seconds from, Seconds to,
                            Result& out) const override;

private:
    OptionProperty<EasingType> easingType_{"easingType", "Easing Type"};
    OptionProperty<EasingMode> easingMode_{"easingMode", "Easing Mode"};
};

template <typename Key, typename Result>
LinearInterpolation<Key, Result>::LinearInterpolation(InviwoApplication* app)
    : InterpolationTyped<Key, Result>(app, std::string(classIdentifier())) {
    for (size_t i = 0; i < Easing::typeCount; ++i) {
        auto type = static_cast<EasingType>(i);
        std::string id(format_as(type));
        easingType_.addOption(id, id, type);
    }
    easingType_.setCurrentStateAsDefault();

    for (size_t i = 0; i < Easing::modeCount; ++i) {
        auto mode = static_cast<EasingMode>(i);
        std::string id(format_as(mode));
        easingMode_.addOption(id, id, mode);
    }
    easingMode_.setSelectedValue(EasingMode::inOut);
    easingMode_.setCurrentStateAsDefault();

    this->addProperties(easingType_, easingMode_);
}

template <typename Key, typename Result>
LinearInterpolation<Key, Result>::LinearInterpolation(const LinearInterpolation& rhs)
    : InterpolationTyped<Key, Result>(rhs), easingType_(rhs.easingType_), easingMode_(rhs.easingMode_) {
    // The PropertyOwner copy constructor copies the property ownership structure, but does not
    // re-register member properties (easingType_, easingMode_) that are part of this class.
    // They are initialized via the member initializer list, so we must explicitly add them back.
    this->addProperties(easingType_, easingMode_);
}

template <typename Key, typename Result>
LinearInterpolation<Key, Result>* LinearInterpolation<Key, Result>::clone() const {
    return new LinearInterpolation<Key, Result>(*this);
}

template <typename Key, typename Result>
std::string LinearInterpolation<Key, Result>::getName() const {
    return "Linear";
}

template <typename Key, typename Result>
std::string_view LinearInterpolation<Key, Result>::getClassIdentifier() const {
    return classIdentifier();
}

template <typename Key, typename Result>
bool LinearInterpolation<Key, Result>::equal(const Interpolation& other) const {
    return classIdentifier() == other.getClassIdentifier();
}

template <typename Key, typename Result>
std::string_view LinearInterpolation<Key, Result>::classIdentifier() {
    static const auto cid = "org.inviwo.animation.linearinterpolation." +
                            Defaultvalues<typename Key::value_type>::getName();
    return cid;
}

template <typename Key, typename Result>
void LinearInterpolation<Key, Result>::operator()(const std::vector<std::unique_ptr<Key>>& keys,
                                                  Seconds /*from*/, Seconds to,
                                                  Result& out) const {

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

    const Easing easing{easingType_.get(), easingMode_.get()};
    out = static_cast<VT>(glm::mix(dv1, dv2, util::ease((to - t1) / (t2 - t1), easing)));
}

template <typename Key, typename Result>
void LinearInterpolation<Key, Result>::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
    PropertyOwner::serialize(s);
}

template <typename Key, typename Result>
void LinearInterpolation<Key, Result>::deserialize(Deserializer& d) {
    PropertyOwner::deserialize(d);
}

}  // namespace animation

}  // namespace inviwo
