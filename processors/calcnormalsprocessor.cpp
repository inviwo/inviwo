/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#include <fancymeshrenderer/processors/calcnormalsprocessor.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo CalcNormalsProcessor::processorInfo_{
    "org.inviwo.CalcNormalsProcessor",      // Class identifier
    "Calculate Normals",                // Display name
    "Mesh Processing",              // Category
    CodeState::Experimental,  // Code state
    Tags::CPU,               // Tags
};
const ProcessorInfo CalcNormalsProcessor::getProcessorInfo() const {
    return processorInfo_;
}

CalcNormalsProcessor::CalcNormalsProcessor()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , mode_("mode", "Mode")
{
    addPort(inport_);
    addPort(outport_);

    mode_.addOption("passThrough", "Pass Through", CalcNormals::Mode::PassThrough);
    mode_.addOption("noWeighting", "No Weighting", CalcNormals::Mode::NoWeighting);
    mode_.addOption("area", "Area-weighting", CalcNormals::Mode::WeightArea);
    mode_.addOption("angle", "Angle-weighting", CalcNormals::Mode::WeightAngle);
    mode_.addOption("nmax", "Based on N.Max", CalcNormals::Mode::WeightNMax);
    mode_.setSelectedValue(CalcNormals::preferredMode());
    mode_.setCurrentStateAsDefault();
    addProperty(mode_);
}
    
void CalcNormalsProcessor::process() {
    auto vd = inport_.getVectorData();
    if (vd.size() == 1)
    {
        outport_.setData(std::shared_ptr<Mesh>(CalcNormals().processMesh(vd[0].get(), mode_.get())));
    } else
    {
        std::shared_ptr<Mesh> m = std::make_shared<Mesh>();
        for (auto i : vd)
        {
            auto i2 = CalcNormals().processMesh(i.get(), mode_.get());
            m->append(*i2);
            delete i2;
        }
        outport_.setData(m);
    }
}

} // namespace

