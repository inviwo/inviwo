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

#ifndef IVW_CAMERA_H
#define IVW_CAMERA_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

/**
 * \class Camera
 *
 * \brief Base class for cameras.
 * Override this class to set your own projection matrix.
 * @see PerspectiveCamera
 * @see OrthographicCamera
 */
class IVW_CORE_API Camera : public IvwSerializable {
public:
    /**
     * \brief Default parameters creates a right handed coordinate system
     * with camera looking towards the negative z-axis.
     * with X = (1, 0, 0), Y = (0, 1, 0), Z = (0, 0, -1)
     *
     * @param vec3 lookFrom Camera position (eye)
     * @param vec3 lookTo Camera focus point (center)
     * @param vec3 lookUp Camera up direction
     */
    Camera(vec3 lookFrom = vec3(0.0f, 0.0f, 2.0f), vec3 lookTo = vec3(0.0f),
               vec3 lookUp = vec3(0.0f, 1.0f, 0.0f), float nearPlane = 0.01f,
               float farPlane = 10000.0f);
    virtual ~Camera() = default;
    Camera(const Camera& other) = default;
    //Camera(Camera&& other) = default;
    Camera& operator=(const Camera& other) = default;

    vec3& getLookFrom() { return lookFrom_; }
    const vec3& getLookFrom() const;
    void setLookFrom(vec3 val);

    vec3& getLookTo() { return lookTo_; }
    const vec3& getLookTo() const;
    void setLookTo(vec3 val);

    vec3& getLookUp() { return lookUp_; }
    const vec3& getLookUp() const;
    void setLookUp(vec3 val);


    /** 
     * \brief Get unnormalized direction of camera: lookTo - lookFrom
     * 
     * @return Unnormalized direction of camera
     */
    vec3 getDirection() const { return lookTo_ - lookFrom_; }

    float getNearPlaneDist() const;
    /** 
     * \brief Set distance to the near plane from lookFrom.
     * 
     * @param val Distance
     */
    void setNearPlaneDist(float val);

    float getFarPlaneDist() const;
    /** 
     * \brief Set distance to the far plane from lookFrom.
     * 
     * @param val Distance
     */
    void setFarPlaneDist(float val);

    const mat4& viewMatrix() const;
    const mat4& projectionMatrix() const;
    const mat4& inverseViewMatrix() const;
    const mat4& inverseProjectionMatrix() const;

    /**
    * \brief Convert from normalized device coordinates (xyz in [-1 1]) to world coordinates.
    *
    * @param vec3 ndcCoords Coordinates in [-1 1]
    * @return vec3 World space position
    */
    vec3 getWorldPosFromNormalizedDeviceCoords(const vec3& ndcCoords) const;

    /**
    * \brief Convert from normalized device coordinates (xyz in [-1 1]) to clip coordinates,
    * where z value of -1 correspond to the near plane and 1 to the far plane. 
    * Coordinates outside of the [-1 1]^3 range will be clipped.
    *
    * @param vec4 ndcCoords xyz clip-coordinates in [-1 1]^3, and the clip w-coordinate used for perspective division.
    * @return vec4 Clip space position
    */
    vec4 getClipPosFromNormalizedDeviceCoords(const vec3& ndcCoords) const;

    virtual void serialize(IvwSerializer& s) const override;
    virtual void deserialize(IvwDeserializer& d) override;

protected:

    bool equalTo(const Camera& other) const;
    /**
     * \brief Calculate and return the projection matrix for the camera.
     *
     * Implement this function to provide your own projection computation functionality.
     * For example orthogonal or perspective projection.
     * This function will be called when the projection matrix is invalid.
     * @see PerspectiveCamera
     * @see OrthographicCamera
     */
    virtual mat4 calculateProjectionMatrix() const = 0;
    void invalidateViewMatrix();
    void invalidateProjectionMatrix();

    vec3 lookFrom_;
    vec3 lookTo_;
    vec3 lookUp_;

    float nearPlaneDist_; ///< Distance to the near plane from lookFrom.
    float farPlaneDist_;  ///< Distance to the far plane from lookFrom.

    // Make mutable to allow then to be changed even though they are called from const function.
    // This allows us to perform lazy evaluation.
    mutable bool invalidViewMatrix_;
    mutable bool invalidProjectionMatrix_;
    mutable mat4 viewMatrix_;
    mutable mat4 projectionMatrix_;
    mutable mat4 inverseViewMatrix_;
    mutable mat4 inverseProjectionMatrix_;
};

class IVW_CORE_API PerspectiveCamera : public Camera {
public:
    PerspectiveCamera(vec3 lookFrom = vec3(0.0f, 0.0f, 2.0f), vec3 lookTo = vec3(0.0f),
                      vec3 lookUp = vec3(0.0f, 1.0f, 0.0f), float nearPlane = 0.01f,
                      float farPlane = 10000.0f, float fieldOfView = 60.f, float aspectRatio = 1.f);
    virtual ~PerspectiveCamera() = default;
    PerspectiveCamera(const PerspectiveCamera& other) = default;
    //Camera(Camera&& other) = default;
    PerspectiveCamera& operator=(const PerspectiveCamera& other) = default;

    friend bool operator==(const PerspectiveCamera& lhs, const PerspectiveCamera& rhs);
    friend bool operator!=(const PerspectiveCamera& lhs, const PerspectiveCamera& rhs);

    float getFovy() const;
    void setFovy(float val);
    float getAspectRatio() const;
    void setAspectRatio(float val);

    virtual void serialize(IvwSerializer& s) const override;
    virtual void deserialize(IvwDeserializer& d) override;

protected:
    virtual mat4 calculateProjectionMatrix() const override;

    float fovy_;
    float aspectRatio_;
};

bool operator==(const PerspectiveCamera& lhs, const PerspectiveCamera& rhs);
bool operator!=(const PerspectiveCamera& lhs, const PerspectiveCamera& rhs);

/**
 * \class OrthographicCamera
 *
 * \brief Camera with no perspective projection.
 * Objects far away will apear as large as objects close.
 * @see Camera
 * @see OrthographicCamera
 */
class IVW_CORE_API OrthographicCamera : public Camera {
public:
    OrthographicCamera(vec3 lookFrom = vec3(0.0f, 0.0f, 2.0f), vec3 lookTo = vec3(0.0f),
                       vec3 lookUp = vec3(0.0f, 1.0f, 0.0f), float nearPlane = 0.01f,
                       float farPlane = 10000.0f, vec4 frustum = vec4(-1, 1, -1, 1));
    virtual ~OrthographicCamera() = default;
    OrthographicCamera(const OrthographicCamera& other) = default;
    //Camera(Camera&& other) = default;
    OrthographicCamera& operator=(const OrthographicCamera& other) = default;

    friend bool operator==(const OrthographicCamera& lhs, const OrthographicCamera& rhs);
    friend bool operator!=(const OrthographicCamera& lhs, const OrthographicCamera& rhs);

    const vec4& getFrustum() const;
    /**
     * \brief Left, right, bottom, top view volume
     *
     * Set view frustum used for projection matrix calculation.
     *
     * @param inviwo::vec4 val
     * @return void
     */
    void setFrustum(vec4 val);

    virtual void serialize(IvwSerializer& s) const override;
    virtual void deserialize(IvwDeserializer& d) override;

protected:
    virtual mat4 calculateProjectionMatrix() const override;

    // Left, right, bottom, top view volume
    vec4 frustum_;
};

bool operator==(const OrthographicCamera& lhs, const OrthographicCamera& rhs);
bool operator!=(const OrthographicCamera& lhs, const OrthographicCamera& rhs);

// Implementation details

inline const vec3& Camera::getLookFrom() const { return lookFrom_; }
inline void Camera::setLookFrom(vec3 val) {
    lookFrom_ = val;
    invalidateViewMatrix();
}
inline const vec3& Camera::getLookTo() const { return lookTo_; }
inline void Camera::setLookTo(vec3 val) {
    lookTo_ = val;
    invalidateViewMatrix();
}
inline const vec3& Camera::getLookUp() const { return lookUp_; }
inline void Camera::setLookUp(vec3 val) {
    lookUp_ = val;
    invalidateViewMatrix();
}

inline float Camera::getNearPlaneDist() const { return nearPlaneDist_; }
inline void Camera::setNearPlaneDist(float val) {
    nearPlaneDist_ = val;
    invalidateProjectionMatrix();
}
inline float Camera::getFarPlaneDist() const { return farPlaneDist_; }
inline void Camera::setFarPlaneDist(float val) {
    farPlaneDist_ = val;
    invalidateProjectionMatrix();
}

inline void Camera::invalidateViewMatrix() { invalidViewMatrix_ = true; }
inline void Camera::invalidateProjectionMatrix() { invalidProjectionMatrix_ = true; }

inline float PerspectiveCamera::getFovy() const { return fovy_; }
inline void PerspectiveCamera::setFovy(float val) {
    fovy_ = val;
    invalidateProjectionMatrix();
}
inline float PerspectiveCamera::getAspectRatio() const { return aspectRatio_; }
inline void PerspectiveCamera::setAspectRatio(float val) {
    aspectRatio_ = val;
    invalidateProjectionMatrix();
}

inline mat4 PerspectiveCamera::calculateProjectionMatrix() const {
    return glm::perspective(glm::radians(fovy_), aspectRatio_, nearPlaneDist_, farPlaneDist_);
};

inline const vec4& OrthographicCamera::getFrustum() const { return frustum_; }

inline void OrthographicCamera::setFrustum(inviwo::vec4 val) {
    frustum_ = val;
    invalidateProjectionMatrix();
}

inline mat4 OrthographicCamera::calculateProjectionMatrix() const {
    return glm::ortho(frustum_.x, frustum_.y, frustum_.z, frustum_.w, nearPlaneDist_, farPlaneDist_);
};

}  // namespace

#endif  // IVW_CAMERA_H
