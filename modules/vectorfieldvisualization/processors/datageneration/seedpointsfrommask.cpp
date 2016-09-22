/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/processors/datageneration/seedpointsfrommask.h>
#include <inviwo/core/datastructures/volume/volumeram.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo SeedPointsFromMask::processorInfo_{
    "org.inviwo.SeedPointsFromMask",  // Class identifier
    "Seed Points From Mask",          // Display name
    "Vector Field Visualization",     // Category
    CodeState::Experimental,          // Code state
    Tags::None,                       // Tags
};
const ProcessorInfo SeedPointsFromMask::getProcessorInfo() const { return processorInfo_; }

SeedPointsFromMask::SeedPointsFromMask() : Processor(), volumes_("volumes"), seedPoints_("seeds") {
    addPort(volumes_);
    addPort(seedPoints_);
}

void SeedPointsFromMask::process() {
    auto points = std::make_shared<std::vector<vec3>>();

    for (const auto &v : volumes_) {
        auto dim = v->getDimensions();
        auto data = static_cast<const unsigned char *>(
            v->getRepresentation<VolumeRAM>()->getData());  // TODO make a dispatch
        size3_t pos;
        size_t i = 0;
        for (pos.z = 0; pos.z < dim.z; pos.z++) {
            for (pos.y = 0; pos.y < dim.y; pos.y++) {
                for (pos.x = 0; pos.x < dim.x; pos.x++) {
                    if (data[i] != 0) {
                        points->push_back(vec3(pos) / vec3(dim - size3_t(1, 1, 1)));
                    }
                    i++;
                }
            }
        }
    }
    seedPoints_.setData(points);
}

}  // namespace