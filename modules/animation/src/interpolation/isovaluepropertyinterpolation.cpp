/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/animation/interpolation/isovaluepropertyinterpolation.h>

#include <inviwo/core/datastructures/tfprimitive.h>

#include <algorithm>
#include <cmath>
#include <tuple>
#include <vector>

namespace inviwo::animation {
namespace {

TFPrimitiveData fade(const TFPrimitive& primitive, double t) {
    auto color = primitive.getColor();
    color.a *= static_cast<float>(t);
    return {primitive.getPosition(), color};
}

TFPrimitiveData fadeIn(const TFPrimitive& primitive, double t) { return fade(primitive, t); }

TFPrimitiveData fadeOut(const TFPrimitive& primitive, double t) { return fade(primitive, 1.0 - t); }

TFPrimitiveData interpolate(const TFPrimitive& source, const TFPrimitive& destination, double t) {
    return {std::lerp(source.getPosition(), destination.getPosition(), t),
            glm::mix(source.getColor(), destination.getColor(), t)};
}

std::vector<std::pair<size_t, size_t>> matchEqualPositions(const IsoValueCollection& source,
                                                           const IsoValueCollection& destination) {
    std::vector<std::pair<size_t, size_t>> matches;
    std::vector<bool> matchedDestination(destination.size(), false);

    for (size_t i = 0; i < source.size(); ++i) {
        for (size_t j = 0; j < destination.size(); ++j) {
            if (!matchedDestination[j] && source[i].getPosition() == destination[j].getPosition()) {
                matches.emplace_back(i, j);
                matchedDestination[j] = true;
                break;
            }
        }
    }
    return matches;
}

std::vector<std::pair<size_t, size_t>> matchPointsByOrder(const IsoValueCollection& source,
                                                          const IsoValueCollection& destination) {
    std::vector<std::pair<size_t, size_t>> matches;
    const auto count = std::min(source.size(), destination.size());
    matches.reserve(count);

    for (size_t i = 0; i < count; ++i) {
        matches.emplace_back(i, i);
    }

    return matches;
}

template <typename Match>
std::vector<TFPrimitiveData> interpolateIsoValues(const IsoValueCollection& source,
                                                  const IsoValueCollection& destination, double t,
                                                  Match match) {
    const auto matches = match(source, destination);

    std::vector<bool> matchedSource(source.size(), false);
    std::vector<bool> matchedDestination(destination.size(), false);
    std::vector<TFPrimitiveData> values;
    values.reserve(source.size() + destination.size() - matches.size());

    for (const auto& [i, j] : matches) {
        values.push_back(interpolate(source[i], destination[j], t));
        matchedSource[i] = true;
        matchedDestination[j] = true;
    }
    for (size_t i = 0; i < source.size(); ++i) {
        if (!matchedSource[i]) values.push_back(fadeOut(source[i], t));
    }
    for (size_t j = 0; j < destination.size(); ++j) {
        if (!matchedDestination[j]) values.push_back(fadeIn(destination[j], t));
    }
    return values;
}

template <typename Value, typename InterpolateValue>
void interpolatePropertyValue(const std::vector<std::unique_ptr<ValueKeyframe<Value>>>& keys, Seconds to,
                              Value& out, InterpolateValue interpolateValue) {

    auto it = std::upper_bound(keys.begin(), keys.end(), to, [](const auto& time, const auto& key) {
        return time < key->getTime();
    });
    const auto& prev = *(*std::prev(it));
    const auto& next = *(*it);

    interpolateValue(prev, next, to, out);
}

}  // namespace

namespace detail {

double interpolationTime(Seconds t1, Seconds t2, std::optional<EasingType> easeIn,
                         std::optional<EasingType> easeOut, Seconds to) {
    return util::ease(static_cast<double>((to - t1) / (t2 - t1)), easeIn, easeOut);
}

void interpolateIsoValuesFade(const IsoValueCollection& source, const IsoValueCollection& destination,
                               double t, IsoValueCollection& out) {
    if (source.getType() != destination.getType()) {
        out = source;
        return;
    }

    out = IsoValueCollection{interpolateIsoValues(source, destination, t, matchEqualPositions),
                             source.getType()};
}

void interpolateIsoValuesBlend(const IsoValueCollection& source,
                                          const IsoValueCollection& destination, double t,
                                          IsoValueCollection& out) {
    if (source.getType() != destination.getType()) {
        out = source;
        return;
    }

    out = IsoValueCollection{interpolateIsoValues(source, destination, t, matchPointsByOrder),
                             source.getType()};
}

}  // namespace detail

IsoValuePropertyInterpolationFade::IsoValuePropertyInterpolationFade(InviwoApplication* app)
    : InterpolationTyped<ValueKeyframe<IsoValueProperty::value_type>,
                         IsoValueProperty::value_type>(app) {}

IsoValuePropertyInterpolationFade::IsoValuePropertyInterpolationFade(
    const IsoValuePropertyInterpolationFade& rhs)
    : InterpolationTyped<ValueKeyframe<IsoValueProperty::value_type>,
                         IsoValueProperty::value_type>(rhs) {}

IsoValuePropertyInterpolationFade* IsoValuePropertyInterpolationFade::clone() const {
    return new IsoValuePropertyInterpolationFade(*this);
}

std::string_view IsoValuePropertyInterpolationFade::getDisplayName() const { return "Fade"; }

std::string_view IsoValuePropertyInterpolationFade::classIdentifier() {
    return "org.inviwo.animation.IsoValuePropertyInterpolationFade";
}

std::string_view IsoValuePropertyInterpolationFade::getClassIdentifier() const {
    return classIdentifier();
}

bool IsoValuePropertyInterpolationFade::equal(const Interpolation& other) const {
    return classIdentifier() == other.getClassIdentifier();
}

void IsoValuePropertyInterpolationFade::operator()(
    const std::vector<std::unique_ptr<ValueKeyframe<IsoValueProperty::value_type>>>& keys,
    Seconds /*from*/, Seconds to, IsoValueProperty::value_type& out) const {
    interpolatePropertyValue(keys, to, out, [](const auto& prev, const auto& next, Seconds time,
                                               IsoValueCollection& value) {
        detail::interpolateIsoValuesFade(prev.getValue(), next.getValue(),
                                          detail::interpolationTime(prev.getTime(), next.getTime(),
                                                                    prev.getEaseIn(),
                                                                    next.getEaseOut(), time),
                                          value);
    });
}

IsoValuePropertyInterpolationBlend::IsoValuePropertyInterpolationBlend(
    InviwoApplication* app)
    : InterpolationTyped<ValueKeyframe<IsoValueProperty::value_type>,
                         IsoValueProperty::value_type>(app)
    , segments{"segments", "Segments", util::ordinalCount(16uz)}
    , simplify{"simplify", "Simplify", util::ordinalLength(0.0, 0.1).setInc(0.0001)} {
    addProperties(segments, simplify);
}

IsoValuePropertyInterpolationBlend::IsoValuePropertyInterpolationBlend(
    const IsoValuePropertyInterpolationBlend& rhs)
    : InterpolationTyped<ValueKeyframe<IsoValueProperty::value_type>,
                         IsoValueProperty::value_type>(rhs)
    , segments{rhs.segments}
    , simplify{rhs.simplify} {
    addProperties(segments, simplify);
}

IsoValuePropertyInterpolationBlend*
IsoValuePropertyInterpolationBlend::clone() const {
    return new IsoValuePropertyInterpolationBlend(*this);
}

std::string_view IsoValuePropertyInterpolationBlend::getDisplayName() const {
    return "Blend";
}

std::string_view IsoValuePropertyInterpolationBlend::classIdentifier() {
    return "org.inviwo.animation.IsoValuePropertyInterpolationBlend";
}

std::string_view IsoValuePropertyInterpolationBlend::getClassIdentifier() const {
    return classIdentifier();
}

bool IsoValuePropertyInterpolationBlend::equal(const Interpolation& other) const {
    return classIdentifier() == other.getClassIdentifier();
}

void IsoValuePropertyInterpolationBlend::operator()(
    const std::vector<std::unique_ptr<ValueKeyframe<IsoValueProperty::value_type>>>& keys,
    Seconds /*from*/, Seconds to, IsoValueProperty::value_type& out) const {
    interpolatePropertyValue(keys, to, out, [](const auto& prev, const auto& next, Seconds time,
                                               IsoValueCollection& value) {
        detail::interpolateIsoValuesBlend(
            prev.getValue(), next.getValue(),
            detail::interpolationTime(prev.getTime(), next.getTime(), prev.getEaseIn(),
                                      next.getEaseOut(), time),
            value);
    });
}

}  // namespace inviwo::animation
