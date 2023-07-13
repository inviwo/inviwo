/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2021 Inviwo Foundation
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

#pragma once

#include <modules/vectorfieldvisualization/vectorfieldvisualizationmoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processortraits.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/foreach.h>
#include <modules/vectorfieldvisualization/algorithms/integrallineoperations.h>
#include <modules/vectorfieldvisualization/integrallinetracer.h>
#include <modules/vectorfieldvisualization/ports/seedpointsport.h>

namespace inviwo {

class HighVelocityLineRegions : public Processor {
    using SamplerVec3 = SpatialSampler<3, 3, double>;
    // using SamplerFloat = SpatialSampler<3, 1, double>;
    using Tracer = IntegralLineTracer<SamplerVec3>;

public:
    HighVelocityLineRegions();
    virtual ~HighVelocityLineRegions();

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    enum MeasureType : int {
        MaxScalar,                 // Maximum scalar value along line.
        ScalarMinLengthThreshold,  // Binary. Does the line have a piece with length
                                   // integrationProperties_.numberOfSteps of values above
                                   // scalarThreshold_?
        MaxScalarForLength,  // What's the maximum scalar threshold for which we find a line piece
                             // of length integrationProperties_.numberOfSteps?
    };

private:
    DataInport<SamplerVec3> velocitySampler_, scalarSampler_;
    // DataInport<SamplerFloat> scalarSampler_;
    VolumeOutport volumeOut_;
    IntegralLineSetOutport linesOut_;

    TemplateOptionProperty<MeasureType> measureType_;

    IntegralLineProperties integrationProperties_;
    IntSize3Property fieldSize_;
    DoubleProperty fieldSubSize_;
    DoubleVec2Property dataRange_;

    DoubleProperty scalarThreshold_;
    BoolProperty filterLines_, topLinesOnly_;

    std::shared_ptr<IntegralLineSet> lines_;
};

}  // namespace inviwo
