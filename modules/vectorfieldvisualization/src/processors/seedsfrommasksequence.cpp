/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2022 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/processors/seedsfrommasksequence.h>

#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/volume/volumeram.h>                // for VolumeRAM
#include <inviwo/core/datastructures/volume/volumeramprecision.h>       // IWYU pragma: keep
#include <inviwo/core/ports/volumeport.h>                               // for VolumeSequenceInport
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags, Tags::CPU
#include <inviwo/core/properties/ordinalproperty.h>                     // for FloatProperty
#include <inviwo/core/util/glmconvert.h>                                // for glm_convert
#include <inviwo/core/util/glmvec.h>                                    // for vec3, size3_t
#include <inviwo/core/util/indexmapper.h>                               // for IndexMapper, Inde...
#include <inviwo/core/util/volumeramutils.h>                            // for forEachVoxel
#include <inviwo/core/util/volumesequenceutils.h>                       // for getTimestamp, has...
#include <modules/vectorfieldvisualization/ports/seedpointsport.h>      // for SeedPoint4DVector

#include <cstddef>                                                      // for size_t
#include <memory>                                                       // for shared_ptr, make_...
#include <random>                                                       // for mt19937, uniform_...
#include <string>                                                       // for string
#include <string_view>                                                  // for string_view
#include <type_traits>                                                  // for remove_extent_t
#include <unordered_set>                                                // for unordered_set
#include <vector>                                                       // for vector

#include <glm/vec3.hpp>                                                 // for operator*, operator+

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo SeedsFromMaskSequence::processorInfo_{
    "org.inviwo.SeedsFromMaskSequence",  // Class identifier
    "Seed Points From Mask Sequence",    // Display name
    "Seed Points",                       // Category
    CodeState::Stable,                   // Code state
    Tags::CPU,                           // Tags
};
const ProcessorInfo SeedsFromMaskSequence::getProcessorInfo() const { return processorInfo_; }

SeedsFromMaskSequence::SeedsFromMaskSequence()
    : Processor()
    , sequence_("sequence")
    , seeds_("seeds_")
    , randomSampling_("randomSampling", "Random sampling (keep percentage)", 1.f, 0.f, 1.f, 0.01f) {

    addPort(sequence_);
    addPort(seeds_);
    addProperty(randomSampling_);
}

void SeedsFromMaskSequence::process() {
    auto inSequence = sequence_.getData();
    auto& volumes = *inSequence;
    auto outvec = std::make_shared<SeedPoint4DVector>();
    auto& points = *outvec;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);

    size_t volID = 0;
    for (auto vol : volumes) {
        vol->getRepresentation<VolumeRAM>()->dispatch<void>([&](auto typedVol) -> void {
            float t = 0;
            if (util::hasTimestamp(vol)) {
                t = static_cast<float>(util::getTimestamp(vol));
            } else {
                t = static_cast<float>(volID) / static_cast<float>(volumes.size() - 1);
            }
            auto data = typedVol->getDataTyped();
            auto dim = typedVol->getDimensions();
            vec3 invDim = vec3(1.0f) / vec3(dim);
            util::IndexMapper3D index(dim);
            util::forEachVoxel(*typedVol, [&](const size3_t& pos) {
                if (dis(gen) > randomSampling_.get()) return;
                auto v = util::glm_convert<float>(data[index(pos)]);
                if (v > 0) {
                    points.emplace_back((vec3(pos) + 0.5f) * invDim, t);
                }
            });
            volID++;
        });
    }

    seeds_.setData(outvec);
}

}  // namespace inviwo
