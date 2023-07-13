/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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
#include <modules/vectorfieldvisualization/algorithms/integrallineoperations.h>
#include <modules/vectorfieldvisualization/integrallinetracer.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>
#include <inviwo/core/processors/poolprocessor.h>

namespace inviwo {

/** \docpage{org.inviwo.PathsBetweenRegions, Streamline Pathways}
 * ![](org.inviwo.PathsBetweenRegions.png?classIdentifier=org.inviwo.PathsBetweenRegions)
 * Given two areas, find streamliens that connect them.
 *
 * ### Inports
 *   * __sampler__ SpatialSampler to integrate in.
 *   * __startEndVolume__ Volume with valid region (x) and flags for start and end regions (y).
 *
 * ### Outports
 *   * __lines__ The lines connecting the two regions.
 *   * __startEndVolume__ Volume marking start cells (x) that made it through and the end cells (y)
 * that were reached.
 *
 * ### Properties
 *   * __integrationProperties__
 */
class IVW_MODULE_VECTORFIELDVISUALIZATION_API PathsBetweenRegions : public PoolProcessor {
public:
    using Sampler = SpatialSampler<3, 3, double>;
    using Tracer = IntegralLineTracer<Sampler>;

    PathsBetweenRegions();
    virtual ~PathsBetweenRegions() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataInport<Sampler> sampler_;
    VolumeInport startEndVolume_;  // This volume is expected to contain:
                                   // U: all valid nodes for the graph search (!=0)
                                   // V: the start nodes (>0) and end nodes (<0).

    IntegralLineProperties integrationProperties_;
    BoolProperty filter_;

    VolumeOutport validStartEndVolume_;
    IntegralLineSetOutport integralLines_;
};

}  // namespace inviwo
