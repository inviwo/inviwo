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

namespace inviwo {

CameraBase::CameraBase(vec3 lookFrom, vec3 lookTo, vec3 lookUp, float nearPlane, float farPlane)
    : lookFrom_(lookFrom)
    , lookTo_(lookTo)
    , lookUp_(lookUp)
    , nearPlane_(nearPlane)
    , farPlane_(farPlane)
    , invalidViewMatrix_(true)
    , invalidProjectionMatrix_(true) {}

const mat4& CameraBase::viewMatrix() const {
    if (invalidViewMatrix_) {
        viewMatrix_ = glm::lookAt(lookFrom_, lookTo_, lookUp_);
        inverseViewMatrix_ = glm::inverse(viewMatrix_);
        invalidViewMatrix_ = false;
    }
    return viewMatrix_;
}

const mat4& CameraBase::projectionMatrix() const {
    if (invalidProjectionMatrix_) {
        projectionMatrix_ = calculateProjectionMatrix();
        inverseProjectionMatrix_ = glm::inverse(projectionMatrix_);
        invalidProjectionMatrix_ = false;
    }
    return projectionMatrix_;
}

void CameraBase::serialize(IvwSerializer& s) const {
    s.serialize("lookFrom", lookFrom_);
    s.serialize("lookTo", lookTo_);
    s.serialize("lookUp", lookUp_);
    s.serialize("nearPlane", nearPlane_);
    s.serialize("farPlane", farPlane_);
}
void CameraBase::deserialize(IvwDeserializer& d) {
    d.deserialize("lookFrom", lookFrom_);
    d.deserialize("lookTo", lookTo_);
    d.deserialize("lookUp", lookUp_);
    d.deserialize("nearPlane", nearPlane_);
    d.deserialize("farPlane", farPlane_);
    invalidProjectionMatrix_ = true;
    invalidViewMatrix_ = true;
}

PerspectiveCamera::PerspectiveCamera(vec3 lookFrom, vec3 lookTo, vec3 lookUp, float nearPlane,
                                     float farPlane, float fieldOfView, float aspectRatio)
    : CameraBase(lookFrom, lookTo, lookUp, nearPlane, farPlane)
    , fovy_(fieldOfView)
    , aspectRatio_(aspectRatio){};

void PerspectiveCamera::serialize(IvwSerializer& s) const {
    CameraBase::serialize(s);
    s.serialize("fovy", fovy_);
    s.serialize("aspectRatio", aspectRatio_);
}
void PerspectiveCamera::deserialize(IvwDeserializer& d) {
    d.deserialize("fovy", fovy_);
    d.deserialize("aspectRatio", aspectRatio_);
    CameraBase::deserialize(d);
};

OrthographicCamera::OrthographicCamera(vec3 lookFrom, vec3 lookTo, vec3 lookUp, float nearPlane,
                                       float farPlane, vec4 frustum)
    : CameraBase(lookFrom, lookTo, lookUp, nearPlane, farPlane), frustum_(frustum){};

void OrthographicCamera::serialize(IvwSerializer& s) const {
    CameraBase::serialize(s);
    s.serialize("frustum", frustum_);
}
void OrthographicCamera::deserialize(IvwDeserializer& d) {
    d.deserialize("frustum", frustum_);
    CameraBase::deserialize(d);
}

}  // namespace
