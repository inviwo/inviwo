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

#ifndef INVIWO_PROJECTS_SPLINEINTERPOLATION_H
#define INVIWO_PROJECTS_SPLINEINTERPOLATION_H

#include <inviwo/core/common/inviwo.h>
#include <modules/animation/interpolation/interpolation.h>
#include <inviwo/bspline/interpolation/nurbutilities.h>
#include <tinynurbs/tinynurbs.h>
#include <algorithm>

#include <glm/gtc/vec1.hpp>

namespace inviwo::animation {

/** \class SplineInterpolation
 * Interpolation function for key frames based on cubic b-splines with natural end condition.
 * With only two keyframes in the sequence, this is identical to a linear interpolation.
 * With several keyframes in the sequence, a cubic spline will be formed,
 * with zero curvature at either end - perfect for straight line continuation.
 */
template <typename Key>
class SplineInterpolation : public InterpolationTyped<Key> {
public:
    SplineInterpolation() = default;
    virtual ~SplineInterpolation() = default;

    virtual SplineInterpolation* clone() const override;

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
SplineInterpolation<Key>* SplineInterpolation<Key>::clone() const {
    return new SplineInterpolation<Key>(*this);
}

namespace detail {
template <typename T, typename std::enable_if<!std::is_enum<T>::value, int>::type = 0>
std::string getSplineInterpolationClassIdentifier() {
    return "org.inviwo.bspline.splineinterpolation." + Defaultvalues<T>::getName();
}
template <typename T, typename std::enable_if<std::is_enum<T>::value, int>::type = 0>
std::string getSplineInterpolationClassIdentifier() {
    using ET = typename std::underlying_type<T>::type;
    return "org.inviwo.bspline.splineinterpolation.enum." + Defaultvalues<ET>::getName();
}
}  // namespace detail

template <typename Key>
std::string SplineInterpolation<Key>::classIdentifier() {
    return detail::getSplineInterpolationClassIdentifier<typename Key::value_type>();
}

template <typename Key>
std::string SplineInterpolation<Key>::getClassIdentifier() const {
    return classIdentifier();
}

template <typename Key>
std::string SplineInterpolation<Key>::getName() const {
    return "Cubic B-spline";
}

template <typename Key>
bool SplineInterpolation<Key>::equal(const Interpolation& other) const {
    return classIdentifier() == other.getClassIdentifier();
}
template <typename Key>
auto SplineInterpolation<Key>::operator()(const std::vector<std::unique_ptr<Key>>& keys,
                                          Seconds /*from*/, Seconds to, easing::EasingType) const ->
    typename Key::value_type {

    using VT = typename Key::value_type;

    // This will give you a vec type even for float and double as needed by
    // GetInterpolatingNaturalCubicSpline, need to include glm/gtc/vec1.hpp
    // By default inviwo uses float for floats.. not vec<1, float>
    using DT = glm::vec<util::extent<VT>::value, double>;

    // Get ALL keyframe values and times, and build a spline from it
    std::vector<DT> values;
    std::vector<double> times;
    for (const auto& key : keys) {
        values.push_back(static_cast<DT>(key->getValue()));
        times.push_back((key->getTime()).count());
    }

    // Build the spline
    tinynurbs::Curve<DT> spline;
    ::inviwo::util::GetInterpolatingNaturalCubicSpline(values, times, spline);

    // Evaluate the spline
    if constexpr (util::extent<VT>::value == 1) {
        // convert from glm:vec<1,scalar> to scalar, does not work with casting
        return static_cast<VT>(tinynurbs::curvePoint(spline, to.count())[0]);
    } else {
        return static_cast<VT>(tinynurbs::curvePoint(spline, to.count()));
    }
}

template <typename Key>
void SplineInterpolation<Key>::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
}

template <typename Key>
void SplineInterpolation<Key>::deserialize(Deserializer& d) {
    std::string className;
    d.deserialize("type", className, SerializationTarget::Attribute);
    if (className != getClassIdentifier()) {
        throw SerializationException(
            "Deserialized interpolation: " + getClassIdentifier() +
                " from a serialized interpolation with a different class identifier: " + className,
            IvwContext);
    }
}

}  // namespace inviwo::animation

#endif  // INVIWO_PROJECTS_SPLINEINTERPOLATION_H
