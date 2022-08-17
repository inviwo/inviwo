/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2022 Inviwo Foundation
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

#include <modules/animation/interpolation/camerasphericalinterpolation.h>

#include <glm/gtc/quaternion.hpp>

namespace inviwo {

namespace animation {

CameraSphericalInterpolation* CameraSphericalInterpolation::clone() const {
    return new CameraSphericalInterpolation(*this);
}

std::string CameraSphericalInterpolation::getName() const { return "Orbit/Pan/Tilt"; }

std::string CameraSphericalInterpolation::getClassIdentifier() const { return classIdentifier(); }

bool CameraSphericalInterpolation::equal(const Interpolation& other) const {
    return classIdentifier() == other.getClassIdentifier();
}

std::string CameraSphericalInterpolation::classIdentifier() {
    return "org.inviwo.animation.camerasphericalinterpolation";
}

void CameraSphericalInterpolation::operator()(
    const std::vector<std::unique_ptr<CameraKeyframe>>& keys, Seconds /*from*/, Seconds to,
    easing::EasingType easing, CameraKeyframe::value_type& out) const {

    auto it = std::upper_bound(keys.begin(), keys.end(), to, [](const auto& time, const auto& key) {
        return time < key->getTime();
    });

    const auto& v1 = *(*std::prev(it));
    const auto& t1 = (*std::prev(it))->getTime();

    const auto& v2 = *(*it);
    const auto& t2 = (*it)->getTime();

    auto t = static_cast<float>(easing::ease((to - t1) / (t2 - t1), easing));

    auto fromDir = glm::normalize(v1.getDirection());
    auto rotation = glm::slerp(glm::quat_identity<float, glm::defaultp>(),
                               glm::rotation(fromDir, glm::normalize(v2.getDirection())), t);

    // Adjust for different direction lengths
    auto d = glm::mix(glm::length(v1.getDirection()), glm::length(v2.getDirection()), t);

    if (glm::any(glm::notEqual(v1.getLookFrom(), v2.getLookFrom()))) {
        // Assume that we are orbiting around lookAt
        auto lookTo = glm::mix(v1.getLookTo(), v2.getLookTo(), t);
        auto lookFrom = lookTo - glm::normalize(rotation * fromDir) * d;
        out.setLookFrom(lookFrom);
        out.setLookTo(lookTo);
    } else {
        // Pan/tilt (lookFrom's are equal)
        const auto& lookFrom = v1.getLookFrom();
        auto lookTo = lookFrom + glm::normalize(rotation * fromDir) * d;
        out.setLookFrom(lookFrom);
        out.setLookTo(lookTo);
    }
    // Assume that lookUp vectors are normalized
    auto lookUpQ = glm::slerp(glm::quat_identity<float, glm::defaultp>(),
                              glm::rotation(v1.getLookUp(), v2.getLookUp()), t);
    auto lookUp = glm::normalize(lookUpQ * v1.getLookUp());
    out.setLookUp(lookUp);
}

void CameraSphericalInterpolation::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
}

void CameraSphericalInterpolation::deserialize(Deserializer&) {}

}  // namespace animation

}  // namespace inviwo
