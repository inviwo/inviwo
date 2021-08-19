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

#include <inviwo/core/datastructures/camera/camera.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/ordinalrefproperty.h>
#include <inviwo/core/io/serialization/serialization.h>

#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/ordinalrefproperty.h>

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

void Camera::setLookFrom(vec3 val) {
    if (lookFrom_ != val) {
        lookFrom_ = val;
        invalidateViewMatrix();
        if (camprop_) camprop_->lookFrom_.propertyModified();
    }
}

void Camera::setLookTo(vec3 val) {
    if (lookTo_ != val) {
        lookTo_ = val;
        invalidateViewMatrix();
        if (camprop_) camprop_->lookTo_.propertyModified();
    }
}

void Camera::setLookUp(vec3 val) {
    if (lookUp_ != val) {
        lookUp_ = val;
        invalidateViewMatrix();
        if (camprop_) camprop_->lookUp_.propertyModified();
    }
}

void Camera::setNearPlaneDist(float val) {
    if (nearPlaneDist_ != val) {
        nearPlaneDist_ = val;
        invalidateProjectionMatrix();
        if (camprop_) camprop_->nearPlane_.propertyModified();
    }
}
void Camera::setFarPlaneDist(float val) {
    if (farPlaneDist_ != val) {
        farPlaneDist_ = val;
        invalidateProjectionMatrix();
        if (camprop_) camprop_->farPlane_.propertyModified();
    }
}

void Camera::setAspectRatio(float val) {
    if (aspectRatio_ != val) {
        aspectRatio_ = val;
        invalidateProjectionMatrix();
        if (camprop_) camprop_->aspectRatio_.propertyModified();
    }
}

void Camera::invalidateViewMatrix() { invalidViewMatrix_ = true; }
void Camera::invalidateProjectionMatrix() { invalidProjectionMatrix_ = true; }

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
    s.serialize("near", nearPlaneDist_);
    s.serialize("far", farPlaneDist_);
    s.serialize("aspectRatio", aspectRatio_);
}
void Camera::deserialize(Deserializer& d) {
    d.deserialize("lookFrom", lookFrom_);
    d.deserialize("lookTo", lookTo_);
    d.deserialize("lookUp", lookUp_);
    d.deserialize("near", nearPlaneDist_);
    d.deserialize("far", farPlaneDist_);
    d.deserialize("aspectRatio", aspectRatio_);
    invalidProjectionMatrix_ = true;
    invalidViewMatrix_ = true;
}

void Camera::updateFrom(const Camera& source) {
    setLookFrom(source.getLookFrom());
    setLookTo(source.getLookTo());
    setLookUp(source.getLookUp());

    setNearPlaneDist(source.getNearPlaneDist());
    setFarPlaneDist(source.getFarPlaneDist());
    setAspectRatio(source.getAspectRatio());
}

namespace {
template <auto Dst, auto Call, typename Obj>
auto set(Obj* obj) {
    return [obj](const auto& val) {
        auto& dst = std::invoke(Dst, obj);
        if (dst != val) {
            dst = val;
            std::invoke(Call, obj);
        }
    };
};
}  // namespace

void Camera::configureProperties(CameraProperty& cp, bool attach) {
    if (attach) {
        camprop_ = &cp;
        cp.lookFrom_.setGetAndSet([this]() { return getLookFrom(); },
                                  set<&Camera::lookFrom_, &Camera::invalidateViewMatrix>(this));
        cp.lookTo_.setGetAndSet([this]() { return getLookTo(); },
                                set<&Camera::lookTo_, &Camera::invalidateViewMatrix>(this));
        cp.lookUp_.setGetAndSet([this]() { return getLookUp(); },
                                set<&Camera::lookUp_, &Camera::invalidateViewMatrix>(this));
        cp.aspectRatio_.setGetAndSet(
            [this]() { return getAspectRatio(); },
            set<&Camera::aspectRatio_, &Camera::invalidateProjectionMatrix>(this));
        cp.nearPlane_.setGetAndSet(
            [this]() { return getNearPlaneDist(); },
            set<&Camera::nearPlaneDist_, &Camera::invalidateProjectionMatrix>(this));
        cp.farPlane_.setGetAndSet(
            [this]() { return getFarPlaneDist(); },
            set<&Camera::farPlaneDist_, &Camera::invalidateProjectionMatrix>(this));
    } else {
        camprop_ = nullptr;
        cp.lookFrom_.setGetAndSet([val = cp.lookFrom_.get()]() { return val; }, [](const vec3&) {});
        cp.lookTo_.setGetAndSet([val = cp.lookTo_.get()]() { return val; }, [](const vec3&) {});
        cp.lookUp_.setGetAndSet([val = cp.lookUp_.get()]() { return val; }, [](const vec3&) {});
        cp.aspectRatio_.setGetAndSet([val = cp.aspectRatio_.get()]() { return val; },
                                     [](const float&) {});
        cp.nearPlane_.setGetAndSet([val = cp.nearPlane_.get()]() { return val; },
                                   [](const float&) {});
        cp.farPlane_.setGetAndSet([val = cp.farPlane_.get()]() { return val; },
                                  [](const float&) {});
    }
}

bool Camera::equalTo(const Camera& other) const {
    return !(glm::any(glm::notEqual(lookFrom_, other.lookFrom_)) ||
             glm::any(glm::notEqual(lookTo_, other.lookTo_)) ||
             (nearPlaneDist_ != other.nearPlaneDist_) || (farPlaneDist_ != other.farPlaneDist_) ||
             (aspectRatio_ != other.aspectRatio_));
}

}  // namespace inviwo
