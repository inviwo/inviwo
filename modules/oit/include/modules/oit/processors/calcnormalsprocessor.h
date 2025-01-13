/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <modules/oit/oitmoduledefine.h>  // for IVW_MODULE_MESHRENDERIN...

#include <inviwo/core/ports/meshport.h>             // for MeshInport, MeshOutport
#include <inviwo/core/processors/processor.h>       // for Processor
#include <inviwo/core/processors/processorinfo.h>   // for ProcessorInfo
#include <inviwo/core/properties/optionproperty.h>  // for OptionProperty
#include <inviwo/core/util/staticstring.h>          // for operator+
#include <modules/oit/algorithm/calcnormals.h>      // for CalculateMeshNormalsMode

#include <functional>   // for __base
#include <string>       // for operator==, operator+
#include <string_view>  // for operator==
#include <vector>       // for operator!=, vector, ope...

namespace inviwo {

/** \docpage{org.inviwo.CalcNormalsProcessor, Calculate Normals}
 * ![](org.inviwo.CalcNormalsProcessor.png?classIdentifier=org.inviwo.CalcNormalsProcessor)
 *
 * Calculates a normal buffer for a mesh
 *
 * ### Inports
 *   * __inport__ Mesh inport.
 *
 * ### Outports
 *   * __outport__ Mesh outport.
 *
 * ### Properties
 *   * __Mode__  The weighting modes for calculating normals
 *      * __PassThrough__ mesh is not changed
 *      * __NoWeighting__ no weighting of the normals, simple average
 *      * __WeightArea__  Weight = area of the triangle
 *      * __WeightAngle__ Weight based on the angle. As defined in "Computing vertex normals
 *                        from polygonal facets" by Grit Thürmer and Charles A. Wüthrich 1998.
 *      * __WeightNMax__  Based on "Weights for Computing Vertex Normals from Facet Normals",
 *                        N. Max, 1999. This gives the best results in most cases.
 *
 */
class IVW_MODULE_OIT_API CalcNormalsProcessor : public Processor {
public:
    CalcNormalsProcessor();
    virtual ~CalcNormalsProcessor() = default;

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    MeshInport inport_;
    MeshOutport outport_;
    OptionProperty<meshutil::CalculateMeshNormalsMode> mode_;
};

}  // namespace inviwo
