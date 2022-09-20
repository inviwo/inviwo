/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2022 Inviwo Foundation
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

#include <inviwo/core/datastructures/image/image.h>                  // for Image
#include <inviwo/core/datastructures/image/layer.h>                  // for Layer
#include <inviwo/core/datastructures/image/layerram.h>               // for LayerRAMPrecision
#include <inviwo/core/ports/imageport.h>                             // for ImageOutport, ImageI...
#include <inviwo/core/processors/poolprocessor.h>                    // for Progress, PoolProcessor
#include <inviwo/core/processors/processorinfo.h>                    // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                   // for CodeState, CodeState...
#include <inviwo/core/processors/processortags.h>                    // for Tags, Tags::CPU
#include <inviwo/core/properties/boolproperty.h>                     // for BoolProperty
#include <inviwo/core/properties/ordinalproperty.h>                  // for IntProperty, IntSize...
#include <inviwo/core/util/formats.h>                                // for DataFormat, DataVec4...
#include <inviwo/core/util/glmvec.h>                                 // for size2_t, size3_t
#include <modules/base/algorithm/image/layerramdistancetransform.h>  // for layerDistanceTransform
#include <modules/base/datastructures/imagereusecache.h>             // for ImageReuseCache

#include <functional>   // for __base
#include <memory>       // for shared_ptr, shared_p...
#include <ostream>      // for operator<<
#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t

#include <glm/common.hpp>              // for max
#include <glm/gtx/component_wise.hpp>  // for compMax, compMul
#include <glm/mat2x2.hpp>              // for operator*
#include <glm/vec2.hpp>                // for operator/, vec, oper...

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

std::ostream& operator<<(std::ostream& ss, LayerDistanceTransformRAM::DataRangeMode m) {
    switch (m) {
        case LayerDistanceTransformRAM::DataRangeMode::Diagonal:
            ss << "Diagonal";
            break;
        case LayerDistanceTransformRAM::DataRangeMode::MinMax:
            ss << "MinMax";
            break;
        case LayerDistanceTransformRAM::DataRangeMode::Custom:
            ss << "Custom";
            break;
        default:
            break;
    }
    return ss;
}

}  // namespace inviwo
