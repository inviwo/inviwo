/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <inviwo/core/interaction/cameratrackball.h>
//#include <glm/gtx/decomposition.hpp>
// Dependencies
#include <glm/mat4x4.hpp>
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace glm
{
    using namespace detail;
    /// Make a linear combination of two vectors and return the result.
    // result = (a * ascl) + (b * bscl)
    template <typename T, precision P>
    tvec3<T, P> combine(
        tvec3<T, P> const & a,
        tvec3<T, P> const & b,
        T ascl, T bscl)
    {
        return (a * ascl) + (b * bscl);
    }

    template <typename T, precision P>
    void v3Scale(tvec3<T, P> & v, T desiredLength)
    {
        T len = glm::length(v);
        if (len != 0)
        {
            T l = desiredLength / len;
            v[0] *= l;
            v[1] *= l;
            v[2] *= l;
        }
    }

    /**
    * Matrix decompose
    * http://www.opensource.apple.com/source/WebCore/WebCore-514/platform/graphics/transforms/TransformationMatrix.cpp
    * Decomposes the mode matrix to translations,rotation scale components
    *
    */

    template <typename T, precision P>
    bool decompose(tmat4x4<T, P> const & ModelMatrix, tvec3<T, P> & Scale, tquat<T, P> & Orientation, tvec3<T, P> & Translation, tvec3<T, P> & Skew, tvec4<T, P> & Perspective)
    {
        tmat4x4<T, P> LocalMatrix(ModelMatrix);

        // Normalize the matrix.
        if (LocalMatrix[3][3] == static_cast<T>(0))
            return false;

        for (length_t i = 0; i < 4; ++i)
            for (length_t j = 0; j < 4; ++j)
                LocalMatrix[i][j] /= LocalMatrix[3][3];

        // perspectiveMatrix is used to solve for perspective, but it also provides
        // an easy way to test for singularity of the upper 3x3 component.
        tmat4x4<T, P> PerspectiveMatrix(LocalMatrix);

        for (length_t i = 0; i < 3; i++)
            PerspectiveMatrix[i][3] = 0;
        PerspectiveMatrix[3][3] = 1;

        /// TODO: Fixme!
        if (determinant(PerspectiveMatrix) == static_cast<T>(0))
            return false;

        // First, isolate perspective.  This is the messiest.
        if (LocalMatrix[0][3] != 0 || LocalMatrix[1][3] != 0 || LocalMatrix[2][3] != 0)
        {
            // rightHandSide is the right hand side of the equation.
            tvec4<T, P> RightHandSide;
            RightHandSide[0] = LocalMatrix[0][3];
            RightHandSide[1] = LocalMatrix[1][3];
            RightHandSide[2] = LocalMatrix[2][3];
            RightHandSide[3] = LocalMatrix[3][3];

            // Solve the equation by inverting PerspectiveMatrix and multiplying
            // rightHandSide by the inverse.  (This is the easiest way, not
            // necessarily the best.)
            tmat4x4<T, P> InversePerspectiveMatrix = glm::inverse(PerspectiveMatrix);//   inverse(PerspectiveMatrix, inversePerspectiveMatrix);
            tmat4x4<T, P> TransposedInversePerspectiveMatrix = glm::transpose(InversePerspectiveMatrix);//   transposeMatrix4(inversePerspectiveMatrix, transposedInversePerspectiveMatrix);

            Perspective = TransposedInversePerspectiveMatrix * RightHandSide;
            //  v4MulPointByMatrix(rightHandSide, transposedInversePerspectiveMatrix, perspectivePoint);

            // Clear the perspective partition
            LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = 0;
            LocalMatrix[3][3] = 1;
        } else
        {
            // No perspective.
            Perspective = tvec4<T, P>(0, 0, 0, 1);
        }

        // Next take care of translation (easy).
        Translation = tvec3<T, P>(LocalMatrix[3]);
        LocalMatrix[3] = tvec4<T, P>(0, 0, 0, LocalMatrix[3].w);

        tvec3<T, P> Row[3], Pdum3;

        // Now get scale and shear.
        for (length_t i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                Row[i][j] = LocalMatrix[i][j];

        // Compute X scale factor and normalize first row.
        Scale.x = length(Row[0]);// v3Length(Row[0]);

        v3Scale(Row[0], static_cast<T>(1));

        // Compute XY shear factor and make 2nd row orthogonal to 1st.
        Skew.z = dot(Row[0], Row[1]);
        Row[1] = combine(Row[1], Row[0], static_cast<T>(1), -Skew.z);

        // Now, compute Y scale and normalize 2nd row.
        Scale.y = length(Row[1]);
        v3Scale(Row[1], static_cast<T>(1));
        Skew.z /= Scale.y;

        // Compute XZ and YZ shears, orthogonalize 3rd row.
        Skew.y = glm::dot(Row[0], Row[2]);
        Row[2] = combine(Row[2], Row[0], static_cast<T>(1), -Skew.y);
        Skew.x = glm::dot(Row[1], Row[2]);
        Row[2] = combine(Row[2], Row[1], static_cast<T>(1), -Skew.x);

        // Next, get Z scale and normalize 3rd row.
        Scale.z = length(Row[2]);
        v3Scale(Row[2], static_cast<T>(1));
        Skew.y /= Scale.z;
        Skew.x /= Scale.z;

        // At this point, the matrix (in rows[]) is orthonormal.
        // Check for a coordinate system flip.  If the determinant
        // is -1, then negate the matrix and the scaling factors.
        Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
        if (dot(Row[0], Pdum3) < 0)
        {
            for (length_t i = 0; i < 3; i++)
            {
                Scale.x *= static_cast<T>(-1);
                Row[i] *= static_cast<T>(-1);
            }
        }

        // Now, get the rotations out, as described in the gem.

        // FIXME - Add the ability to return either quaternions (which are
        // easier to recompose with) or Euler angles (rx, ry, rz), which
        // are easier for authors to deal with. The latter will only be useful
        // when we fix https://bugs.webkit.org/show_bug.cgi?id=23799, so I
        // will leave the Euler angle code here for now.

        // ret.rotateY = asin(-Row[0][2]);
        // if (cos(ret.rotateY) != 0) {
        //     ret.rotateX = atan2(Row[1][2], Row[2][2]);
        //     ret.rotateZ = atan2(Row[0][1], Row[0][0]);
        // } else {
        //     ret.rotateX = atan2(-Row[2][0], Row[1][1]);
        //     ret.rotateZ = 0;
        // }

        T s, t, x, y, z, w;

        t = Row[0][0] + Row[1][1] + Row[2][2] + 1.0;

        if (t > 1e-4)
        {
            s = 0.5 / sqrt(t);
            w = 0.25 / s;
            x = (Row[2][1] - Row[1][2]) * s;
            y = (Row[0][2] - Row[2][0]) * s;
            z = (Row[1][0] - Row[0][1]) * s;
        } else if (Row[0][0] > Row[1][1] && Row[0][0] > Row[2][2])
        {
            s = sqrt(1.0 + Row[0][0] - Row[1][1] - Row[2][2]) * 2.0; // S=4*qx 
            x = 0.25 * s;
            y = (Row[0][1] + Row[1][0]) / s;
            z = (Row[0][2] + Row[2][0]) / s;
            w = (Row[2][1] - Row[1][2]) / s;
        } else if (Row[1][1] > Row[2][2])
        {
            s = sqrt(1.0 + Row[1][1] - Row[0][0] - Row[2][2]) * 2.0; // S=4*qy
            x = (Row[0][1] + Row[1][0]) / s;
            y = 0.25 * s;
            z = (Row[1][2] + Row[2][1]) / s;
            w = (Row[0][2] - Row[2][0]) / s;
        } else
        {
            s = sqrt(1.0 + Row[2][2] - Row[0][0] - Row[1][1]) * 2.0; // S=4*qz
            x = (Row[0][2] + Row[2][0]) / s;
            y = (Row[1][2] + Row[2][1]) / s;
            z = 0.25 * s;
            w = (Row[1][0] - Row[0][1]) / s;
        }

        Orientation.x = x;
        Orientation.y = y;
        Orientation.z = z;
        Orientation.w = w;

        return true;
    }
}//namespace glm
namespace inviwo {

CameraTrackball::CameraTrackball(CameraProperty* cameraProp)
    : Trackball(&cameraProp->getLookFrom(), &cameraProp->getLookTo(), &cameraProp->getLookUp())
    , cameraProp_(cameraProp)
    , touchGesture_("touchGesture", "Touch",
    new TouchEvent(TouchEvent::TOUCH_STATE_ANY),
    new Action(this, &CameraTrackball::touchGesture)) {
    static_cast<TrackballObservable*>(this)->addObserver(this);
    cameraProp_->onChange(this, &CameraTrackball::onCameraPropertyChange);

    addProperty(touchGesture_);
}

CameraTrackball::~CameraTrackball() {}

void CameraTrackball::onAllTrackballChanged(const Trackball* trackball) {
    cameraProp_->updateViewMatrix();
}

void CameraTrackball::onLookFromChanged(const Trackball* trackball) {
    // Don't allow zooming such that the lookAt point is further away then the far plane.
    float maxDistance = cameraProp_->getFarPlaneDist() - (cameraProp_->getFarPlaneDist()*0.3f);
    float dist = glm::distance(cameraProp_->getLookTo(), cameraProp_->getLookFrom());
    if (maxDistance < dist)
        cameraProp_->setLookFrom(
            cameraProp_->getLookTo() +
            (glm::normalize(cameraProp_->getLookFrom() - cameraProp_->getLookTo()) * maxDistance));

    cameraProp_->updateViewMatrix();
}

void CameraTrackball::onLookToChanged(const Trackball* trackball) {
    cameraProp_->updateViewMatrix();
}

void CameraTrackball::onLookUpChanged(const Trackball* trackball) {
    cameraProp_->updateViewMatrix();
}

void CameraTrackball::onCameraPropertyChange(){
    setPanSpeedFactor(cameraProp_->getFovy()/60.f);
}

void CameraTrackball::touchGesture(Event* event) {
    TouchEvent* touchEvent = static_cast<TouchEvent*>(event);

    if (touchEvent->getTouchPoints().size() == 2) {
        vec2 v1 = touchEvent->getTouchPoints()[1].getPrevPosNormalized() - touchEvent->getTouchPoints()[0].getPrevPosNormalized();
        vec2 v2 = touchEvent->getTouchPoints()[1].getPosNormalized() - touchEvent->getTouchPoints()[0].getPosNormalized();
        float angle = glm::orientedAngle(glm::normalize(v2), glm::normalize(v1));
        float scale = glm::length(v1) / glm::length(v2);
        *lookUp_ = glm::rotate(*lookUp_, angle, (*lookTo_ - *lookFrom_));
        *lookFrom_ = *lookTo_ + (scale*(*lookFrom_ - *lookTo_));
        notifyAllChanged(this);

    } else 
        if (touchEvent->getTouchPoints().size() == 3) {
        // y -> Ax
        // Solve for A:
        // y x^-1 = A
        // Project points onto plane
        vec3 p[3];
        vec3 p2[3];

        //mat4 X;
        //mat4 Y;
        //for (auto i = 0; i < touchEvent->getTouchPoints().size(); ++i) {
        //    p[i] = cameraProp_->getWorldPosFromNormalizedDeviceCoords(vec3(2.f*touchEvent->getTouchPoints()[0].getPrevPosNormalized() - 1.f, 0));
        //    p2[i] = cameraProp_->getWorldPosFromNormalizedDeviceCoords(vec3(2.f*touchEvent->getTouchPoints()[0].getPosNormalized() - 1.f, 0));
        //    X[i] = vec4(p[i], 1.f);
        //    Y[i] = vec4(p2[i], 1.f);
        //}
        //
        //mat4 affineTransform = Y*glm::inverse(X);
        //*lookUp_ = glm::normalize((affineTransform* vec4((*lookUp_), 1.f)).xyz());
        mat3 X;
        mat3 Y;
        for (auto i = 0; i < touchEvent->getTouchPoints().size(); ++i) {
            p[i] = vec3(touchEvent->getTouchPoints()[i].getPrevPosNormalized(), 1);
            p2[i] = vec3(touchEvent->getTouchPoints()[i].getPosNormalized(), 1);
        }

        X[0] = p[0];
        X[1] = p[1];
        X[2] = p[2];

        Y[0] = p2[0];
        Y[1] = p2[1];
        Y[2] = p2[2];
        mat3 affineTransform = Y*glm::inverse(X);

        mat4 model; vec3 scale, translation, skew; vec4 perspective;
        quat orientation;
        glm::decompose(mat4(affineTransform), scale, orientation, translation, skew, perspective);
        LogInfo("\nScale: " << scale << "\nOrientiation: " << orientation << "\nTranslation: " << translation << "\nSkew: " << skew << "\nPerspective: " << perspective);
        
        //affineTransform[2] = vec3(0, 0, 1.f);

        mat4 TT = mat4(affineTransform)*cameraProp_->viewMatrix();
        LogInfo(TT);
        LogInfo(*lookFrom_);
        //glm::mat3x2 affineTransform = y*glm::inverse(x);
        auto rot = glm::quat(affineTransform);
        *lookFrom_ = *lookTo_ + (affineTransform* (*lookFrom_ - *lookTo_));
        //*lookUp_ = glm::normalize(affineTransform* (*lookUp_));
        //*lookUp_ = glm::rotate(rot, (*lookUp_));
        notifyAllChanged(this);
        LogInfo(affineTransform);

    }
}

}  // namespace