/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <inviwo/core/datastructures/coordinatetransformer.h>           // for StructuredCoordin...
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume
#include <inviwo/core/datastructures/volume/volumeram.h>                // for VolumeRAM
#include <inviwo/core/datastructures/volume/volumeramprecision.h>       // IWYU pragma: keep
#include <inviwo/core/ports/datainport.h>                               // for DataInport
#include <inviwo/core/ports/inportiterable.h>                           // for InportIterable<>:...
#include <inviwo/core/ports/outportiterable.h>                          // for OutportIterable
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags, Tags::CPU
#include <inviwo/core/properties/boolproperty.h>                        // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>                   // for CompositeProperty
#include <inviwo/core/properties/ordinalproperty.h>                     // for IntProperty, Doub...
#include <inviwo/core/util/glmconvert.h>                                // for glm_convert_norma...
#include <inviwo/core/util/glmutils.h>                                  // for Matrix
#include <inviwo/core/util/glmvec.h>                                    // for vec3, vec4, uvec3
#include <inviwo/core/util/indexmapper.h>                               // for IndexMapper, Inde...
#include <inviwo/core/util/volumeramutils.h>                            // for forEachVoxel
#include <modules/vectorfieldvisualization/ports/seedpointsport.h>      // for SeedPoints3DOutport

#include <functional>     // for __base
#include <memory>         // for shared_ptr, make_...
#include <string_view>    // for string_view
#include <type_traits>    // for remove_extent_t
#include <unordered_set>  // for unordered_set

#include <fmt/core.h>      // for format_to, basic_...
#include <glm/mat4x4.hpp>  // for operator*, mat
#include <glm/vec3.hpp>    // for operator*, operator+
#include <glm/vec4.hpp>    // for operator*, operator+

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo SeedPointsFromMask::processorInfo_{
    "org.inviwo.SeedPointsFromMask",  // Class identifier
    "Seed Points From Mask",          // Display name
    "Seed Points",                    // Category
    CodeState::Stable,                // Code state
    Tags::CPU,                        // Tags
};
const ProcessorInfo& SeedPointsFromMask::getProcessorInfo() const { return processorInfo_; }

SeedPointsFromMask::SeedPointsFromMask()
    : Processor()
    , volumes_("volumes")
    , seedPoints_("seeds")
    , threshold_("threshold", "Threshold", 0.5, 0.0, 1.0, 0.01)
    , enableSuperSample_("enableSuperSample", "Enable Super Sample", false)
    , superSample_("superSample", "Super Sample", 1, 1, 10)

    , randomness_("randomness", "Randomness")
    , useSameSeed_("useSameSeed", "Use same seed", true)
    , seed_("seed", "Seed", 1, 0, 1000)
    , transformToWorld_("transformToWorld", "Transform To World Space", false)
    , mt_()
    , dis_(.0f, 1.f) {
    addPort(volumes_);
    addPort(seedPoints_);

    addProperty(threshold_);
    addProperty(transformToWorld_);

    addProperty(enableSuperSample_);
    addProperty(superSample_);

    addProperty(randomness_);
    randomness_.addProperty(useSameSeed_);
    randomness_.addProperty(seed_);
    useSameSeed_.onChange([&]() { seed_.setVisible(useSameSeed_.get()); });

    superSample_.setVisible(false);
    randomness_.setVisible(false);
    enableSuperSample_.onChange([&]() {
        superSample_.setVisible(enableSuperSample_.get());
        randomness_.setVisible(enableSuperSample_.get());
    });
}

void SeedPointsFromMask::process() {
    if (useSameSeed_.get()) {
        mt_.seed(seed_.get());
    }

    auto points = std::make_shared<std::vector<vec3>>();

    for (const auto& v : volumes_) {
        v->getRepresentation<VolumeRAM>()->dispatch<void>([&](auto volPrecision) {
            auto dim = volPrecision->getDimensions();
            auto data = volPrecision->getDataTyped();
            util::IndexMapper3D index(dim);
            vec3 invDim = vec3(1.0f) / vec3(dim);

            auto transform = [transformToWorld = transformToWorld_.get(),
                              m = v->getCoordinateTransformer().getDataToWorldMatrix()](auto p) {
                if (transformToWorld) {
                    vec4 WP = m * vec4(p, 1.0f);
                    return vec3(WP) / WP.w;
                } else {
                    return p;
                }
            };

            util::forEachVoxel(*volPrecision, [&](const size3_t& pos) {
                if (util::glm_convert_normalized<double>(data[index(pos)]) > threshold_.get()) {
                    if (enableSuperSample_.get()) {
                        for (int j = 0; j < superSample_.get(); j++) {
                            const auto x = dis_(mt_);
                            const auto y = dis_(mt_);
                            const auto z = dis_(mt_);
                            points->push_back(transform((vec3(pos) + vec3{x, y, z}) * invDim));
                        }
                    } else {
                        points->push_back(transform((vec3(pos) + 0.5f) * invDim));
                    }
                }
            });
        });
    }
    seedPoints_.setData(points);
}

}  // namespace inviwo
