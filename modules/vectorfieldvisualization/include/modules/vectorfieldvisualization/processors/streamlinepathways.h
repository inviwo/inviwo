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
#include <inviwo/core/ports/meshport.h>
#include <modules/vectorfieldvisualization/algorithms/integrallineoperations.h>
#include <modules/vectorfieldvisualization/integrallinetracer.h>
#include <modules/vectorfieldvisualization/ports/seedpointsport.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>

namespace inviwo {

/** \docpage{org.inviwo.StreamlinePathways, Streamline Pathways}
 * ![](org.inviwo.StreamlinePathways.png?classIdentifier=org.inviwo.StreamlinePathways)
 * Given two areas, find streamliens that connect them.
 *
 * ### Inports
 *   * __sampler__ SpatialSampler to integrate in.
 *
 * ### Outports
 *   * __startMesh__ Cuboid mesh marking the start volume.
 *   * __endMesh__ Cuboid mesh marking the end volume.
 *
 * ### Properties
 *   * __startRegion__ The cuboid that integrations are started from.
 *   * __endRegion__ The end region that is tested against.
 */
class IVW_MODULE_VECTORFIELDVISUALIZATION_API StreamlinePathways : public Processor {
public:
    using Sampler = SpatialSampler<3, 3, double>;
    using Tracer = IntegralLineTracer<Sampler>;

    StreamlinePathways();
    virtual ~StreamlinePathways() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataInport<Sampler> sampler_;
    SeedPointsInport<3> seeds_;
    BrushingAndLinkingInport lineSelection_;
    MeshOutport startMesh_, endMesh_;
    IntegralLineSetOutport integralLines_;

    struct VolumeRegion : public CompositeProperty {

        VolumeRegion(const std::string& identifier, const std::string& displayName,
                     const vec4& defaultColor,
                     ConstraintBehavior limitBehavior = ConstraintBehavior::Mutable);
        void updateSamplerRegion(const dvec3& min, const dvec3& max);
        std::shared_ptr<Mesh> toMesh(const mat4& worldToModelMat) const;
        mat4 getAsMatrix(const dmat4& worldToModelMat) const;

        DoubleVec3Property min_, max_;
        FloatVec4Property color_;
    } startRegion_, endRegion_;

    IntegralLineProperties integrationProperties_;

    std::shared_ptr<IntegralLineSet> lines_;
    bool integrationRequired_;
};

}  // namespace inviwo
