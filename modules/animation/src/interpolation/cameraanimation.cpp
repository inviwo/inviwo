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

#include <modules/animation/interpolation/cameraanimation.h>

#include <inviwo/core/io/serialization/serializebase.h>
#include <inviwo/core/io/serialization/serializer.h>
#include <inviwo/core/algorithm/easing.h>
#include <inviwo/core/util/logcentral.h>
#include <modules/animation/datastructures/animationtime.h>
#include <modules/animation/datastructures/camerakeyframe.h>
#include <modules/animation/interpolation/interpolation.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iterator>
#include <ratio>

#include <glm/common.hpp>
#include <glm/detail/qualifier.hpp>
#include <glm/detail/type_quat.hpp>
#include <glm/exponential.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/vec3.hpp>
#include <glm/vector_relational.hpp>

namespace inviwo {
class Deserializer;

namespace animation {

namespace {

dvec3 rotationAxis(CameraAnimation::RotationAxis rot, bool alignToObject,
                   const CameraAnimation::CameraState& cam) {
    const auto camAxis = [&]() {
        switch (rot) {
            case CameraAnimation::RotationAxis::Yaw:
                return cam.up;
            case CameraAnimation::RotationAxis::Pitch:
                return glm::cross(cam.dir, cam.up);
            case CameraAnimation::RotationAxis::Roll:
                return cam.dir;
            default:
                return cam.up;
        }
    }();

    if (alignToObject) {
        std::array<size_t, 3> order{0, 1, 2};
        const auto ind = std::ranges::max(order, std::ranges::less{},
                                          [&](size_t i) { return std::abs(camAxis[i]); });

        static constexpr std::array axes{dvec3{1.0, 0.0, 0.0}, dvec3{0.0, 1.0, 0.0},
                                         dvec3{0.0, 0.0, 1.0}};
        return axes[ind] * std::copysign(1.0, glm::dot(camAxis, axes[ind]));
    } else {
        return camAxis;
    }
}

CameraAnimation::CameraState cameraState(const CameraKeyframe& cam) {
    // save current camera vectors (direction, up) to be able to do absolute rotations
    const vec3 camDir = glm::normalize(cam.getDirection());
    const vec3 camUp = glm::normalize(cam.getLookUp());
    return {.dir = camDir, .up = camUp};
}

}  // namespace

CameraAnimation* CameraAnimation::clone() const { return new CameraAnimation(*this); }

std::string CameraAnimation::getName() const { return "Animate"; }

std::string_view CameraAnimation::getClassIdentifier() const { return classIdentifier(); }

bool CameraAnimation::equal(const Interpolation& other) const {
    return classIdentifier() == other.getClassIdentifier();
}

std::string_view CameraAnimation::classIdentifier() {
    return "org.inviwo.animation.cameraanimation";
}

void CameraAnimation::operator()(const std::vector<std::unique_ptr<CameraKeyframe>>& keys,
                                 Seconds /*from*/, Seconds to, Easing easingType,
                                 CameraKeyframe::value_type& out) const {

    auto it = std::upper_bound(keys.begin(), keys.end(), to, [](const auto& time, const auto& key) {
        return time < key->getTime();
    });

    const auto& v1 = *(*std::prev(it));
    const auto& t1 = (*std::prev(it))->getTime();

    const auto& v2 = *(*it);
    const auto& t2 = (*it)->getTime();

    const auto t = static_cast<double>((to - t1) / (t2 - t1));

    const auto f = 0.2;
    const auto tmod = 2.0 / f * std::abs(std::abs(std::fmod(t - f / 4.0, f)) - f / 2.0);


    auto swing = Swing{.axis = dvec3{0.0, 0.0, 1.0}, //rotationAxis(RotationAxis::Yaw, true, cameraState(v1)),
                       .dir = v1.getDirection(),
                       .up = v1.getLookUp(), 
                       .amplitude = 0.25,
                       .step = 0.01,
                       .current = tmod};

    const auto angle = swing.amplitude * (util::ease(tmod, easingType) - 0.5);

    //log::info("{} {} {}", t, tmod, angle);

    // Rotate LookFrom around LookTo using axis
    const auto rotation = dmat3(glm::rotate(-angle, swing.axis));
    const auto lookTo = dvec3{v1.getLookTo()};

    out.setLook(lookTo - rotation * swing.dir, lookTo, rotation * swing.up);
}

void CameraAnimation::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
}

void CameraAnimation::deserialize(Deserializer&) {}

}  // namespace animation

}  // namespace inviwo
