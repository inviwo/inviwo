/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <modules/animation/interpolation/cameralinearinterpolation.h>
#include <glm/gtc/quaternion.hpp>

namespace inviwo {

namespace animation {

CameraLinearInterpolation* CameraLinearInterpolation::clone() const {
    return new CameraLinearInterpolation(*this);
}

std::string CameraLinearInterpolation::getName() const { return "Linear"; }

std::string CameraLinearInterpolation::getClassIdentifier() const { return classIdentifier(); }

bool CameraLinearInterpolation::equal(const Interpolation& other) const {
    return classIdentifier() == other.getClassIdentifier();
}

std::string CameraLinearInterpolation::classIdentifier() {
    return "org.inviwo.animation.cameralinearinterpolation";
}

void CameraLinearInterpolation::operator()(const std::vector<std::unique_ptr<CameraKeyframe>>& keys,
                                           Seconds /*from*/, Seconds to, easing::EasingType easing,
                                           CameraKeyframe::value_type& out) const {

    auto it = std::upper_bound(keys.begin(), keys.end(), to, [](const auto& time, const auto& key) {
        return time < key->getTime();
    });

    const auto& v1 = *(*std::prev(it));
    const auto& t1 = (*std::prev(it))->getTime();

    const auto& v2 = *(*it);
    const auto& t2 = (*it)->getTime();

    auto t = easing::ease((to - t1) / (t2 - t1), easing);
    auto fromDir = glm::normalize(v1.getDirection());

    auto lookTo = glm::mix(dvec3(v1.getLookTo()), dvec3(v2.getLookTo()), t);
    auto lookFrom = glm::mix(dvec3(v1.getLookFrom()), dvec3(v2.getLookFrom()), t);
    // Assume that lookUp vectors are normalized
    auto lookUpQ = glm::slerp(glm::quat_identity<double, glm::defaultp>(),
                              glm::rotation(dvec3(v1.getLookUp()), dvec3(v2.getLookUp())), t);
    auto lookUp = glm::normalize(lookUpQ * dvec3(v1.getLookUp()));

    out.setLookFrom(lookFrom);
    out.setLookTo(lookTo);
    out.setLookUp(lookUp);
}

void CameraLinearInterpolation::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
}

void CameraLinearInterpolation::deserialize(Deserializer&) {}

}  // namespace animation

}  // namespace inviwo
