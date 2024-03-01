/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/base/processors/layerdistancetransform.h>

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/util/formats.h>
#include <modules/base/algorithm/image/layerramdistancetransform.h>
#include <modules/base/algorithm/dataminmax.h>

#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/glmcomp.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LayerDistanceTransform::processorInfo_{
    "org.inviwo.LayerDistanceTransform",  // Class identifier
    "Layer Distance Transform",           // Display name
    "Layer Operation",                    // Category
    CodeState::Stable,                    // Code state
    Tags::CPU,                            // Tags
    R"(Computes the distance transform of a Layer using a threshold value
    The result is the distance from each pixel to the closest feature. It will only work
    correctly for layers with a orthogonal basis. It uses the Saito's algorithm to compute
    the Euclidean distance.)"_unindentHelp,
};

const ProcessorInfo LayerDistanceTransform::getProcessorInfo() const { return processorInfo_; }

LayerDistanceTransform::LayerDistanceTransform()
    : PoolProcessor()
    , inport_("input", "Input layer"_help)
    , outport_("output", "Scalar Layer representing the distance transform (float)"_help)
    , threshold_(
          "threshold", "Threshold",
          "Pixels with a value  __larger___ then the then the threshold will be considered as features, i.e. have a zero distance."_help,
          0.5, {0.0, ConstraintBehavior::Ignore}, {1.0, ConstraintBehavior::Ignore})
    , flip_("flip", "Flip",
            "Consider features as pixels with a values __smaller__ then threshold instead."_help,
            false)
    , normalize_("normalize", "Use normalized threshold",
                 "Use normalized values when comparing to the threshold."_help, true)
    , resultDistScale_("distScale", "Scaling Factor",
                       util::ordinalScale(1.0, 1000.0)
                           .set("Scaling factor to apply to the output distance field."_help))
    , resultSquaredDist_("distSquared", "Squared Distance",
                         "Output the squared distance field"_help, false)
    , upsampling_("uniformUpsampling", "Uniform Upsampling",
                  {{"none", "None", Upsampling::None},
                   {"uniform", "Uniform", Upsampling::Uniform},
                   {"custom", "Custom", Upsampling::Custom}})
    , upsampleFactorUniform_("upsampleFactorUniform", "Uniform Upsampling", 1, 1, 10)
    , upsampleFactorVec2_("upsampleFactorVec2", "Custom Upsampling", size2_t(1), size2_t(1),
                          size2_t(10)) {

    addPort(inport_);
    addPort(outport_);

    addProperties(threshold_, flip_, normalize_, resultDistScale_, resultSquaredDist_, upsampling_,
                  upsampleFactorUniform_, upsampleFactorVec2_);

    upsampleFactorVec2_.readonlyDependsOn(
        upsampling_, [](const auto& p) { return p.get() != Upsampling::Custom; });
    upsampleFactorUniform_.readonlyDependsOn(
        upsampling_, [](const auto& p) { return p.get() != Upsampling::Uniform; });
}

void LayerDistanceTransform::process() {
    const size2_t upsample = [&]() -> size2_t {
        switch (upsampling_) {
            case Upsampling::Uniform:
                return size2_t{static_cast<size_t>(upsampleFactorUniform_.get())};
            case Upsampling::Custom:
                return upsampleFactorVec2_.get();
            case Upsampling::None:
            default:
                return {1, 1};
        }
    }();

    const auto calc =
        [srcLayer = inport_.getData(), upsample = upsample, threshold = threshold_.get(),
         normalize = normalize_.get(), flip = flip_.get(), square = resultSquaredDist_.get(),
         scale = resultDistScale_.get()](pool::Progress progress) -> std::shared_ptr<Layer> {
        auto dim = glm::max(srcLayer->getDimensions(), size2_t(1u));

        auto layer = std::make_shared<Layer>(
            srcLayer->config().updateFrom({.dimensions = upsample * dim,
                                           .format = DataFormatBase::get(DataFormatId::Float32),
                                           .swizzleMask = swizzlemasks::defaultData(1)}));
        auto destRam =
            static_cast<LayerRAMPrecision<float>*>(layer->getEditableRepresentation<LayerRAM>());

        util::layerDistanceTransform(srcLayer.get(), destRam, upsample, threshold, normalize, flip,
                                     square, scale, progress);
        auto max = glm::compMax(util::layerMinMax(layer.get(), IgnoreSpecialValues::Yes).second);
        layer->dataMap.dataRange = dvec2{0.0, max};
        layer->dataMap.valueRange = dvec2{0.0, max};

        return layer;
    };

    outport_.clear();
    dispatchOne(calc, [this](std::shared_ptr<Layer> result) {
        outport_.setData(result);
        newResults();
    });
}

}  // namespace inviwo
