/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <modules/base/processors/layerdistancetransformram.h>

#include <modules/base/algorithm/dataminmax.h>
#include <modules/base/algorithm/image/layerramdistancetransform.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>

namespace inviwo {

const ProcessorInfo LayerDistanceTransformRAM::processorInfo_{
    "org.inviwo.LayerDistanceTransformRAM",  // Class identifier
    "Image Distance Transform",              // Display name
    "Image Operation",                       // Category
    CodeState::Stable,                       // Code state
    Tags::CPU,                               // Tags
};
const ProcessorInfo LayerDistanceTransformRAM::getProcessorInfo() const { return processorInfo_; }

LayerDistanceTransformRAM::LayerDistanceTransformRAM()
    : PoolProcessor()
    , imagePort_("inputImage")
    , outport_("outputImage", DataVec4UInt8::get(), false)
    , threshold_("threshold", "Threshold", 0.5, 0.0, 1.0)
    , flip_("flip", "Flip", false)
    , normalize_("normalize", "Use normalized threshold", true)
    , resultDistScale_("distScale", "Scaling Factor", 1.0f, 0.0f, 1.0e3, 0.05f)
    , resultSquaredDist_("distSquared", "Squared Distance", false)
    , uniformUpsampling_("uniformUpsampling", "Uniform Upsampling", false)
    , upsampleFactorUniform_("upsampleFactorUniform", "Sampling Factor", 1, 1, 10)
    , upsampleFactorVec2_("upsampleFactorVec2", "Sampling Factor", size2_t(1), size2_t(1),
                          size2_t(10)) {

    addPort(imagePort_);
    addPort(outport_);

    addProperties(threshold_, flip_, normalize_, resultDistScale_, resultSquaredDist_,
                  uniformUpsampling_, upsampleFactorVec2_, upsampleFactorUniform_);

    upsampleFactorVec2_.visibilityDependsOn(uniformUpsampling_,
                                            [](const auto& p) { return !p.get(); });
    upsampleFactorUniform_.visibilityDependsOn(uniformUpsampling_,
                                               [](const auto& p) { return p.get(); });
}

LayerDistanceTransformRAM::~LayerDistanceTransformRAM() = default;

void LayerDistanceTransformRAM::process() {
    const auto calc = [image = imagePort_.getData(),
                       upsample = uniformUpsampling_.get() ? size3_t(upsampleFactorUniform_.get())
                                                           : upsampleFactorVec2_.get(),
                       threshold = threshold_.get(), normalize = normalize_.get(),
                       flip = flip_.get(), square = resultSquaredDist_.get(),
                       scale = resultDistScale_.get(),
                       &cache = imageCache_](pool::Progress progress) -> std::shared_ptr<Image> {
        auto imgDim = glm::max(image->getDimensions(), size2_t(1u));

        auto [dstImage, dstRepr] = cache.getTypedUnused<float>(upsample * imgDim);

        // pass meta data on
        dstImage->getColorLayer()->setModelMatrix(image->getColorLayer()->getModelMatrix());
        dstImage->getColorLayer()->setWorldMatrix(image->getColorLayer()->getWorldMatrix());
        dstImage->copyMetaDataFrom(*image);

        util::layerDistanceTransform(image->getColorLayer(), dstRepr, upsample, threshold,
                                     normalize, flip, square, scale, progress);

        cache.add(dstImage);
        return dstImage;
    };

    outport_.clear();
    dispatchOne(calc, [this](std::shared_ptr<Image> result) {
        outport_.setData(result);
        newResults();
    });
}

}  // namespace inviwo
