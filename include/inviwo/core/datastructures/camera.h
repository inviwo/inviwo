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
 * \class CameraBase
 *
 * \brief Base class for cameras.
 * Override this class to set your own projection matrix.
 * @see PerspectiveCamera
 * @see OrthographicCamera
 */
class IVW_CORE_API CameraBase : public IvwSerializable {
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
    CameraBase(vec3 lookFrom = vec3(0.0f, 0.0f, 2.0f), vec3 lookTo = vec3(0.0f),
               vec3 lookUp = vec3(0.0f, 1.0f, 0.0f), float nearPlane = 0.01f,
               float farPlane = 10000.0f);
    virtual ~CameraBase() {}

    vec3 getLookFrom() const;
    void setLookFrom(vec3 val);

    vec3 getLookTo() const;
    void setLookTo(vec3 val);

    vec3 getLookUp() const;
    void setLookUp(vec3 val);

    float getNearPlane() const;
    void setNearPlane(float val);

    float getFarPlane() const;
    void setFarPlane(float val);

    const mat4& viewMatrix() const;
    const mat4& projectionMatrix() const;
    mat4 inverseViewMatrix() const;
    mat4 inverseProjectionMatrix() const;

    virtual void serialize(IvwSerializer& s) const override;
    virtual void deserialize(IvwDeserializer& d) override;

protected:
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

    float nearPlane_;
    float farPlane_;

    // Make mutable to allow then to be changed even though they are called from const function.
    // This allows us to perform lazy evaluation.
    mutable bool invalidViewMatrix_;
    mutable bool invalidProjectionMatrix_;
    mutable mat4 viewMatrix_;
    mutable mat4 projectionMatrix_;
    mutable mat4 inverseViewMatrix_;
    mutable mat4 inverseProjectionMatrix_;
};

class IVW_CORE_API PerspectiveCamera : public CameraBase {
public:
    PerspectiveCamera(vec3 lookFrom = vec3(0.0f, 0.0f, 2.0f), vec3 lookTo = vec3(0.0f),
                      vec3 lookUp = vec3(0.0f, 1.0f, 0.0f), float nearPlane = 0.01f,
                      float farPlane = 10000.0f, float fieldOfView = 60.f, float aspectRatio = 1.f);
    virtual ~PerspectiveCamera() {}

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

/**
 * \class OrthographicCamera
 *
 * \brief Camera with no perspective projection.
 * Objects far away will apear as large as objects close.
 * @see CameraBase
 * @see OrthographicCamera
 */
class IVW_CORE_API OrthographicCamera : public CameraBase {
public:
    OrthographicCamera(vec3 lookFrom = vec3(0.0f, 0.0f, 2.0f), vec3 lookTo = vec3(0.0f),
                       vec3 lookUp = vec3(0.0f, 1.0f, 0.0f), float nearPlane = 0.01f,
                       float farPlane = 10000.0f, vec4 frustum = vec4(-1, 1, -1, 1));
    virtual ~OrthographicCamera() {}

    vec4 getFrustum() const;
    /**
     * \brief Left, right, bottom, top view volume
     *
     * Set view frustum used for projection matrix calculation.
     *
     * @param inviwo::vec4 val
     * @return void
     */
    void setFrustum(inviwo::vec4 val);

    virtual void serialize(IvwSerializer& s) const override;
    virtual void deserialize(IvwDeserializer& d) override;

protected:
    virtual mat4 calculateProjectionMatrix() const override;

    // Left, right, bottom, top view volume
    vec4 frustum_;
};

// Implementation details

inline vec3 CameraBase::getLookFrom() const { return lookFrom_; }
inline void CameraBase::setLookFrom(vec3 val) {
    lookFrom_ = val;
    invalidateViewMatrix();
}
inline vec3 CameraBase::getLookTo() const { return lookTo_; }
inline void CameraBase::setLookTo(vec3 val) {
    lookTo_ = val;
    invalidateViewMatrix();
}
inline vec3 CameraBase::getLookUp() const { return lookUp_; }
inline void CameraBase::setLookUp(vec3 val) {
    lookUp_ = val;
    invalidateViewMatrix();
}

inline float CameraBase::getNearPlane() const { return nearPlane_; }
inline void CameraBase::setNearPlane(float val) {
    nearPlane_ = val;
    invalidateProjectionMatrix();
}
inline float CameraBase::getFarPlane() const { return farPlane_; }
inline void CameraBase::setFarPlane(float val) {
    farPlane_ = val;
    invalidateProjectionMatrix();
}

inline mat4 CameraBase::inverseViewMatrix() const {
    if (invalidViewMatrix_) viewMatrix();
    return inverseViewMatrix_;
}

inline mat4 CameraBase::inverseProjectionMatrix() const {
    if (invalidProjectionMatrix_) projectionMatrix();
    return inverseProjectionMatrix_;
}

inline void CameraBase::invalidateViewMatrix() { invalidViewMatrix_ = true; }
inline void CameraBase::invalidateProjectionMatrix() { invalidProjectionMatrix_ = true; }

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
    return glm::perspective(fovy_, aspectRatio_, nearPlane_, farPlane_);
};

inline vec4 OrthographicCamera::getFrustum() const { return frustum_; }

inline void OrthographicCamera::setFrustum(inviwo::vec4 val) {
    frustum_ = val;
    invalidateProjectionMatrix();
}

inline mat4 OrthographicCamera::calculateProjectionMatrix() const {
    return glm::ortho(frustum_.x, frustum_.y, frustum_.z, frustum_.w, nearPlane_, farPlane_);
};

}  // namespace

#endif  // IVW_CAMERA_H
