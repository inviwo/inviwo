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

#ifndef IVW_MATHPROCESSOR_H
#define IVW_MATHPROCESSOR_H

#include <modules/base/basemoduledefine.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>

#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/buttonproperty.h>

namespace inviwo {
/**
 * \class
 * \brief
 */
class IVW_MODULE_BASE_API MathProcessor : public Processor {
public:
    MathProcessor();
    virtual ~MathProcessor() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    mat4 tripleLocalRotation(void);

    void mprAngle1Changed(void);
    void mprAngle2Changed(void);
    void mprAngle3Changed(void);

private:
    CompositeProperty eulerAnglesToRotationMatrix_;
    FloatVec3Property eulerAngles_;
    FloatMat4Property rotationMatrix_;

    CompositeProperty invertMatrix_;
    FloatMat4Property matrixToInvert_;
    FloatMat4Property matrixInverse_;

    CompositeProperty Vec3FloatElementCombiner_;
    FloatProperty vec3x_;
    FloatProperty vec3y_;
    FloatProperty vec3z_;
    FloatVec3Property combinedXYZ_;

    CompositeProperty vec3Scale_;
    FloatVec3Property vec3ScaleIn_;
    FloatVec3Property vec3ScalingFactors_;
    FloatVec3Property vec3ScaleOut_;

    CompositeProperty vec3Transform_;
    FloatVec3Property vec3TransformIn_;
    FloatMat4Property mat4TransformationMatrix_;
    FloatVec3Property vec3TransformOut_;

    CompositeProperty rotationAroundVec3_;
    FloatVec3Property vec3RotationAroundIn_;
    FloatProperty angleRotationAroundIn_;
    FloatMat4Property mat4RotationAroundOut_;

    CompositeProperty chainTransformations_;
    FloatMat4Property transformIn1_;
    FloatMat4Property transformIn2_;
    FloatMat4Property transformOut_;

    CompositeProperty tripleLocalRotation_;
    FloatVec3Property vec3TripleRotateIn1_;
    FloatVec3Property vec3TripleRotateIn2_;
    FloatVec3Property vec3TripleRotateIn3_;
    FloatProperty angleTripleRotateIn1_;
    FloatProperty angleTripleRotateIn2_;
    FloatProperty angleTripleRotateIn3_;
    FloatMat4Property tripleRotationOut_;

    CompositeProperty mprTransform_;
    ButtonProperty mprResetButton_;
    FloatVec3Property mprBasis1In_;
    FloatVec3Property mprBasis2In_;
    FloatVec3Property mprBasis3In_;
    vec3 mprBasis1_;
    vec3 mprBasis2_;
    vec3 mprBasis3_;
    FloatProperty mprAngle1In_;
    FloatProperty mprAngle2In_;
    FloatProperty mprAngle3In_;
    float mprAngle1_old_;
    float mprAngle2_old_;
    float mprAngle3_old_;
    FloatVec3Property mprBasis1Out_;
    FloatVec3Property mprBasis2Out_;
    FloatVec3Property mprBasis3Out_;
};
}  // namespace inviwo

#endif // IVW_MATHPROCESSOR_H
