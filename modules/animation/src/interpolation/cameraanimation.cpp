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

dvec3 rotationAxis(CameraAnimation::RotationAxis rot, dvec3 dir, dvec3 up) {
    static constexpr std::array axes{dvec3{1.0, 0.0, 0.0}, dvec3{0.0, 1.0, 0.0},
                                     dvec3{0.0, 0.0, 1.0}};
    static constexpr std::array<size_t, 3> order{0, 1, 2};

    static constexpr auto toObject = [](const dvec3& camdir) {
        const auto ind = std::ranges::max(order, std::ranges::less{},
                                          [&](size_t i) { return std::abs(camdir[i]); });
        return axes[ind] * std::copysign(1.0, glm::dot(camdir, axes[ind]));
    };

    switch (rot) {
        case CameraAnimation::RotationAxis::CameraYaw:
            return up;
        case CameraAnimation::RotationAxis::CameraPitch:
            return glm::cross(dir, up);
        case CameraAnimation::RotationAxis::CameraRoll:
            return dir;

        case CameraAnimation::RotationAxis::ObjectYaw:
            return toObject(up);
        case CameraAnimation::RotationAxis::ObjectPitch:
            return toObject(glm::cross(dir, up));
        case CameraAnimation::RotationAxis::ObjectRoll:
            return toObject(dir);

        case CameraAnimation::RotationAxis::WorldX:
            return axes[0];
        case CameraAnimation::RotationAxis::WorldY:
            return axes[1];
        case CameraAnimation::RotationAxis::WorldZ:
            return axes[2];

        default:
            return axes[0];
    }
}

}  // namespace

CameraAnimation::CameraAnimation(InviwoApplication* app)
    : InterpolationTyped<CameraKeyframe, CameraKeyframe::value_type>(app)
    , amplitude{"amplitude", "Amplitude",
                util::ordinalLength(15.0, 180.0).set("Maximum rotation angle in degrees"_help)}
    , periods{"periods", "Periods",
              util::ordinalLength(1.0, 20.0).setInc(0.01).set(
                  "Number of periods during the animation"_help)}
    , axis{"axis",
           "Axis",
           {RotationAxis::CameraYaw, RotationAxis::CameraPitch, RotationAxis::CameraRoll,
            RotationAxis::ObjectYaw, RotationAxis::ObjectPitch, RotationAxis::ObjectRoll,
            RotationAxis::WorldX, RotationAxis::WorldY, RotationAxis::WorldZ}} {

    addProperties(amplitude, periods, axis);
}

CameraAnimation::CameraAnimation(const CameraAnimation& rhs)
    : InterpolationTyped<CameraKeyframe, CameraKeyframe::value_type>(rhs)
    , amplitude{rhs.amplitude}
    , periods{rhs.periods}
    , axis{rhs.axis} {

    addProperties(amplitude, periods, axis);
}

CameraAnimation* CameraAnimation::clone() const { return new CameraAnimation(*this); }

std::string_view CameraAnimation::getDisplayName() const { return "Wiggle"; }

std::string_view CameraAnimation::getClassIdentifier() const { return classIdentifier(); }

bool CameraAnimation::equal(const Interpolation& other) const {
    return classIdentifier() == other.getClassIdentifier();
}

std::string_view CameraAnimation::classIdentifier() {
    return "org.inviwo.animation.cameraanimation";
}

void CameraAnimation::operator()(const std::vector<std::unique_ptr<CameraKeyframe>>& keys,
                                 Seconds /*from*/, Seconds to,
                                 CameraKeyframe::value_type& out) const {
    auto it = std::upper_bound(keys.begin(), keys.end(), to, [](const auto& time, const auto& key) {
        return time < key->getTime();
    });

    const auto& prev = *(*std::prev(it));
    const auto& next = *(*it);

    const auto t1 = prev.getTime();
    const auto t2 = next.getTime();

    const auto easeIn = prev.getEaseIn();
    const auto easeOut = next.getEaseOut();

    const auto t = static_cast<double>((to - t1) / (t2 - t1));

    const auto freq =
        periods.get() > 0.0 ? 1.0 / periods.get() : std::numeric_limits<double>::max();
    const auto tmod = 2.0 / freq * std::abs(std::abs(std::fmod(t - freq / 4.0, freq)) - freq / 2.0);
    const auto tease = util::ease(tmod, easeIn, easeOut);

    const dvec3 dir = prev.getDirection();
    const dvec3 up = prev.getLookUp();
    const dvec3 rotAxis = rotationAxis(axis.get(), glm::normalize(dir), up);

    const auto angle = glm::radians(amplitude.get()) * 2.0 * (tease - 0.5);

    const auto rotation = dmat3(glm::rotate(-angle, rotAxis));
    const auto lookTo = dvec3{prev.getLookTo()};

    out.setLook(lookTo - rotation * dir, lookTo, rotation * up);
}

}  // namespace animation

}  // namespace inviwo
