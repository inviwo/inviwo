/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2020 Inviwo Foundation
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

#include <inviwo/core/datastructures/camera.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/ordinalrefproperty.h>
#include <inviwo/core/io/serialization/serialization.h>

#include <memory>

namespace inviwo {

Camera::Camera(vec3 lookFrom, vec3 lookTo, vec3 lookUp, float nearPlane, float farPlane,
               float aspectRatio)
    : lookFrom_(lookFrom)
    , lookTo_(lookTo)
    , lookUp_(lookUp)
    , nearPlaneDist_(nearPlane)
    , farPlaneDist_(farPlane)
    , aspectRatio_{aspectRatio}
    , invalidViewMatrix_(true)
    , invalidProjectionMatrix_(true)
    , viewMatrix_{1.0f}
    , projectionMatrix_{1.0f}
    , inverseViewMatrix_{1.0f}
    , inverseProjectionMatrix_{1.0f} {}

const mat4& Camera::getViewMatrix() const {
    if (invalidViewMatrix_) {
        viewMatrix_ = calculateViewMatrix();
        inverseViewMatrix_ = glm::inverse(viewMatrix_);
        invalidViewMatrix_ = false;
    }
    return viewMatrix_;
}

mat4 Camera::calculateViewMatrix() const { return glm::lookAt(lookFrom_, lookTo_, lookUp_); }

const mat4& Camera::getProjectionMatrix() const {
    if (invalidProjectionMatrix_) {
        projectionMatrix_ = calculateProjectionMatrix();
        inverseProjectionMatrix_ = glm::inverse(projectionMatrix_);
        invalidProjectionMatrix_ = false;
    }
    return projectionMatrix_;
}

const mat4& Camera::getInverseViewMatrix() const {
    if (invalidViewMatrix_) getViewMatrix();
    return inverseViewMatrix_;
}

const mat4& Camera::getInverseProjectionMatrix() const {
    if (invalidProjectionMatrix_) getProjectionMatrix();
    return inverseProjectionMatrix_;
}

vec3 Camera::getWorldPosFromNormalizedDeviceCoords(const vec3& ndcCoords) const {
    vec4 clipCoords = getClipPosFromNormalizedDeviceCoords(ndcCoords);
    vec4 eyeCoords = getInverseProjectionMatrix() * clipCoords;
    vec4 worldCoords = getInverseViewMatrix() * eyeCoords;
    worldCoords /= worldCoords.w;
    return vec3(worldCoords);
}

vec4 Camera::getClipPosFromNormalizedDeviceCoords(const vec3& ndcCoords) const {
    const auto& projection = getProjectionMatrix();
    const float clipW = projection[2][3] / (ndcCoords.z - (projection[2][2] / projection[3][2]));
    return vec4(ndcCoords * clipW, clipW);
}

vec3 Camera::getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(
    const vec2& normalizedScreenCoord) const {
    // Default to using focus point for depth
    vec4 lookToClipCoord = getProjectionMatrix() * getViewMatrix() * vec4(getLookTo(), 1.f);
    return vec3(2.f * normalizedScreenCoord - 1.f, lookToClipCoord.z / lookToClipCoord.w);
}

void Camera::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
    s.serialize("lookFrom", lookFrom_);
    s.serialize("lookTo", lookTo_);
    s.serialize("lookUp", lookUp_);
    s.serialize("nearPlaneDist", nearPlaneDist_);
    s.serialize("farPlaneDist", farPlaneDist_);
    s.serialize("aspectRatio", aspectRatio_);
}
void Camera::deserialize(Deserializer& d) {
    d.deserialize("lookFrom", lookFrom_);
    d.deserialize("lookTo", lookTo_);
    d.deserialize("lookUp", lookUp_);
    d.deserialize("nearPlaneDist", nearPlaneDist_);
    d.deserialize("farPlaneDist", farPlaneDist_);
    d.deserialize("aspectRatio", aspectRatio_);
    invalidProjectionMatrix_ = true;
    invalidViewMatrix_ = true;
}

void Camera::updateFrom(const Camera* source) {
    setLookFrom(source->getLookFrom());
    setLookTo(source->getLookTo());
    setLookUp(source->getLookUp());

    setNearPlaneDist(source->getNearPlaneDist());
    setFarPlaneDist(source->getFarPlaneDist());
    setAspectRatio(source->getAspectRatio());
}

bool Camera::equalTo(const Camera& other) const {
    return !(glm::any(glm::notEqual(lookFrom_, other.lookFrom_)) ||
             glm::any(glm::notEqual(lookTo_, other.lookTo_)) ||
             (nearPlaneDist_ != other.nearPlaneDist_) || (farPlaneDist_ != other.farPlaneDist_) ||
             (aspectRatio_ != other.aspectRatio_));
}

PerspectiveCamera::PerspectiveCamera(vec3 lookFrom, vec3 lookTo, vec3 lookUp, float nearPlane,
                                     float farPlane, float fieldOfView, float aspectRatio)
    : Camera(lookFrom, lookTo, lookUp, nearPlane, farPlane, aspectRatio), fovy_(fieldOfView) {}

PerspectiveCamera::PerspectiveCamera(const PerspectiveCamera& other) = default;

PerspectiveCamera& PerspectiveCamera::operator=(const PerspectiveCamera& other) = default;

PerspectiveCamera* PerspectiveCamera::clone() const { return new PerspectiveCamera(*this); }

std::string PerspectiveCamera::getClassIdentifier() const { return classIdentifier; }

const std::string PerspectiveCamera::classIdentifier = "PerspectiveCamera";

void PerspectiveCamera::updateFrom(const Camera* source) {
    Camera::updateFrom(source);
    if (auto pc = dynamic_cast<const PerspectiveCamera*>(source)) {
        setFovy(pc->getFovy());
    } else if (auto oc = dynamic_cast<const OrthographicCamera*>(source)) {
        setFovy(glm::degrees(
            2.0f * std::atan(oc->getWidth() / 2.0f / glm::distance(getLookTo(), getLookFrom()))));
    }
}

void PerspectiveCamera::configureProperties(CameraProperty* comp) {
    auto fov = dynamic_cast<FloatRefProperty*>(comp->getCameraProperty("fov"));

    auto getter = [this]() { return getFovy(); };
    auto setter = [this](const float& val) { setFovy(val); };

    if (fov) {
        fov->setGetAndSet(getter, setter);
    } else {
        fov = new FloatRefProperty("fov", "FOV", getter, setter,
                                   {0.0f, ConstraintBehavior::Immutable},
                                   {180.0f, ConstraintBehavior::Immutable}, 0.1f);
        comp->addCamerapProperty(fov);
    }
    fov->setVisible(true);
}

bool PerspectiveCamera::equal(const Camera& other) const {
    if (auto rhs = dynamic_cast<const PerspectiveCamera*>(&other)) {
        return equalTo(other) && fovy_ == rhs->fovy_;
    } else {
        return false;
    }
}

void PerspectiveCamera::serialize(Serializer& s) const {
    Camera::serialize(s);
    s.serialize("fovy", fovy_);
}
void PerspectiveCamera::deserialize(Deserializer& d) {
    d.deserialize("fovy", fovy_);
    Camera::deserialize(d);
}

OrthographicCamera::OrthographicCamera(vec3 lookFrom, vec3 lookTo, vec3 lookUp, float nearPlane,
                                       float farPlane, float width, float aspectRatio)
    : Camera(lookFrom, lookTo, lookUp, nearPlane, farPlane, aspectRatio), width_{width} {}

OrthographicCamera::OrthographicCamera(const OrthographicCamera&) = default;

OrthographicCamera& OrthographicCamera::operator=(const OrthographicCamera&) = default;

OrthographicCamera* OrthographicCamera::clone() const { return new OrthographicCamera(*this); }

std::string OrthographicCamera::getClassIdentifier() const { return classIdentifier; }

const std::string OrthographicCamera::classIdentifier = "OrthographicCamera";

void OrthographicCamera::updateFrom(const Camera* source) {
    Camera::updateFrom(source);
    if (auto oc = dynamic_cast<const OrthographicCamera*>(source)) {
        setWidth(oc->getWidth());
    } else if (auto pc = dynamic_cast<const PerspectiveCamera*>(source)) {
        setWidth(glm::distance(getLookTo(), getLookFrom()) *
                 std::tan(0.5f * glm::radians(pc->getFovy())));
    }
}

void OrthographicCamera::configureProperties(CameraProperty* comp) {
    auto width = dynamic_cast<FloatRefProperty*>(comp->getCameraProperty("width"));
    
    auto getter = [this]() { return getWidth(); };
    auto setter = [this](const float& val) { setWidth(val); };

    if (width) {
        width->setGetAndSet(getter, setter);
    } else {
        width = new FloatRefProperty("width", "Width", getter, setter,
                                     {0.0f, ConstraintBehavior::Immutable},
                                     {1000.0f, ConstraintBehavior::Ignore}, 0.1f);
        comp->addCamerapProperty(width);
    }
    width->setVisible(true);
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

void SkewedPerspectiveCamera::updateFrom(const Camera* source) {
    Camera::updateFrom(source);
    if (auto sc = dynamic_cast<const SkewedPerspectiveCamera*>(source)) {
        setFovy(sc->getFovy());
        setOffset(sc->getOffset());
    } else if (auto pc = dynamic_cast<const PerspectiveCamera*>(source)) {
        setFovy(pc->getFovy());
    } else if (auto oc = dynamic_cast<const OrthographicCamera*>(source)) {
        setFovy(glm::degrees(
            2.0f * std::atan(oc->getWidth() / 2.0f / glm::distance(getLookTo(), getLookFrom()))));
    }
}

void SkewedPerspectiveCamera::configureProperties(CameraProperty* comp) {
    auto fov = dynamic_cast<FloatRefProperty*>(comp->getCameraProperty("fov"));
    auto offset = dynamic_cast<FloatVec2RefProperty*>(comp->getCameraProperty("separation"));

    auto fovGetter = [this]() { return getFovy(); };
    auto fovSetter = [this](const float& val) { setFovy(val); };

    auto offGetter = [this]() { return getOffset(); };
    auto offSetter = [this](const vec2& val) { setOffset(val); };

    if (fov) {
        fov->setGetAndSet(fovGetter, fovSetter);
    } else {
        fov = new FloatRefProperty("fov", "FOV", fovGetter, fovSetter,
                                   {0.0f, ConstraintBehavior::Immutable},
                                   {180.0f, ConstraintBehavior::Immutable}, 0.1f);
        comp->addCamerapProperty(fov);
    }

    fov->setVisible(true);

    if (offset) {
        offset->setGetAndSet(offGetter, offSetter);
    } else {
        offset = new FloatVec2RefProperty("separation", "Separation", offGetter, offSetter,
                                          {vec2(-10.0f), ConstraintBehavior::Ignore},
                                          {vec2(10.0f), ConstraintBehavior::Ignore}, vec2(0.01f));
        comp->addCamerapProperty(offset);
    }

    offset->setVisible(true);
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
    s.serialize("fovy", fovy_);
    s.serialize("offset", offset_);
}
void SkewedPerspectiveCamera::deserialize(Deserializer& d) {
    d.deserialize("fovy", fovy_);
    d.deserialize("offset", offset_);
    Camera::deserialize(d);
}

}  // namespace inviwo
