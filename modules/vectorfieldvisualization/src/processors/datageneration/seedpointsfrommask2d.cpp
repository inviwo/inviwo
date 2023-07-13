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

#include <modules/vectorfieldvisualization/processors/datageneration/seedpointsfrommask2d.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/util/imageramutils.h>
#include <inviwo/core/util/indexmapper.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo SeedPointsFromMask2D::processorInfo_{
    "org.inviwo.SeedPointsFromMask2D",  // Class identifier
    "Seed Points From Mask 2D",         // Display name
    "Seed Points",                      // Category
    CodeState::Stable,                  // Code state
    Tags::CPU,                          // Tags
};
const ProcessorInfo SeedPointsFromMask2D::getProcessorInfo() const { return processorInfo_; }

SeedPointsFromMask2D::SeedPointsFromMask2D()
    : Processor()
    , images_("images")
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
    addPorts(images_, seedPoints_);

    addProperties(threshold_, transformToWorld_, enableSuperSample_, superSample_, randomness_);
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

void SeedPointsFromMask2D::process() {
    if (useSameSeed_.get()) {
        mt_.seed(seed_.get());
    }

    auto points = std::make_shared<std::vector<vec2>>();

    for (const auto& v : images_) {
        auto layer = v->getColorLayer();
        layer->getRepresentation<LayerRAM>()->dispatch<void>([&](auto imgPrecision) {
            auto dim = imgPrecision->getDimensions();
            auto data = imgPrecision->getDataTyped();
            util::IndexMapper2D index(dim);
            vec2 invDim = vec2(1.0f) / vec2(dim);

            auto transform =
                [transformToWorld = transformToWorld_.get(),
                 m = layer->getCoordinateTransformer().getDataToWorldMatrix()](auto p) {
                    if (transformToWorld) {
                        vec3 WP = m * vec3(p, 1.0f);
                        return vec2(WP) / WP.z;
                    } else {
                        return p;
                    }
                };

            util::forEachPixel(*imgPrecision, [&](const size2_t& pos) {
                if (util::glm_convert_normalized<double>(data[index(pos)]) > threshold_.get()) {
                    if (enableSuperSample_.get()) {
                        for (int j = 0; j < superSample_.get(); j++) {
                            const auto x = dis_(mt_);
                            const auto y = dis_(mt_);
                            points->push_back(transform((vec2(pos) + vec2{x, y}) * invDim));
                        }
                    } else {
                        points->push_back(transform((vec2(pos) + 0.5f) * invDim));
                    }
                }
            });
        });
    }
    seedPoints_.setData(points);
}

}  // namespace inviwo
