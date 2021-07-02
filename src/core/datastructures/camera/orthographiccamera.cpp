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

#include <inviwo/core/datastructures/camera/orthographiccamera.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/ordinalrefproperty.h>
#include <inviwo/core/io/serialization/serialization.h>

#include <inviwo/core/datastructures/camera/cameratools.h>
#include <inviwo/core/datastructures/camera/perspectivecamera.h>
#include <inviwo/core/datastructures/camera/skewedperspectivecamera.h>

namespace inviwo {

OrthographicCamera::OrthographicCamera(vec3 lookFrom, vec3 lookTo, vec3 lookUp, float nearPlane,
                                       float farPlane, float width, float aspectRatio)
    : Camera(lookFrom, lookTo, lookUp, nearPlane, farPlane, aspectRatio), width_{width} {}

OrthographicCamera::OrthographicCamera(const OrthographicCamera&) = default;

OrthographicCamera& OrthographicCamera::operator=(const OrthographicCamera&) = default;

OrthographicCamera* OrthographicCamera::clone() const { return new OrthographicCamera(*this); }

std::string OrthographicCamera::getClassIdentifier() const { return classIdentifier; }

const std::string OrthographicCamera::classIdentifier = "OrthographicCamera";

void OrthographicCamera::setWidth(float width) {
    if (width_ != width) {
        width_ = width;
        invalidateProjectionMatrix();
        if (camprop_) {
            if (auto p = util::getCameraWidthProperty(*camprop_)) {
                p->propertyModified();
            }
        }
    }
}

void OrthographicCamera::zoom(float factor, std::optional<mat4>) {
    setWidth(width_ * (1.0f - factor));
}

void OrthographicCamera::updateFrom(const Camera& source) {
    Camera::updateFrom(source);
    if (auto oc = dynamic_cast<const OrthographicCamera*>(&source)) {
        setWidth(oc->getWidth());
    } else if (auto pc = dynamic_cast<const PerspectiveCamera*>(&source)) {
        setWidth(util::fovyToWidth(pc->getFovy(), glm::distance(getLookTo(), getLookFrom()),
                                   getAspectRatio()));
    } else if (auto sc = dynamic_cast<const SkewedPerspectiveCamera*>(&source)) {
        setWidth(util::fovyToWidth(sc->getFovy(), glm::distance(getLookTo(), getLookFrom()),
                                   getAspectRatio()));
    }
}

void OrthographicCamera::configureProperties(CameraProperty& cp, bool attach) {
    Camera::configureProperties(cp, attach);
    if (attach) {
        util::updateOrCreateCameraWidthProperty(
            cp, [this]() { return getWidth(); },
            [this](const float& val) {
                if (width_ != val) {
                    width_ = val;
                    invalidateProjectionMatrix();
                }
            });
    } else if (auto width = util::getCameraWidthProperty(cp)) {
        width->setGetAndSet([val = width->get()]() { return val; }, [](const float&) {});
    }
}

bool OrthographicCamera::equal(const Camera& other) const {
    if (auto rhs = dynamic_cast<const OrthographicCamera*>(&other)) {
        return equalTo(other) && width_ == rhs->width_;
    } else {
        return false;
    }
}

mat4 OrthographicCamera::calculateProjectionMatrix() const {
    const float halfWidth = 0.5f * width_;
    const float halfHeight = halfWidth / aspectRatio_;
    return glm::ortho(-halfWidth, +halfWidth, -halfHeight, +halfHeight, nearPlaneDist_,
                      farPlaneDist_);
}

vec4 OrthographicCamera::getClipPosFromNormalizedDeviceCoords(const vec3& ndcCoords) const {
    return vec4{ndcCoords, 1.0f};
}

void OrthographicCamera::serialize(Serializer& s) const {
    Camera::serialize(s);
    s.serialize("width", width_);
}
void OrthographicCamera::deserialize(Deserializer& d) {
    d.deserialize("width", width_);
    Camera::deserialize(d);
}

}  // namespace inviwo
