/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#include <modules/base/processors/mathprocessor.h>
#include <glm/gtx/euler_angles.hpp>
#include <limits>

namespace inviwo {

const ProcessorInfo MathProcessor::processorInfo_{
    "org.inviwo.MathProcessor",    // Class identifier
    "Math",                        // Display name
    "Math",                        // Category
    CodeState::Experimental,       // Code state
    Tags::CPU,                     // Tags
};
const ProcessorInfo MathProcessor::getProcessorInfo() const { return processorInfo_; }

MathProcessor::MathProcessor() : Processor()
    , eulerAnglesToRotationMatrix_("eulerAnglesToRotationMatrix", "Euler Angles to Rotation Matrix")
    , eulerAngles_("eulerAngles", "Euler Angles", vec3(0.0f), vec3(-glm::two_pi<float>()), vec3(glm::two_pi<float>()))
    , rotationMatrix_("rotationMatrix", "Rotation Matrix", mat4(1.0f), mat4(0.0f) - 1e9f, mat4(0.0f) + 1e9f)
    , invertMatrix_("invertMatrix", "Invert Matrix")
    , matrixToInvert_("matrixToInvert", "Matrix to Invert", mat4(1.0f), mat4(0.0f) - 1e9f, mat4(0.0f) + 1e9f)
    , matrixInverse_("matrixInverse", "Matrix Inverse", mat4(1.0f), mat4(0.0f) - 1e9f, mat4(0.0f) + 1e9f)
    , Vec3FloatElementCombiner_("Vec3FloatElementCombiner", "3 Floats <-> Vec3Float")
    , vec3x_("vec3x", "X", 0.0f, -1e9f, 1e9f)
    , vec3y_("vec3y", "Y", 0.0f, -1e9f, 1e9f)
    , vec3z_("vec3z", "Z", 0.0f, -1e9f, 1e9f)
    , combinedXYZ_("combinedXYZ", "XYZ", vec3(0.0f), vec3(-1e9f), vec3(1e9f))
    , vec3Scale_("Vec3Scale", "Vec3 Scaling")
    , vec3ScaleIn_("vec3ScaleIn", "Vec3 In", vec3(1.0f), vec3(-1e9f), vec3(1e9f))
    , vec3ScalingFactors_("vec3ScalingFactors", "Vec3 Scaling Factors", vec3(1.0f), vec3(-1e9f), vec3(1e9f))
    , vec3ScaleOut_("vec3ScaleOut", "Vec3 Out", vec3(1.0f), vec3(-1e9f), vec3(1e9f))
    , vec3Transform_("vec3Transform", "Vec3 Transform")
    , vec3TransformIn_("vec3TransformIn", "Vec3 In", vec3(1.0f), vec3(-1e9f), vec3(1e9f))
    , mat4TransformationMatrix_("mat4TransformationMatrix", "Transformation Matrix", mat4(1.0f), mat4(0.0f) - 1e9f, mat4(0.0f) + 1e9f)
    , vec3TransformOut_("vec3TransformOut", "Vec3 Out", vec3(1.0f), vec3(-1e9f), vec3(1e9f))
    , rotationAroundVec3_("rotationAroundVec3", "Rotation around Vec3")
    , vec3RotationAroundIn_("vec3RotationAroundIn", "Vec3 In", vec3(1.0f), vec3(-1e9f), vec3(1e9f))
    , angleRotationAroundIn_("angleRotationAroundIn", "Angle In", 0.0f, -1e9f, 1e9f)
    , mat4RotationAroundOut_("mat4RotationAroundOut", "Rotation Matrix", mat4(1.0f), mat4(0.0f) - 1e9f, mat4(0.0f) + 1e9f)
    , chainTransformations_("chainTransformations", "Transformation Chain")
    , transformIn1_("transformIn1", "Transformation 1", mat4(1.0f), mat4(0.0f) - 1e9f, mat4(0.0f) + 1e9f)
    , transformIn2_("transformIn2", "Transformation 2", mat4(1.0f), mat4(0.0f) - 1e9f, mat4(0.0f) + 1e9f)
    , transformOut_("transformOut", "Result", mat4(1.0f), mat4(0.0f) - 1e9f, mat4(0.0f) + 1e9f)
    , tripleLocalRotation_("tripleLocalRotation", "Triple Local Rotation")
    , vec3TripleRotateIn1_("vec3TripleRotateIn1_", "Vec3 In 1", vec3(1.0f), vec3(-1e9f), vec3(1e9f))
    , vec3TripleRotateIn2_("vec3TripleRotateIn2_", "Vec3 In 2", vec3(1.0f), vec3(-1e9f), vec3(1e9f))
    , vec3TripleRotateIn3_("vec3TripleRotateIn3_", "Vec3 In 3", vec3(1.0f), vec3(-1e9f), vec3(1e9f))
    , angleTripleRotateIn1_("angleTripleRotateIn1", "Angle In 1", 0.0f, -1e9f, 1e9f)
    , angleTripleRotateIn2_("angleTripleRotateIn2", "Angle In 2", 0.0f, -1e9f, 1e9f)
    , angleTripleRotateIn3_("angleTripleRotateIn3", "Angle In 3", 0.0f, -1e9f, 1e9f)
    , tripleRotationOut_("tripleRotationOut", "Result", mat4(1.0f), mat4(0.0f) - 1e9f, mat4(0.0f) + 1e9f)
    , mprTransform_("mprTransform_", "MPR Transform")
    , mprResetButton_("mprResetButton_", "Reset MPR")
    , mprBasis1In_("mprBasis1In_", "Basis In 1", vec3(1.0f, 0.0f, 0.0f), vec3(-1e9f), vec3(1e9f))
    , mprBasis2In_("mprBasis2In_", "Basis In 2", vec3(0.0f, 1.0f, 0.0f), vec3(-1e9f), vec3(1e9f))
    , mprBasis3In_("mprBasis3In_", "Basis In 3", vec3(0.0f, 0.0f, 1.0f), vec3(-1e9f), vec3(1e9f))
    , mprBasis1_(mprBasis1In_.get())
    , mprBasis2_(mprBasis2In_.get())
    , mprBasis3_(mprBasis3In_.get())
    , mprAngle1In_("mprAngle1In_", "Angle In 1", 0.0f, -1e9f, 1e9f)
    , mprAngle2In_("mprAngle2In_", "Angle In 2", 0.0f, -1e9f, 1e9f)
    , mprAngle3In_("mprAngle3In_", "Angle In 3", 0.0f, -1e9f, 1e9f)
    , mprAngle1_old_(mprAngle1In_.get())
    , mprAngle2_old_(mprAngle2In_.get())
    , mprAngle3_old_(mprAngle3In_.get())
    , mprBasis1Out_("mprBasis1Out_", "Basis Out 1", mprBasis1In_.get(), vec3(-1e9f), vec3(1e9f))
    , mprBasis2Out_("mprBasis2Out_", "Basis Out 2", mprBasis2In_.get(), vec3(-1e9f), vec3(1e9f))
    , mprBasis3Out_("mprBasis3Out_", "Basis Out 3", mprBasis3In_.get(), vec3(-1e9f), vec3(1e9f))
{
    // euler angles to rotation matrix
    eulerAngles_.onChange([this]() { rotationMatrix_ = glm::orientate4(eulerAngles_.get()); });
    eulerAnglesToRotationMatrix_.addProperty(eulerAngles_);
    rotationMatrix_.setReadOnly(true);
    eulerAnglesToRotationMatrix_.addProperty(rotationMatrix_);
    eulerAnglesToRotationMatrix_.setCollapsed(true);
    addProperty(eulerAnglesToRotationMatrix_);

    // invert matrix
    matrixToInvert_.onChange([this]() { matrixInverse_ = glm::inverse(matrixToInvert_.get()); });
    invertMatrix_.addProperty(matrixToInvert_);
    matrixInverse_.setReadOnly(true);
    invertMatrix_.addProperty(matrixInverse_);
    invertMatrix_.setCollapsed(true);
    addProperty(invertMatrix_);

    // combine floats to vec3 or split vec3 to floats
    vec3x_.onChange([this]() { combinedXYZ_ = vec3{vec3x_, vec3y_, vec3z_}; });
    vec3y_.onChange([this]() { combinedXYZ_ = vec3{vec3x_, vec3y_, vec3z_}; });
    vec3z_.onChange([this]() { combinedXYZ_ = vec3{vec3x_, vec3y_, vec3z_}; });
    combinedXYZ_.onChange([this]() {
        const auto& v = combinedXYZ_.get();
        vec3x_ = v.x;
        vec3y_ = v.y;
        vec3z_ = v.z;
    });
    Vec3FloatElementCombiner_.addProperty(vec3x_);
    Vec3FloatElementCombiner_.addProperty(vec3y_);
    Vec3FloatElementCombiner_.addProperty(vec3z_);
    Vec3FloatElementCombiner_.addProperty(combinedXYZ_);
    Vec3FloatElementCombiner_.setCollapsed(true);
    addProperty(Vec3FloatElementCombiner_);

    // scale vector
    vec3ScaleIn_.onChange([this]() { vec3ScaleOut_ = vec3ScalingFactors_.get() * vec3ScaleIn_.get(); });
    vec3ScalingFactors_.onChange([this]() { vec3ScaleOut_ = vec3ScalingFactors_.get() * vec3ScaleIn_.get(); });
    vec3Scale_.addProperty(vec3ScaleIn_);
    vec3Scale_.addProperty(vec3ScalingFactors_);
    vec3ScaleOut_.setReadOnly(true);
    vec3Scale_.addProperty(vec3ScaleOut_);
    vec3Scale_.setCollapsed(true);
    addProperty(vec3Scale_);

    // transform vector
    vec3TransformIn_.onChange([this]() { vec3TransformOut_ = vec3(mat4TransformationMatrix_.get() * vec4(vec3TransformIn_.get(), 0.0f)); });
    mat4TransformationMatrix_.onChange([this]() { vec3TransformOut_ = vec3(mat4TransformationMatrix_.get() * vec4(vec3TransformIn_.get(), 0.0f)); });
    vec3Transform_.addProperty(vec3TransformIn_);
    vec3Transform_.addProperty(mat4TransformationMatrix_);
    vec3TransformOut_.setReadOnly(true);
    vec3Transform_.addProperty(vec3TransformOut_);
    vec3Transform_.setCollapsed(true);
    addProperty(vec3Transform_);

    // rotation around vector
    vec3RotationAroundIn_.onChange([this]() { mat4RotationAroundOut_ = glm::rotate(angleRotationAroundIn_.get(), vec3RotationAroundIn_.get()); });
    angleRotationAroundIn_.onChange([this]() { mat4RotationAroundOut_ = glm::rotate(angleRotationAroundIn_.get(), vec3RotationAroundIn_.get()); });
    rotationAroundVec3_.addProperty(vec3RotationAroundIn_);
    rotationAroundVec3_.addProperty(angleRotationAroundIn_);
    mat4RotationAroundOut_.setReadOnly(true);
    rotationAroundVec3_.addProperty(mat4RotationAroundOut_);
    rotationAroundVec3_.setCollapsed(true);
    addProperty(rotationAroundVec3_);

    // chain trainsformations
    transformIn1_.onChange([this]() { transformOut_ = transformIn2_.get() * transformIn1_.get(); });
    transformIn2_.onChange([this]() { transformOut_ = transformIn2_.get() * transformIn1_.get(); });
    chainTransformations_.addProperty(transformIn1_);
    chainTransformations_.addProperty(transformIn2_);
    transformOut_.setReadOnly(true);
    chainTransformations_.addProperty(transformOut_);
    chainTransformations_.setCollapsed(true);
    addProperty(chainTransformations_);

    // angle triple rotation
    vec3TripleRotateIn1_.onChange([this]() { tripleRotationOut_ = tripleLocalRotation(); });
    vec3TripleRotateIn2_.onChange([this]() { tripleRotationOut_ = tripleLocalRotation(); });
    vec3TripleRotateIn3_.onChange([this]() { tripleRotationOut_ = tripleLocalRotation(); });
    angleTripleRotateIn1_.onChange([this]() { tripleRotationOut_ = tripleLocalRotation(); });
    angleTripleRotateIn2_.onChange([this]() { tripleRotationOut_ = tripleLocalRotation(); });
    angleTripleRotateIn3_.onChange([this]() { tripleRotationOut_ = tripleLocalRotation(); });
    tripleLocalRotation_.addProperty(vec3TripleRotateIn1_);
    tripleLocalRotation_.addProperty(vec3TripleRotateIn2_);
    tripleLocalRotation_.addProperty(vec3TripleRotateIn3_);
    tripleLocalRotation_.addProperty(angleTripleRotateIn1_);
    tripleLocalRotation_.addProperty(angleTripleRotateIn2_);
    tripleLocalRotation_.addProperty(angleTripleRotateIn3_);
    tripleRotationOut_.setReadOnly(true);
    tripleLocalRotation_.addProperty(tripleRotationOut_);
    tripleLocalRotation_.setCollapsed(true);
    addProperty(tripleLocalRotation_);

    // mpr transform
    mprResetButton_.onChange([this]() {
        // reset basis
        mprBasis1_ = mprBasis1In_.get();
        mprBasis2_ = mprBasis2In_.get();
        mprBasis3_ = mprBasis3In_.get();
        // reset output
        mprBasis1Out_ = mprBasis1In_.get();
        mprBasis2Out_ = mprBasis2In_.get();
        mprBasis3Out_ = mprBasis3In_.get();
        // reset rotation angles
        mprAngle1In_ = 0.0f;
        mprAngle2In_ = 0.0f;
        mprAngle3In_ = 0.0f;
        mprAngle1_old_ = 0.0f;
        mprAngle2_old_ = 0.0f;
        mprAngle3_old_ = 0.0f;
    });

    mprBasis1In_.onChange([this]() { mprBasis1_ = mprBasis1In_.get(); });
    mprBasis2In_.onChange([this]() { mprBasis2_ = mprBasis2In_.get(); });
    mprBasis3In_.onChange([this]() { mprBasis3_ = mprBasis3In_.get(); });

    mprAngle1In_.onChange([this]() { mprAngle1Changed(); });
    mprAngle2In_.onChange([this]() { mprAngle2Changed(); });
    mprAngle3In_.onChange([this]() { mprAngle3Changed(); });

    mprBasis1Out_.setReadOnly(true);
    mprBasis2Out_.setReadOnly(true);
    mprBasis3Out_.setReadOnly(true);

    mprTransform_.addProperty(mprResetButton_);
    mprTransform_.addProperty(mprBasis1In_);
    mprTransform_.addProperty(mprBasis2In_);
    mprTransform_.addProperty(mprBasis3In_);
    mprTransform_.addProperty(mprAngle1In_);
    mprTransform_.addProperty(mprAngle2In_);
    mprTransform_.addProperty(mprAngle3In_);
    mprTransform_.addProperty(mprBasis1Out_);
    mprTransform_.addProperty(mprBasis2Out_);
    mprTransform_.addProperty(mprBasis3Out_);
    mprTransform_.setCollapsed(true);
    addProperty(mprTransform_);
}

void MathProcessor::mprAngle1Changed()
{
    // signed angle offset
    const auto angle_offset = mprAngle1In_.get() - mprAngle1_old_;

    // rotate around basis 1
    const auto R = glm::rotate(angle_offset, mprBasis1_);
    mprBasis2_ = glm::normalize(vec3(R * vec4(mprBasis2_, 0.0f)));
    mprBasis3_ = glm::normalize(vec3(R * vec4(mprBasis3_, 0.0f)));

    // make basis square -> correct for numeric errors
    const auto tmp = glm::normalize(glm::cross(mprBasis2_, mprBasis3_));
    mprBasis3_ = glm::normalize(glm::cross(tmp, mprBasis2_));

    // set result
    mprBasis2Out_ = mprBasis2_;
    mprBasis3Out_ = mprBasis3_;

    // store new angle
    mprAngle1_old_ = mprAngle1In_.get();
}

void MathProcessor::mprAngle2Changed()
{
    // signed angle offset
    const auto angle_offset = mprAngle2In_.get() - mprAngle2_old_;

    // rotate around basis 2
    const auto R = glm::rotate(angle_offset, mprBasis2_);
    mprBasis1_ = glm::normalize(vec3(R * vec4(mprBasis1_, 0.0f)));
    mprBasis3_ = glm::normalize(vec3(R * vec4(mprBasis3_, 0.0f)));

    // make basis square -> correct for numeric errors
    const auto tmp = glm::normalize(glm::cross(mprBasis1_, mprBasis3_));
    mprBasis3_ = glm::normalize(glm::cross(tmp, mprBasis1_));

    // set result
    mprBasis1Out_ = mprBasis1_;
    mprBasis3Out_ = mprBasis3_;

    // store new angle
    mprAngle2_old_ = mprAngle2In_.get();
}

void MathProcessor::mprAngle3Changed()
{
    // signed angle offset
    const auto angle_offset = mprAngle3In_.get() - mprAngle3_old_;

    // rotate around basis 3
    const auto R = glm::rotate(angle_offset, mprBasis3_);
    mprBasis1_ = glm::normalize(vec3(R * vec4(mprBasis1_, 0.0f)));
    mprBasis2_ = glm::normalize(vec3(R * vec4(mprBasis2_, 0.0f)));

    // make basis square -> correct for numeric errors
    const auto tmp = glm::normalize(glm::cross(mprBasis1_, mprBasis2_));
    mprBasis2_ = glm::normalize(glm::cross(tmp, mprBasis1_));

    // set result
    mprBasis1Out_ = mprBasis1_;
    mprBasis2Out_ = mprBasis2_;

    // store new angle
    mprAngle3_old_ = mprAngle3In_.get();
}

mat4 MathProcessor::tripleLocalRotation(void)
{
    // rotate around first axis in first local coord. sys.
    const auto R1 = glm::rotate(angleTripleRotateIn1_.get(), vec3TripleRotateIn1_.get());

    // bring 2nd vector in 1st local coord. sys. and rotate there
    const auto v2 = vec3(R1 * vec4(vec3TripleRotateIn2_.get(), 0.0f));
    const auto R2 = glm::rotate(angleTripleRotateIn2_.get(), v2) * R1;

    // bring 3rd vector in 2nd local coord. sys. and rotate there
    const auto v3 = vec3(R2 * vec4(vec3TripleRotateIn3_.get(), 0.0f));
    const auto R3 = glm::rotate(angleTripleRotateIn3_.get(), v3) * R2;

    // return 3 times local rotation
    return R3;
}


}  // namespace inviwo
