/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2021 Inviwo Foundation
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

#include <inviwo/core/datastructures/camera/skewedperspectivecamera.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/ordinalrefproperty.h>
#include <inviwo/core/io/serialization/serialization.h>

#include <inviwo/core/datastructures/camera/cameratools.h>
#include <inviwo/core/datastructures/camera/orthographiccamera.h>
#include <inviwo/core/datastructures/camera/perspectivecamera.h>

#include <memory>
#include <utility>

namespace inviwo {

SkewedPerspectiveCamera::SkewedPerspectiveCamera(vec3 lookFrom, vec3 lookTo, vec3 lookUp,
                                                 float nearPlane, float farPlane, float fieldOfView,
                                                 float aspectRatio, vec2 offset)
    : Camera(lookFrom, lookTo, lookUp, nearPlane, farPlane, aspectRatio)
    , fovy_(fieldOfView)
    , offset_(offset) {}

SkewedPerspectiveCamera::SkewedPerspectiveCamera(const SkewedPerspectiveCamera&) = default;
SkewedPerspectiveCamera& SkewedPerspectiveCamera::operator=(const SkewedPerspectiveCamera&) =
    default;

SkewedPerspectiveCamera* SkewedPerspectiveCamera::clone() const {
    return new SkewedPerspectiveCamera(*this);
}

std::string SkewedPerspectiveCamera::getClassIdentifier() const { return classIdentifier; }

const std::string SkewedPerspectiveCamera::classIdentifier = "SkewedPerspectiveCamera";

void SkewedPerspectiveCamera::setLookFrom(vec3 val) {
    Camera::setLookFrom(val);
    invalidateProjectionMatrix();
}

void SkewedPerspectiveCamera::setLookTo(vec3 val) {
    Camera::setLookTo(val);
    invalidateProjectionMatrix();
}

void SkewedPerspectiveCamera::setFovy(float val) {
    if (fovy_ != val) {
        fovy_ = val;
        invalidateProjectionMatrix();
        if (camprop_) {
            if (auto p = util::getCameraFovProperty(*camprop_)) {
                p->propertyModified();
            }
        }
    }
}

void SkewedPerspectiveCamera::setOffset(vec2 offset) {
    if (offset_ != offset) {
        offset_ = offset;
        invalidateViewMatrix();
        invalidateProjectionMatrix();
        if (camprop_) {
            if (auto p = util::getCameraEyeOffsetProperty(*camprop_)) {
                p->propertyModified();
            }
        }
    }
}

void SkewedPerspectiveCamera::zoom(float factor, std::optional<mat4> boundingBox) {
    setLookFrom(util::perspectiveZoom(*this, factor, boundingBox));
}

void SkewedPerspectiveCamera::updateFrom(const Camera& source) {
    Camera::updateFrom(source);
    if (auto sc = dynamic_cast<const SkewedPerspectiveCamera*>(&source)) {
        setFovy(sc->getFovy());
        setOffset(sc->getOffset());
    } else if (auto pc = dynamic_cast<const PerspectiveCamera*>(&source)) {
        setFovy(pc->getFovy());
    } else if (auto oc = dynamic_cast<const OrthographicCamera*>(&source)) {
        const auto dist = util::widthToViewDist(oc->getWidth(), getFovy(), getAspectRatio());
        setLookFrom(getLookTo() + dist * glm::normalize(getLookFrom() - getLookTo()));
    }
}

void SkewedPerspectiveCamera::configureProperties(CameraProperty& cp, bool attach) {
    Camera::configureProperties(cp, attach);

    if (attach) {
        util::updateOrCreateCameraFovProperty(
            cp, [this]() { return getFovy(); },
            [this](const float& val) {
                if (fovy_ != val) {
                    fovy_ = val;
                    invalidateProjectionMatrix();
                }
            });

        util::updateOrCreateCameraEyeOffsetProperty(
            cp, [this]() { return getOffset(); },
            [this](const vec2& val) {
                if (offset_ != val) {
                    offset_ = val;
                    invalidateViewMatrix();
                    invalidateProjectionMatrix();
                }
            });

    } else {
        if (auto fov = util::getCameraFovProperty(cp)) {
            fov->setGetAndSet([val = fov->get()]() { return val; }, [](const float&) {});
        }
        if (auto offset = util::getCameraEyeOffsetProperty(cp)) {
            offset->setGetAndSet([val = offset->get()]() { return val; }, [](const vec2&) {});
        }
    }
}

mat4 SkewedPerspectiveCamera::calculateViewMatrix() const {
    const vec3 xoffset{offset_.x * glm::normalize(glm::cross(lookTo_ - lookFrom_, lookUp_))};
    const vec3 yoffset{offset_.y * lookUp_};
    return glm::lookAt(lookFrom_ + xoffset + yoffset, lookTo_ + xoffset + yoffset, lookUp_);
}

bool SkewedPerspectiveCamera::equal(const Camera& other) const {
    if (auto rhs = dynamic_cast<const SkewedPerspectiveCamera*>(&other)) {
        return equalTo(other) && fovy_ == rhs->fovy_ && glm::all(glm::equal(offset_, rhs->offset_));
    } else {
        return false;
    }
}

mat4 SkewedPerspectiveCamera::calculateProjectionMatrix() const {
    const float halfHeight = nearPlaneDist_ * std::tan(0.5f * glm::radians(fovy_));
    const float halfWidth = halfHeight * aspectRatio_;

    const float scale = nearPlaneDist_ / glm::distance(lookTo_, lookFrom_);

    // Move the frustum in the opposite direction as the lookFrom.
    const float left = -halfWidth - offset_.x * scale;
    const float right = +halfWidth - offset_.x * scale;

    const float bottom = -halfHeight - offset_.y * scale;
    const float top = +halfHeight - offset_.y * scale;

    return glm::frustum(left, right, bottom, top, nearPlaneDist_, farPlaneDist_);
}

void SkewedPerspectiveCamera::serialize(Serializer& s) const {
    Camera::serialize(s);
    s.serialize("fov", fovy_);
    s.serialize("offset", offset_);
}
void SkewedPerspectiveCamera::deserialize(Deserializer& d) {
    d.deserialize("fov", fovy_);
    d.deserialize("offset", offset_);
    Camera::deserialize(d);
}

}  // namespace inviwo
