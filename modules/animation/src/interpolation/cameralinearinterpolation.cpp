/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2024 Inviwo Foundation
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

#include <inviwo/core/io/serialization/serializebase.h>       // for SerializationTarget, Serial...
#include <inviwo/core/io/serialization/serializer.h>          // for Serializer
#include <inviwo/core/util/glmvec.h>                          // for dvec3
#include <modules/animation/datastructures/animationtime.h>   // for Seconds
#include <modules/animation/datastructures/camerakeyframe.h>  // for CameraKeyframe, CameraKeyfr...
#include <modules/animation/datastructures/easing.h>          // for ease, EasingType
#include <modules/animation/interpolation/interpolation.h>    // for Interpolation

#include <algorithm>  // for upper_bound
#include <chrono>     // for operator-, operator<, opera...
#include <iterator>   // for prev
#include <ratio>      // for ratio

#include <glm/common.hpp>                    // for mix
#include <glm/detail/qualifier.hpp>          // for defaultp
#include <glm/detail/type_quat.hpp>          // for operator*, qua::qua<T, Q>
#include <glm/ext/quaternion_common.hpp>     // for slerp
#include <glm/ext/quaternion_geometric.hpp>  // for dot
#include <glm/geometric.hpp>                 // for cross, normalize, dot
#include <glm/gtx/quaternion.hpp>            // for quat_identity, rotation
#include <glm/vec3.hpp>                      // for operator*, operator+

namespace inviwo {
class Deserializer;

namespace animation {

CameraLinearInterpolation* CameraLinearInterpolation::clone() const {
    return new CameraLinearInterpolation(*this);
}

std::string CameraLinearInterpolation::getName() const { return "Linear"; }

std::string_view CameraLinearInterpolation::getClassIdentifier() const { return classIdentifier(); }

bool CameraLinearInterpolation::equal(const Interpolation& other) const {
    return classIdentifier() == other.getClassIdentifier();
}

std::string_view CameraLinearInterpolation::classIdentifier() {
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
