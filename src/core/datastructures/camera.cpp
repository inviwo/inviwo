/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

namespace inviwo {

Camera::Camera(vec3 lookFrom, vec3 lookTo, vec3 lookUp, float nearPlane, float farPlane)
    : lookFrom_(lookFrom)
    , lookTo_(lookTo)
    , lookUp_(lookUp)
    , nearPlaneDist_(nearPlane)
    , farPlaneDist_(farPlane)
    , invalidViewMatrix_(true)
    , invalidProjectionMatrix_(true) {}

const mat4& Camera::viewMatrix() const {
    if (invalidViewMatrix_) {
        viewMatrix_ = glm::lookAt(lookFrom_, lookTo_, lookUp_);
        inverseViewMatrix_ = glm::inverse(viewMatrix_);
        invalidViewMatrix_ = false;
    }
    return viewMatrix_;
}

const mat4& Camera::projectionMatrix() const {
    if (invalidProjectionMatrix_) {
        projectionMatrix_ = calculateProjectionMatrix();
        inverseProjectionMatrix_ = glm::inverse(projectionMatrix_);
        invalidProjectionMatrix_ = false;
    }
    return projectionMatrix_;
}

const mat4& Camera::inverseViewMatrix() const {
    if (invalidViewMatrix_) viewMatrix();
    return inverseViewMatrix_;
}

const mat4& Camera::inverseProjectionMatrix() const {
    if (invalidProjectionMatrix_) projectionMatrix();
    return inverseProjectionMatrix_;
}

vec3 Camera::getWorldPosFromNormalizedDeviceCoords(const vec3& ndcCoords) const {
    vec4 clipCoords = getClipPosFromNormalizedDeviceCoords(ndcCoords);
    vec4 eyeCoords = inverseProjectionMatrix() * clipCoords;
    vec4 worldCoords = inverseViewMatrix() * eyeCoords;
    worldCoords /= worldCoords.w;
    return worldCoords.xyz();
}

vec4 Camera::getClipPosFromNormalizedDeviceCoords(const vec3& ndcCoords) const {
    float clipW = projectionMatrix_[2][3] /
                  (ndcCoords.z - (projectionMatrix_[2][2] / projectionMatrix_[3][2]));
    return vec4(ndcCoords * clipW, clipW);
}

vec3 Camera::getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(
    const vec2& normalizedScreenCoord) const {
    // Default to using focus point for depth
    vec4 lookToClipCoord = projectionMatrix() * viewMatrix() * vec4(getLookTo(), 1.f);
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
    , aspectRatio_(aspectRatio){};

PerspectiveCamera* PerspectiveCamera::clone() const { return new PerspectiveCamera(*this); }

bool PerspectiveCamera::update(const Camera* source) {
    if (auto perspectiveCamera = dynamic_cast<const PerspectiveCamera*>(source)) {
        *this = *perspectiveCamera;
        return true;
    } else {
        return false;
    }
}

void PerspectiveCamera::configureProperties(CompositeProperty* comp) {
    auto fov = dynamic_cast<FloatProperty*>(comp->getPropertyByIdentifier("fov"));
    if (fov) {
        setFovy(fov->get());
    } else {
        fov = new FloatProperty("fov", "FOV", 60.0f, 30.0f, 360.0f, 0.1f);
        comp->addProperty(fov, true);
        fov->setSerializationMode(PropertySerializationMode::ALL);
    }

    fov->onChange([this, fov]() { setFovy(fov->get()); });
}

bool operator==(const PerspectiveCamera& lhs, const PerspectiveCamera& rhs) {
    return !(lhs.equalTo(rhs) | (lhs.fovy_ != rhs.fovy_) | (lhs.aspectRatio_ != rhs.aspectRatio_));
}

bool operator!=(const PerspectiveCamera& lhs, const PerspectiveCamera& rhs) {
    return (lhs.equalTo(rhs) | (lhs.fovy_ != rhs.fovy_) | (lhs.aspectRatio_ != rhs.aspectRatio_));
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
};

OrthographicCamera::OrthographicCamera(vec3 lookFrom, vec3 lookTo, vec3 lookUp, float nearPlane,
                                       float farPlane, vec4 frustum)
    : Camera(lookFrom, lookTo, lookUp, nearPlane, farPlane), frustum_(frustum){};

OrthographicCamera* OrthographicCamera::clone() const { return new OrthographicCamera(*this); }

bool OrthographicCamera::update(const Camera* source) {
    if (auto orthographicCamera = dynamic_cast<const OrthographicCamera*>(source)) {
        *this = *orthographicCamera;
        return true;
    } else {
        return false;
    }
}


void OrthographicCamera::configureProperties(CompositeProperty* comp) {
    auto widthProp = dynamic_cast<FloatProperty*>(comp->getPropertyByIdentifier("width"));
    if (widthProp) {
        const float oldWidth{frustum_.y - frustum_.x};
        const float oldHeight{frustum_.w - frustum_.z};
        auto aspect = oldWidth / oldHeight;
        const float width = widthProp->get();
        setFrustum({-width / 2.0f, width / 2.0f, -width / 2.0f / aspect, +width / 2.0f / aspect});
    } else {
        widthProp = new FloatProperty("width", "Width", 10, 0.01f, 1000.0f, 0.1f);
        comp->addProperty(widthProp, true);
        widthProp->setSerializationMode(PropertySerializationMode::ALL);
    }

    widthProp->onChange([this, widthProp]() {
        // Left, right, bottom, top view volume
        const float oldWidth{frustum_.y - frustum_.x};
        const float oldHeight{frustum_.w - frustum_.z};
        auto aspect = oldWidth / oldHeight;
        const float width = widthProp->get();
        setFrustum({-width / 2.0f, width / 2.0f, -width / 2.0f / aspect, +width / 2.0f / aspect});
    });
}

bool operator==(const OrthographicCamera& lhs, const OrthographicCamera& rhs) {
    return !(lhs.equalTo(rhs) | glm::any(glm::notEqual(lhs.frustum_, rhs.frustum_)));
}

bool operator!=(const OrthographicCamera& lhs, const OrthographicCamera& rhs) {
    return (lhs.equalTo(rhs) | glm::any(glm::notEqual(lhs.frustum_, rhs.frustum_)));
}

float OrthographicCamera::getAspectRatio() const {
    // Left, right, bottom, top view volume
    const float width{frustum_.y - frustum_.x};
    const float height{frustum_.w - frustum_.z};
    return width / height;
}

void OrthographicCamera::setAspectRatio(float val) {
    // Left, right, bottom, top view volume
    const float width{frustum_.y - frustum_.x};
    const float height{width / val};
    frustum_.z = -height / 2.0f;
    frustum_.w = +height / 2.0f;
    invalidateProjectionMatrix();
}

void OrthographicCamera::serialize(Serializer& s) const {
    Camera::serialize(s);
    s.serialize("frustum", frustum_);
}
void OrthographicCamera::deserialize(Deserializer& d) {
    d.deserialize("frustum", frustum_);
    Camera::deserialize(d);
}


SkewedPerspectiveCamera::SkewedPerspectiveCamera(vec3 lookFrom, vec3 lookTo, vec3 lookUp, float nearPlane,
	float farPlane, vec4 frustum, vec2 frustumOffset)
	: Camera(lookFrom, lookTo, lookUp, nearPlane, farPlane), frustum_(frustum), frustumSkewOffset_(frustumOffset){};

SkewedPerspectiveCamera* SkewedPerspectiveCamera::clone() const { return new SkewedPerspectiveCamera(*this); }

bool SkewedPerspectiveCamera::update(const Camera* source) {
	if (auto skewedPerspectiveCamera = dynamic_cast<const SkewedPerspectiveCamera*>(source)) {
		*this = *skewedPerspectiveCamera;
		return true;
	}
	else {
		return false;
	}
}


void SkewedPerspectiveCamera::configureProperties(CompositeProperty* comp) {
	auto widthProp = dynamic_cast<FloatProperty*>(comp->getPropertyByIdentifier("skewed-frustum-width"));
	if (widthProp) {
		const float oldWidth{ frustum_.y - frustum_.x };
		const float oldHeight{ frustum_.w - frustum_.z };
		auto aspect = oldWidth / oldHeight;
		const float width = widthProp->get();
		setFrustum({ -width / 2.0f, width / 2.0f, -width / 2.0f / aspect, +width / 2.0f / aspect });
	}
	else {
		widthProp = new FloatProperty("skewed-frustum-width", "Skewed Frustum Width", 0.1f, 0.001f, 10.0f, 0.0001f);
		comp->addProperty(widthProp, true);
        widthProp->setSerializationMode(PropertySerializationMode::ALL);
	}

	widthProp->onChange([this, widthProp]() {
		// Left, right, bottom, top view volume
		const float oldWidth{ frustum_.y - frustum_.x };
		const float oldHeight{ frustum_.w - frustum_.z };
		auto aspect = oldWidth / oldHeight;
		const float width = widthProp->get();
		setFrustum({ -width / 2.0f, width / 2.0f, -width / 2.0f / aspect, +width / 2.0f / aspect });
	});


	auto offsetProp = dynamic_cast<FloatVec2Property*>(comp->getPropertyByIdentifier("seperation"));
	if (offsetProp) {		
		const vec2 offset = offsetProp->get();
		setFrustumOffset(offset);
	}
	else {
		offsetProp = new FloatVec2Property("seperation", "Separation", vec2(0.0f), vec2(-10.0f), vec2(10.0f), vec2(0.01f));
		comp->addProperty(offsetProp, true);
        offsetProp->setSerializationMode(PropertySerializationMode::ALL);
	}

	offsetProp->onChange([this, offsetProp, widthProp]() {
		const vec2 offset = offsetProp->get();
		const float oldWidth{ frustum_.y - frustum_.x };
		const float oldHeight{ frustum_.w - frustum_.z };
		auto aspect = oldWidth / oldHeight;
		const float width = widthProp->get();
		setFrustumOffset(offset);
		setFrustum({ -width / 2.0f, width / 2.0f, -width / 2.0f / aspect, +width / 2.0f / aspect });
	});
}

bool operator==(const SkewedPerspectiveCamera& lhs, const SkewedPerspectiveCamera& rhs) {
    return !(lhs.equalTo(rhs) | glm::any(glm::notEqual(lhs.frustum_, rhs.frustum_)) |
             glm::any(glm::notEqual(lhs.frustumSkewOffset_, rhs.frustumSkewOffset_)));
}

bool operator!=(const SkewedPerspectiveCamera& lhs, const SkewedPerspectiveCamera& rhs) {
    return (lhs.equalTo(rhs) | glm::any(glm::notEqual(lhs.frustum_, rhs.frustum_)) |
            glm::any(glm::notEqual(lhs.frustumSkewOffset_, rhs.frustumSkewOffset_)));
}

float SkewedPerspectiveCamera::getAspectRatio() const {
	// Left, right, bottom, top view volume
	const float width{ frustum_.y - frustum_.x };
	const float height{ frustum_.w - frustum_.z };
	return width / height;
}

void SkewedPerspectiveCamera::setAspectRatio(float val) {
	// Left, right, bottom, top view volume
	const float width{ frustum_.y - frustum_.x };
	const float height{ width / val };
	frustum_.z = -height / 2.0f;
	frustum_.w = +height / 2.0f;
	invalidateProjectionMatrix();
}

mat4 SkewedPerspectiveCamera::calculateProjectionMatrix() const {
	vec4 f(frustum_);
	float left = f.x + frustumSkewOffset_.x;
	float right = (f.x + frustumSkewOffset_.x) + (f.y - f.x);
	float up = f.z + frustumSkewOffset_.y;
	float down = (f.z + frustumSkewOffset_.y) + (f.w - f.z);
	return glm::frustum(left, right, up, down, nearPlaneDist_, farPlaneDist_);
};


void SkewedPerspectiveCamera::serialize(Serializer& s) const {
	Camera::serialize(s);
	s.serialize("frustum", frustum_);
	s.serialize("frustumOffset", frustumSkewOffset_);
}
void SkewedPerspectiveCamera::deserialize(Deserializer& d) {
	d.deserialize("frustum", frustum_);
	d.deserialize("frustumOffset", frustumSkewOffset_);
	Camera::deserialize(d);
}

}  // namespace
