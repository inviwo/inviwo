/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <inviwo/propertybasedtesting/processors/randomvolumegenerator.h>

#include <modules/base/algorithm/volume/volumegeneration.h>

#include <random>

namespace inviwo {

const ProcessorInfo RandomVolumeGenerator::processorInfo_{
    "org.inviwo.RandomVolumeGenerator",  // Class identifier
    "Random Volume Generator",           // Display name
    "Undefined",                         // Category
    CodeState::Experimental,             // Code state
    Tags::None,                          // Tags
};
const ProcessorInfo RandomVolumeGenerator::getProcessorInfo() const { return processorInfo_; }

RandomVolumeGenerator::RandomVolumeGenerator()
    : Processor()
    , outport_("volume")
    , seed_("seed", "Seed", 42, 0, INT_MAX)
    , numPoints_("numPoints", "Number of Points", 5, 1, 500)
    , resolution_("resolution", "Resolution", 32, 1, 1024) {

    addPort(outport_);
    addProperty(seed_);
    addProperty(numPoints_);
}

void RandomVolumeGenerator::process() {
    std::default_random_engine generator(seed_.get());
    std::uniform_real_distribution<double> dis(0.0, 1.0);

    const size3_t dimensions(resolution_.get());
    const mat3 basis(1.0);  // identity matrix

    std::vector<dvec3> points(numPoints_.get());
    for (auto& pos : points) {
        pos = dvec3(dis(generator), dis(generator), dis(generator));
    }

    std::shared_ptr<Volume> volume =
        util::generateVolume(dimensions, basis, [&](const size3_t& ind) {
            auto rel = dvec3(ind) / dvec3(dimensions);  // position clamped to [0,1]^3
            double dist = INFINITY;                     // distance to closest point
            for (const auto& point : points) {
                dist = std::min(dist, glm::length(rel - point));
            }
            return util::glm_convert_normalized<float>(dist);
        });
    outport_.setData(volume);
}

}  // namespace inviwo
