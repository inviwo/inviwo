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

namespace inviwo {
class Deserializer;

namespace animation {

namespace {}  // namespace

TransferFunctionInterpolation* TransferFunctionInterpolation::clone() const {
    return new TransferFunctionInterpolation(*this);
}

std::string TransferFunctionInterpolation::getName() const { return "Optimal Transport"; }

std::string_view TransferFunctionInterpolation::getClassIdentifier() const {
    return classIdentifier();
}

bool TransferFunctionInterpolation::equal(const Interpolation& other) const {
    return classIdentifier() == other.getClassIdentifier();
}

std::string_view TransferFunctionInterpolation::classIdentifier() {
    return "org.inviwo.animation.TransferFunctionInterpolation";
}

void TransferFunctionInterpolation::operator()(
    const std::vector<std::unique_ptr<ValueKeyframe<TransferFunction>>>& keys, Seconds /*from*/,
    Seconds to, Easing easing, TransferFunction& out) const {

    auto it = std::upper_bound(keys.begin(), keys.end(), to, [](const auto& time, const auto& key) {
        return time < key->getTime();
    });

    const auto& v1 = *(*std::prev(it));
    const auto& t1 = (*std::prev(it))->getTime();

    const auto& v2 = *(*it);
    const auto& t2 = (*it)->getTime();

    const auto t = static_cast<double>((to - t1) / (t2 - t1));

    const auto interpolated = algorithm::optimalTransportInterpolation(
        v1.getValue().get(), v2.getValue().get(), util::ease(t, easing));

    out.set(interpolated);
}

void TransferFunctionInterpolation::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
}

void TransferFunctionInterpolation::deserialize(Deserializer&) {}

}  // namespace animation

}  // namespace inviwo
