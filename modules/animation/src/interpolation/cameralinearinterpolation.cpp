/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2026 Inviwo Foundation
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

#include <inviwo/core/io/serialization/serializebase.h>
#include <inviwo/core/io/serialization/serializer.h>
#include <inviwo/core/io/serialization/deserializer.h>
#include <inviwo/core/util/glmvec.h>
#include <modules/animation/datastructures/animationtime.h>
#include <modules/animation/datastructures/camerakeyframe.h>
#include <modules/animation/interpolation/interpolation.h>

#include <algorithm>
#include <chrono>
#include <iterator>
#include <ratio>

#include <glm/common.hpp>
#include <glm/detail/qualifier.hpp>
#include <glm/detail/type_quat.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/vec3.hpp>

namespace inviwo {

namespace animation {

CameraLinearInterpolation::CameraLinearInterpolation(InviwoApplication* app)
    : InterpolationTyped<CameraKeyframe, CameraKeyframe::value_type>(app, classIdentifier()) {
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

    addProperties(easingType_, easingMode_);
}

CameraLinearInterpolation::CameraLinearInterpolation(const CameraLinearInterpolation& rhs)
    : InterpolationTyped<CameraKeyframe, CameraKeyframe::value_type>(rhs)
    , easingType_(rhs.easingType_)
    , easingMode_(rhs.easingMode_) {
    addProperties(easingType_, easingMode_);
}

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
                                           Seconds /*from*/, Seconds to,
                                           CameraKeyframe::value_type& out) const {

    auto it = std::upper_bound(keys.begin(), keys.end(), to, [](const auto& time, const auto& key) {
        return time < key->getTime();
    });

    const auto& v1 = *(*std::prev(it));
    const auto& t1 = (*std::prev(it))->getTime();

    const auto& v2 = *(*it);
    const auto& t2 = (*it)->getTime();

    const Easing easing{easingType_.get(), easingMode_.get()};
    auto t = util::ease((to - t1) / (t2 - t1), easing);

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
    PropertyOwner::serialize(s);
}

void CameraLinearInterpolation::deserialize(Deserializer& d) {
    PropertyOwner::deserialize(d);
}

}  // namespace animation

}  // namespace inviwo
