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
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/io/serialization/serialization.h>

namespace inviwo {

Camera::Camera(vec3 lookFrom, vec3 lookTo, vec3 lookUp, float nearPlane, float farPlane)
    : lookFrom_(lookFrom)
    , lookTo_(lookTo)
    , lookUp_(lookUp)
    , nearPlaneDist_(nearPlane)
    , farPlaneDist_(farPlane)
    , invalidViewMatrix_(true)
    , invalidProjectionMatrix_(true) {}

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
    s.serialize("lookFrom", lookFrom_);
    s.serialize("lookTo", lookTo_);
    s.serialize("lookUp", lookUp_);
    s.serialize("nearPlaneDist", nearPlaneDist_);
    s.serialize("farPlaneDist", farPlaneDist_);
}
void Camera::deserialize(Deserializer& d) {
    d.deserialize("lookFrom", lookFrom_);
    d.deserialize("lookTo", lookTo_);
    d.deserialize("lookUp", lookUp_);
    d.deserialize("nearPlaneDist", nearPlaneDist_);
    d.deserialize("farPlaneDist", farPlaneDist_);
    invalidProjectionMatrix_ = true;
    invalidViewMatrix_ = true;
}

bool Camera::equalTo(const Camera& other) const {
    return !(glm::any(glm::notEqual(lookFrom_, other.lookFrom_)) |
             glm::any(glm::notEqual(lookTo_, other.lookTo_)) |
             (nearPlaneDist_ != other.nearPlaneDist_) | (farPlaneDist_ != other.farPlaneDist_));
}

PerspectiveCamera::PerspectiveCamera(vec3 lookFrom, vec3 lookTo, vec3 lookUp, float nearPlane,
                                     float farPlane, float fieldOfView, float aspectRatio)
    : Camera(lookFrom, lookTo, lookUp, nearPlane, farPlane)
    , fovy_(fieldOfView)
    , aspectRatio_(aspectRatio) {}

PerspectiveCamera::PerspectiveCamera(const PerspectiveCamera& other)
    : Camera(other), fovy_{other.fovy_}, aspectRatio_{other.aspectRatio_} {}

PerspectiveCamera& PerspectiveCamera::operator=(const PerspectiveCamera& other) {
    if (this != &other) {
        Camera::operator=(other);
        fovy_ = other.fovy_;
        aspectRatio_ = other.aspectRatio_;
    }

    return *this;
}

PerspectiveCamera* PerspectiveCamera::clone() const { return new PerspectiveCamera(*this); }

bool PerspectiveCamera::update(const Camera* source) {
    if (auto perspectiveCamera = dynamic_cast<const PerspectiveCamera*>(source)) {
        *this = *perspectiveCamera;

        return true;
    } else {
        return false;
    }
}

void PerspectiveCamera::configureProperties(CompositeProperty* comp, Config config) {
    auto fovProp = dynamic_cast<FloatProperty*>(comp->getPropertyByIdentifier("fov"));

    if (config == Config::Hide) {
        if (fovProp) fovProp->setVisible(false);
        return;
    }

    if (fovProp) {
        setFovy(fovProp->get());
    } else {
        float initialFov = 38.0f;
        if (auto widthProp = dynamic_cast<FloatProperty*>(comp->getPropertyByIdentifier("width"))) {
            initialFov = glm::degrees(2.0f * std::atan(widthProp->get() / 2.0f /
                                                       glm::distance(getLookTo(), getLookFrom())));
        }
        fovProp = new FloatProperty("fov", "FOV", initialFov, 10.0f, 180.0f, 0.1f);
        fovProp->setSerializationMode(PropertySerializationMode::All);
        fovProp->setCurrentStateAsDefault();
        comp->insertProperty(comp->size() - 1, fovProp, true);
    }
    fovProp->setVisible(true);
    fovCallbackHolder_ = fovProp->onChangeScoped([this, fovProp]() { setFovy(fovProp->get()); });
}

bool operator==(const PerspectiveCamera& lhs, const PerspectiveCamera& rhs) {
    return lhs.equalTo(rhs) && lhs.fovy_ == rhs.fovy_ && lhs.aspectRatio_ == rhs.aspectRatio_;
}

bool operator!=(const PerspectiveCamera& lhs, const PerspectiveCamera& rhs) {
    return !(lhs == rhs);
}

void PerspectiveCamera::serialize(Serializer& s) const {
    Camera::serialize(s);
    s.serialize("fovy", fovy_);
    s.serialize("aspectRatio", aspectRatio_);
}
void PerspectiveCamera::deserialize(Deserializer& d) {
    d.deserialize("fovy", fovy_);
    d.deserialize("aspectRatio", aspectRatio_);
    Camera::deserialize(d);
}

OrthographicCamera::OrthographicCamera(vec3 lookFrom, vec3 lookTo, vec3 lookUp, float nearPlane,
                                       float farPlane, float width, float aspectRatio)
    : Camera(lookFrom, lookTo, lookUp, nearPlane, farPlane)
    , aspectRatio_{aspectRatio}
    , width_{width} {}

OrthographicCamera::OrthographicCamera(const OrthographicCamera& other)
    : Camera(other), aspectRatio_{other.aspectRatio_}, width_{other.width_} {}

OrthographicCamera& OrthographicCamera::operator=(const OrthographicCamera& other) {
    if (this != &other) {
        Camera::operator=(other);
        aspectRatio_ = other.aspectRatio_;
        width_ = other.width_;
    }

    return *this;
}

OrthographicCamera* OrthographicCamera::clone() const { return new OrthographicCamera(*this); }

bool OrthographicCamera::update(const Camera* source) {
    if (auto orthographicCamera = dynamic_cast<const OrthographicCamera*>(source)) {
        *this = *orthographicCamera;
        return true;
    } else {
        return false;
    }
}

void OrthographicCamera::configureProperties(CompositeProperty* comp, Config config) {
    auto widthProp = dynamic_cast<FloatProperty*>(comp->getPropertyByIdentifier("width"));

    if (config == Config::Hide) {
        if (widthProp) widthProp->setVisible(false);
        return;
    }

    if (widthProp) {
        setWidth(widthProp->get());
    } else {
        float initialWidth = 10.0f;
        if (auto fovProp = dynamic_cast<FloatProperty*>(comp->getPropertyByIdentifier("fov"))) {
            initialWidth = glm::distance(getLookTo(), getLookFrom()) *
                           std::tan(0.5f * glm::radians(fovProp->get()));
        }
        widthProp = new FloatProperty("width", "Width", initialWidth, 0.01f, 1000.0f, 0.1f);
        comp->insertProperty(comp->size() - 1, widthProp, true);
        widthProp->setSerializationMode(PropertySerializationMode::All);
    }

    widthProp->setVisible(true);
    widthCallbackHolder_ =
        widthProp->onChangeScoped([this, widthProp]() { setWidth(widthProp->get()); });
}

bool operator==(const OrthographicCamera& lhs, const OrthographicCamera& rhs) {
    return lhs.equalTo(rhs) && lhs.width_ == rhs.width_;
}

bool operator!=(const OrthographicCamera& lhs, const OrthographicCamera& rhs) {
    return !(rhs == lhs);
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
    s.serialize("aspectRatio", aspectRatio_);
    s.serialize("width", width_);
}
void OrthographicCamera::deserialize(Deserializer& d) {
    d.deserialize("aspectRatio", aspectRatio_);
    d.deserialize("width", width_);
    Camera::deserialize(d);
}

SkewedPerspectiveCamera::SkewedPerspectiveCamera(vec3 lookFrom, vec3 lookTo, vec3 lookUp,
                                                 float nearPlane, float farPlane, float fieldOfView,
                                                 float aspectRatio, vec2 offset)
    : Camera(lookFrom, lookTo, lookUp, nearPlane, farPlane)
    , fovy_(fieldOfView)
    , aspectRatio_(aspectRatio)
    , offset_(offset) {}

SkewedPerspectiveCamera::SkewedPerspectiveCamera(const SkewedPerspectiveCamera& other)
    : Camera(other), fovy_{other.fovy_}, aspectRatio_{other.aspectRatio_}, offset_{other.offset_} {}

SkewedPerspectiveCamera& SkewedPerspectiveCamera::operator=(const SkewedPerspectiveCamera& other) {
    if (this != &other) {
        Camera::operator=(other);
        fovy_ = other.fovy_;
        aspectRatio_ = other.aspectRatio_;
        offset_ = other.offset_;
    }

    return *this;
}

SkewedPerspectiveCamera* SkewedPerspectiveCamera::clone() const {
    return new SkewedPerspectiveCamera(*this);
}

bool SkewedPerspectiveCamera::update(const Camera* source) {
    if (auto skewedPerspectiveCamera = dynamic_cast<const SkewedPerspectiveCamera*>(source)) {
        *this = *skewedPerspectiveCamera;
        return true;
    } else {
        return false;
    }
}

void SkewedPerspectiveCamera::configureProperties(CompositeProperty* comp, Config config) {
    auto fovProp = dynamic_cast<FloatProperty*>(comp->getPropertyByIdentifier("fov"));
    auto offsetProp = dynamic_cast<FloatVec2Property*>(comp->getPropertyByIdentifier("separation"));

    if (config == Config::Hide) {
        if (fovProp) fovProp->setVisible(false);
        if (offsetProp) offsetProp->setVisible(false);
        return;
    }

    if (fovProp) {
        setFovy(fovProp->get());
    } else {
        float initialFov = 38.0f;
        if (auto widthProp = dynamic_cast<FloatProperty*>(comp->getPropertyByIdentifier("width"))) {
            initialFov = glm::degrees(2.0f * std::atan(widthProp->get() / 2.0f /
                                                       glm::distance(getLookTo(), getLookFrom())));
        }
        fovProp = new FloatProperty("fov", "FOV", initialFov, 10.0f, 180.0f, 0.1f);
        fovProp->setSerializationMode(PropertySerializationMode::All);
        fovProp->setCurrentStateAsDefault();
        comp->insertProperty(comp->size() - 1, fovProp, true);
    }

    fovProp->setVisible(true);
    fovCallbackHolder_ = fovProp->onChangeScoped([this, fovProp]() { setFovy(fovProp->get()); });

    if (offsetProp) {
        setOffset(offsetProp->get());
    } else {
        offsetProp = new FloatVec2Property("separation", "Separation", vec2(0.0f), vec2(-10.0f),
                                           vec2(10.0f), vec2(0.01f));
        offsetProp->setSerializationMode(PropertySerializationMode::All);
        offsetProp->setCurrentStateAsDefault();
        comp->insertProperty(comp->size() - 1, offsetProp, true);
    }

    offsetProp->setVisible(true);
    offsetCallbackHolder_ =
        offsetProp->onChangeScoped([this, offsetProp]() { setOffset(offsetProp->get()); });
}

bool operator==(const SkewedPerspectiveCamera& lhs, const SkewedPerspectiveCamera& rhs) {
    return lhs.equalTo(rhs) && lhs.fovy_ == rhs.fovy_ && lhs.aspectRatio_ == rhs.aspectRatio_ &&
           glm::all(glm::equal(lhs.offset_, rhs.offset_));
}

bool operator!=(const SkewedPerspectiveCamera& lhs, const SkewedPerspectiveCamera& rhs) {
    return !(lhs == rhs);
}

mat4 SkewedPerspectiveCamera::calculateViewMatrix() const {
    const vec3 xoffset{offset_.x * glm::normalize(glm::cross(lookTo_ - lookFrom_, lookUp_))};
    const vec3 yoffset{offset_.y * lookUp_};
    return glm::lookAt(lookFrom_ + xoffset + yoffset, lookTo_ + xoffset + yoffset, lookUp_);
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
    s.serialize("aspectRatio", aspectRatio_);
    s.serialize("offset", offset_);
}
void SkewedPerspectiveCamera::deserialize(Deserializer& d) {
    d.deserialize("fovy", fovy_);
    d.deserialize("aspectRatio", aspectRatio_);
    d.deserialize("offset", offset_);
    Camera::deserialize(d);
}

}  // namespace inviwo
