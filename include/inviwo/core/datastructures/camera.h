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
class IVW_CORE_API CameraBase { 
public:
    /** 
     * \brief Default parameters creates a right handed coordinate system
     * with camera looking towards the negative z-axis.
     * with X = (1, 0, 0), Y = (0, 1, 0), Z = (0, 0, -1)
     * 
     * @param vec3 eye Camera position ( look from )
     * @param vec3 center Camera focus point (look to)
     * @param vec3 lookUp Camera up direction
     */
    CameraBase(vec3 eye = vec3(0.0f, 0.0f, 2.0f),
              vec3 center = vec3(0.0f),
              vec3 lookUp = vec3(0.0f, 1.0f, 0.0f));
    virtual ~CameraBase(){}

    inviwo::vec3 getLookFrom() const { return lookFrom_; }
    void setLookFrom(inviwo::vec3 val) { lookFrom_ = val; invalidateViewMatrix(); }
    inviwo::vec3 getLookTo() const { return lookTo_; }
    void setLookTo(inviwo::vec3 val) { lookTo_ = val; invalidateViewMatrix(); }
    inviwo::vec3 getLookUp() const { return lookUp_; }
    void setLookUp(inviwo::vec3 val) { lookUp_ = val; invalidateViewMatrix(); }


    float getNearPlaneDist() const { return nearPlane_; }
    void setNearPlane(float val) { nearPlane_ = val; invalidateProjectionMatrix();}
    float getFarPlaneDist() const { return farPlane_; }
    void setFarPlane(float val) { farPlane_ = val; invalidateProjectionMatrix(); }

    const mat4& viewMatrix() const;
    const mat4& projectionMatrix() const;
    mat4 inverseViewMatrix() const { return glm::inverse(viewMatrix()); }
    mat4 inverseProjectionMatrix() const { return glm::inverse(projectionMatrix()); }
protected:
    void updateViewMatrix();
    virtual void updateProjectionMatrix() = 0;
    void invalidateViewMatrix() { invalidViewMatrix_ = true; }
    void invalidateProjectionMatrix() { invalidProjectionMatrix_ = true; }

    mat4 viewMatrix_;
    mat4 projectionMatrix_;

    vec3 lookFrom_;
    vec3 lookTo_;
    vec3 lookUp_;

    float nearPlane_;
    float farPlane_;

    bool invalidViewMatrix_;
    bool invalidProjectionMatrix_;
};

class IVW_CORE_API PerspectiveCamera: public CameraBase { 
public:
    PerspectiveCamera(vec3 eye = vec3(0.0f, 0.0f, 2.0f)
        , vec3 center = vec3(0.0f)
        , vec3 lookUp = vec3(0.0f, 1.0f, 0.0f)
        , float fieldOfView = 60.f, float aspectRatio = 1.f): CameraBase(eye, center, lookUp), fovy_(fieldOfView), aspectRatio_(aspectRatio) {};
    virtual ~PerspectiveCamera(){}

    float getFovy() const { return fovy_; }
    void setFovy(float val) { fovy_ = val; invalidateProjectionMatrix(); }
    float getAspectRatio() const { return aspectRatio_; }
    void setAspectRatio(float val) { aspectRatio_ = val; invalidateProjectionMatrix(); }
protected:

    virtual void updateProjectionMatrix() {
        projectionMatrix_ = glm::perspective(fovy_, aspectRatio_, nearPlane_, farPlane_);
    };
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
class IVW_CORE_API OrthographicCamera: public CameraBase { 
public:
    OrthographicCamera(vec3 eye = vec3(0.0f, 0.0f, 2.0f)
        , vec3 center = vec3(0.0f)
        , vec3 lookUp = vec3(0.0f, 1.0f, 0.0f)
        , vec4 frustum = vec4(-1, 1, -1, 1)): CameraBase(eye, center, lookUp), frustum_(frustum) {};
    virtual ~OrthographicCamera(){}

    inviwo::vec4 getFrustum() const { return frustum_; }
    /** 
     * \brief Left, right, bottom, top view volume
     *
     * Set view frustum used for projection matrix calculation.
     * 
     * @param inviwo::vec4 val 
     * @return void 
     */
    void setFrustum(inviwo::vec4 val) { frustum_ = val; invalidateProjectionMatrix(); }
protected:

    virtual void updateProjectionMatrix() {
        projectionMatrix_ = glm::ortho(frustum_.x, frustum_.y, frustum_.z, frustum_.w, nearPlane_, farPlane_);
    };
    // Left, right, bottom, top view volume
    vec4 frustum_;
};

} // namespace

#endif // IVW_CAMERA_H

