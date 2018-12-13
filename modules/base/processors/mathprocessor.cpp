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
    , rotationMatrix_("rotationMatrix", "Rotation Matrix", mat4(1.0f), mat4(-1e9f), mat4(1e9f))
    , invertMatrix_("invertMatrix", "Invert Matrix")
    , matrixToInvert_("matrixToInvert", "Matrix to Invert", mat4(1.0f), mat4(-1e9f), mat4(1e9f))
    , matrixInverse_("matrixInverse", "Matrix Inverse", mat4(1.0f), mat4(-1e9f), mat4(1e9f))
    , Vec3FloatElementCombiner_("Vec3FloatElementCombiner", "3 Floats <-> Vec3Float")
    , vec3x_("vec3x", "X", 0.0f, -1e9f, 1e9f)
    , vec3y_("vec3y", "Y", 0.0f, -1e9f, 1e9f)
    , vec3z_("vec3z", "Z", 0.0f, -1e9f, 1e9f)
    , combinedXYZ_("combinedXYZ", "XYZ", vec3(0.0f), vec3(-1e9f), vec3(1e9f))
    , vec3Scale_("Vec3Scale", "Vec3 Scaling")
    , vec3ScaleIn_("vec3ScaleIn", "Vec3 In", vec3(1.0f), vec3(-1e9f), vec3(1e9f))
    , vec3ScalingFactors_("vec3ScalingFactors", "Vec3 Scaling Factors", vec3(1.0f), vec3(-1e9f), vec3(1e9f))
    , vec3ScaleOut_("vec3ScaleOut", "Vec3 Out", vec3(1.0f), vec3(-1e9f), vec3(1e9f))
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
}

}  // namespace inviwo
