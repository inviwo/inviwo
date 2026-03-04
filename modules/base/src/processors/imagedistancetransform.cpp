/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2026 Inviwo Foundation
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

#include <modules/base/processors/imagedistancetransform.h>

#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/processors/poolprocessor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/glmvec.h>
#include <modules/base/algorithm/image/layerramdistancetransform.h>
#include <modules/base/datastructures/imagereusecache.h>
#include <modules/base/algorithm/dataminmax.h>

#include <functional>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>

#include <glm/common.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/mat2x2.hpp>
#include <glm/vec2.hpp>

namespace inviwo {

const ProcessorInfo ImageDistanceTransform::processorInfo_{
    "org.inviwo.ImageDistanceTransform",  // Class identifier
    "Image Distance Transform",           // Display name
    "Image Operation",                    // Category
    CodeState::Stable,                    // Code state
    Tags::CPU,                            // Tags
    R"(Computes the closest distance to a threshold value for each texel in the first color layer
    of the input Image. It uses Saito's algorithm to compute the Euclidean distance.
    Note: Only works correctly for Layers with an orthogonal basis.)"_unindentHelp,
};
const ProcessorInfo& ImageDistanceTransform::getProcessorInfo() const { return processorInfo_; }

ImageDistanceTransform::ImageDistanceTransform()
    : PoolProcessor()
    , imagePort_("inputImage", "Input image"_help)
    , outport_("outputImage", "Scalar image representing the distance transform (float)"_help,
               DataVec4UInt8::get(), HandleResizeEvents::No)
    , threshold_(
          "threshold", "Threshold",
          "Pixels with a value  __larger___ than the threshold will be considered as features, i.e. have a zero distance."_help,
          0.5, {0.0, ConstraintBehavior::Ignore}, {1.0, ConstraintBehavior::Ignore})
    , flip_("flip", "Flip",
            "Consider features as pixels with a values __smaller__ than threshold instead."_help,
            false)
    , normalize_("normalize", "Use normalized threshold",
                 "Use normalized values when comparing to the threshold."_help, true)
    , resultDistScale_("distScale", "Scaling Factor",
                       util::ordinalScale(1.0, 1000.0)
                           .set("Scaling factor to apply to the output distance field."_help))
    , resultSquaredDist_("distSquared", "Squared Distance",
                         "Output the squared distance field"_help, false)
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

ImageDistanceTransform::~ImageDistanceTransform() = default;

void ImageDistanceTransform::process() {
    const auto image = imagePort_.getData();
    const auto upsample = uniformUpsampling_.get() ? size3_t(upsampleFactorUniform_.get())
                                                   : upsampleFactorVec2_.get();
    const auto imgDim = glm::max(image->getDimensions(), size2_t(1u));

    auto [dstImage, dstRepr] = imageCache_.getTypedUnused<float>(upsample * imgDim);
    const auto calc =
        [image, upsample, dstImage, dstRepr, threshold = threshold_.get(),
         normalize = normalize_.get(), flip = flip_.get(), square = resultSquaredDist_.get(),
         scale = resultDistScale_.get()](pool::Progress progress) -> std::shared_ptr<Image> {
        // pass meta data on
        dstImage->getColorLayer()->setModelMatrix(image->getColorLayer()->getModelMatrix());
        dstImage->getColorLayer()->setWorldMatrix(image->getColorLayer()->getWorldMatrix());
        dstImage->copyMetaDataFrom(*image);

        util::layerDistanceTransform(image->getColorLayer(), dstRepr, upsample, threshold,
                                     normalize, flip, square, scale, progress);

        auto max = glm::compMax(util::layerMinMax(dstRepr, IgnoreSpecialValues::Yes).second);
        dstImage->getColorLayer()->dataMap.dataRange = dvec2{0.0, max};
        dstImage->getColorLayer()->dataMap.valueRange = dvec2{0.0, max};

        return dstImage;
    };

    outport_.clear();
    dispatchOne(calc, [this](std::shared_ptr<Image> result) {
        imageCache_.add(result);
        outport_.setData(result);
        newResults();
    });
}

}  // namespace inviwo
