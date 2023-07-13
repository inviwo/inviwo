/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
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
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/ports/meshport.h>
#include <modules/vectorfieldvisualization/integrallinetracer.h>
#include <modules/vectorfieldvisualization/datastructures/integrallineset.h>
#include <modules/vectorfieldvisualization/ports/seedpointsport.h>

namespace inviwo {

/** \docpage{org.inviwo.ShowStreamPath, Show Stream Path}
 * ![](org.inviwo.ShowStreamPath.png?classIdentifier=org.inviwo.ShowStreamPath)
 * Takes the output of the StreamSpanningTree processor and shows the path to a chosen end point.
 */
class IVW_MODULE_VECTORFIELDVISUALIZATION_API ShowStreamPath : public Processor {
public:
    ShowStreamPath();
    virtual ~ShowStreamPath() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    using Sampler = SpatialSampler<3, 3, double>;
    using Tracer = IntegralLineTracer<Sampler>;

    // Inports
    DataInport<Sampler> velocitySampler_;
    VolumeInport indexVolume_;
    VolumeInport destinationVolume_;

    // Properties
    IntSize3Property destination_;
    IntegralLineProperties integrationProperties_;
    IntSizeTProperty samplingStride_;
    FloatVec4Property lineColor_, jumpColor_;
    BoolProperty markJumps_;
    BoolProperty integrate_;

    // Outports
    MeshOutport pointMesh_, lineMesh_, jumpMesh_;
    IntegralLineSetOutport integralLines_;
    SeedPoints3DOutport endPointSeed_;
};

}  // namespace inviwo
