/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2022 Inviwo Foundation
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

#include <modules/meshrenderinggl/processors/calcnormalsprocessor.h>

#include <inviwo/core/datastructures/geometry/mesh.h>       // for Mesh
#include <inviwo/core/ports/meshport.h>                     // for MeshInport, MeshOutport
#include <inviwo/core/processors/processor.h>               // for Processor
#include <inviwo/core/processors/processorinfo.h>           // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>          // for CodeState, CodeState::Experim...
#include <inviwo/core/processors/processortags.h>           // for Tags, Tags::CPU
#include <inviwo/core/properties/optionproperty.h>          // for OptionPropertyOption, OptionP...
#include <inviwo/core/util/staticstring.h>                  // for operator+
#include <modules/meshrenderinggl/algorithm/calcnormals.h>  // for CalculateMeshNormalsMode, cal...

#include <memory>                                           // for shared_ptr

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo CalcNormalsProcessor::processorInfo_{
    "org.inviwo.CalcNormalsProcessor",  // Class identifier
    "Calculate Normals",                // Display name
    "Mesh Processing",                  // Category
    CodeState::Experimental,            // Code state
    Tags::CPU,                          // Tags
};
const ProcessorInfo CalcNormalsProcessor::getProcessorInfo() const { return processorInfo_; }

CalcNormalsProcessor::CalcNormalsProcessor()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , mode_("mode", "Mode",
            {
                {"passThrough", "Pass Through", meshutil::CalculateMeshNormalsMode::PassThrough},
                {"noWeighting", "No Weighting", meshutil::CalculateMeshNormalsMode::NoWeighting},
                {"area", "Area-weighting", meshutil::CalculateMeshNormalsMode::WeightArea},
                {"angle", "Angle-weighting", meshutil::CalculateMeshNormalsMode::WeightAngle},
                {"nmax", "Based on N.Max", meshutil::CalculateMeshNormalsMode::WeightNMax},
            },
            4) {
    addPort(inport_);
    addPort(outport_);

    addProperty(mode_);
}

void CalcNormalsProcessor::process() {
    outport_.setData(
        std::shared_ptr<Mesh>(meshutil::calculateMeshNormals(*inport_.getData(), mode_)));
}

}  // namespace inviwo
