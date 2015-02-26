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

CameraBase::CameraBase(vec3 eye /*= vec3(0.0f, 0.0f, -2.0f)*/, vec3 center /*= vec3(0.0f)*/, vec3 lookUp /*= vec3(0.0f, 1.0f, 0.0f)*/)
    : lookFrom_(eye)
    , lookTo_(center)
    , lookUp_(lookUp)
    , farPlane_(10000.0f)
    , nearPlane_(0.01f) 
    , invalidViewMatrix_(true)
    , invalidProjectionMatrix_(true) {

}

const mat4& CameraBase::viewMatrix() const {
    if (invalidViewMatrix_) {
        const_cast<CameraBase*>(this)->updateViewMatrix();
        const_cast<CameraBase*>(this)->invalidViewMatrix_ = false;
    }
    return viewMatrix_;
    //mat4 view = glm::mat4_cast(orientation_);
    //return glm::translate(view, -lookFrom_);
}

const mat4& CameraBase::projectionMatrix() const {
    if (invalidProjectionMatrix_) {
        const_cast<CameraBase*>(this)->updateProjectionMatrix();
        const_cast<CameraBase*>(this)->invalidProjectionMatrix_ = false;
    }
    return projectionMatrix_;
}

void CameraBase::updateViewMatrix() {
    viewMatrix_ = glm::lookAt(lookFrom_, lookTo_, lookUp_);
}

} // namespace

