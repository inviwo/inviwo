/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <modules/vectorfieldvisualization/processors/seed3dto4d.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo Seed3Dto4D::processorInfo_{
    "org.inviwo.Seed3Dto4D",  // Class identifier
    "Seed3Dto4D",             // Display name
    "Seeding",                // Category
    CodeState::Stable,        // Code state
    Tags::CPU,                // Tags
};
const ProcessorInfo Seed3Dto4D::getProcessorInfo() const { return processorInfo_; }

Seed3Dto4D::Seed3Dto4D()
    : Processor()
    , seed3d_("seed3d_")
    , seed4d_("seed4d_")
    , w_("w", "4th component", 0.f, 0.f, 1.f, 0.01f) {

    addPort(seed3d_);
    addPort(seed4d_);
    addProperty(w_);
}

void Seed3Dto4D::process() {

    auto outvec = std::make_shared<SeedPoint4DVector>();
    for (auto &inData : seed3d_.getVectorData()) {
        const auto &inVec = *inData;
        outvec->reserve(inVec.size());
        for (const auto &p : inVec) {
            outvec->emplace_back(p, w_.get());
        }
    }
    seed4d_.setData(outvec);
}

}  // namespace inviwo
