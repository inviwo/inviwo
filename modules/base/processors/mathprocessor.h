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
};
}  // namespace inviwo

#endif // IVW_MATHPROCESSOR_H
