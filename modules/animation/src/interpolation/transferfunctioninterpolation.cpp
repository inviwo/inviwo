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

#include <modules/animation/interpolation/transferfunctioninterpolation.h>

#include <inviwo/core/io/serialization/serializebase.h>
#include <inviwo/core/io/serialization/serializer.h>
#include <inviwo/core/algorithm/easing.h>

#include <inviwo/core/algorithm/optimaltransport.h>

#include <inviwo/core/util/logcentral.h>
#include <modules/animation/datastructures/animationtime.h>
#include <modules/animation/interpolation/interpolation.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iterator>
#include <ratio>
#include <set>

namespace inviwo {
class Deserializer;

namespace animation {

namespace {}  // namespace

TFInterpolationOptimalTransport::TFInterpolationOptimalTransport(InviwoApplication* app)
    : InterpolationTyped<ValueKeyframe<TransferFunction>, TransferFunction>(app)
    , segments{"segments", "Segments", util::ordinalCount(16uz)}
    , simplify{"simplify", "Simplify", util::ordinalLength(0.0, 0.1).setInc(0.0001)} {

    addProperties(segments, simplify);
}

TFInterpolationOptimalTransport::TFInterpolationOptimalTransport(
    const TFInterpolationOptimalTransport& rhs)
    : InterpolationTyped<ValueKeyframe<TransferFunction>, TransferFunction>(rhs)
    , segments{rhs.segments}
    , simplify{rhs.simplify} {

    addProperties(segments, simplify);
}

TFInterpolationOptimalTransport* TFInterpolationOptimalTransport::clone() const {
    return new TFInterpolationOptimalTransport(*this);
}

std::string_view TFInterpolationOptimalTransport::getDisplayName() const {
    return "Optimal Transport";
}

std::string_view TFInterpolationOptimalTransport::getClassIdentifier() const {
    return classIdentifier();
}

bool TFInterpolationOptimalTransport::equal(const Interpolation& other) const {
    return classIdentifier() == other.getClassIdentifier();
}

std::string_view TFInterpolationOptimalTransport::classIdentifier() {
    return "org.inviwo.animation.TFInterpolationOptimalTransport";
}

void TFInterpolationOptimalTransport::operator()(
    const std::vector<std::unique_ptr<ValueKeyframe<TransferFunction>>>& keys, Seconds /*from*/,
    Seconds to, TransferFunction& out) const {

    auto it = std::upper_bound(keys.begin(), keys.end(), to, [](const auto& time, const auto& key) {
        return time < key->getTime();
    });
    const auto& prev = *(*std::prev(it));
    const auto& next = *(*it);

    const auto t1 = prev.getTime();
    const auto t2 = next.getTime();

    const auto& v1 = prev.getValue();
    const auto& v2 = next.getValue();

    const auto easeIn = prev.getEaseIn();
    const auto easeOut = next.getEaseOut();

    const auto t = util::ease(static_cast<double>((to - t1) / (t2 - t1)), easeIn, easeOut);

    const auto interpolated =
        algorithm::optimalTransportInterpolation(v1.get(), v2.get(), t, segments.get());

    if (simplify.get() > 0.0) {
        out.set(TransferFunction::simplify(interpolated, simplify.get()));
    } else {
        out.set(interpolated);
    }
}

TFInterpolationBlend::TFInterpolationBlend(InviwoApplication* app)
    : InterpolationTyped<ValueKeyframe<TransferFunction>, TransferFunction>(app) {}

TFInterpolationBlend::TFInterpolationBlend(const TFInterpolationBlend& rhs)
    : InterpolationTyped<ValueKeyframe<TransferFunction>, TransferFunction>(rhs) {}

TFInterpolationBlend* TFInterpolationBlend::clone() const {
    return new TFInterpolationBlend(*this);
}

std::string_view TFInterpolationBlend::getDisplayName() const { return "Blend"; }

std::string_view TFInterpolationBlend::getClassIdentifier() const { return classIdentifier(); }

bool TFInterpolationBlend::equal(const Interpolation& other) const {
    return classIdentifier() == other.getClassIdentifier();
}

std::string_view TFInterpolationBlend::classIdentifier() {
    return "org.inviwo.animation.TFInterpolationBlend";
}

void TFInterpolationBlend::operator()(
    const std::vector<std::unique_ptr<ValueKeyframe<TransferFunction>>>& keys, Seconds /*from*/,
    Seconds to, TransferFunction& out) const {

    auto it = std::upper_bound(keys.begin(), keys.end(), to, [](const auto& time, const auto& key) {
        return time < key->getTime();
    });
    const auto& prev = *(*std::prev(it));
    const auto& next = *(*it);

    const auto t1 = prev.getTime();
    const auto t2 = next.getTime();

    const auto& v1 = prev.getValue();
    const auto& v2 = next.getValue();

    const auto easeIn = prev.getEaseIn();
    const auto easeOut = next.getEaseOut();

    const auto t = util::ease(static_cast<double>((to - t1) / (t2 - t1)), easeIn, easeOut);

    std::set<double> positions;
    positions.insert_range(v1.get() |
                           std::views::transform([](const auto& item) { return item.pos; }));
    positions.insert_range(v2.get() |
                           std::views::transform([](const auto& item) { return item.pos; }));

    const auto interpolated =
        positions | std::views::transform([&](double pos) {
            return TFPrimitiveData{pos, glm::mix(v1.sample(pos), v2.sample(pos), t)};
        }) |
        std::ranges::to<std::vector>();

    out.set(interpolated);
}

}  // namespace animation

}  // namespace inviwo
