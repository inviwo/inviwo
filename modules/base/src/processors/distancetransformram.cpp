/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <modules/base/processors/distancetransformram.h>
#include <modules/base/algorithm/dataminmax.h>
#include <modules/base/algorithm/volume/volumeramdistancetransform.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>

namespace inviwo {

const ProcessorInfo DistanceTransformRAM::processorInfo_{
    "org.inviwo.DistanceTransformRAM",  // Class identifier
    "Volume Distance Transform",        // Display name
    "Volume Operation",                 // Category
    CodeState::Stable,                  // Code state
    Tags::CPU,                          // Tags
};
const ProcessorInfo DistanceTransformRAM::getProcessorInfo() const { return processorInfo_; }

DistanceTransformRAM::DistanceTransformRAM()
    : PoolProcessor(pool::Option::DelayDispatch)
    , volumePort_("inputVolume")
    , outport_("outputVolume")
    , threshold_("threshold", "Threshold", 0.5, 0.0, 1.0)
    , flip_("flip", "Flip", false)
    , normalize_("normalize", "Use normalized threshold", true)
    , resultDistScale_("distScale", "Scaling Factor", 1.0f, 0.0f, 1.0e3, 0.05f)
    , resultSquaredDist_("distSquared", "Squared Distance", false)
    , uniformUpsampling_("uniformUpsampling", "Uniform Upsampling", false)
    , upsampleFactorUniform_("upsampleFactorUniform", "Sampling Factor", 1, 1, 10)
    , upsampleFactorVec3_("upsampleFactorVec3", "Sampling Factor", size3_t(1), size3_t(1),
                          size3_t(10))
    , dataRangeOutput_("dataRange", "Output Range", 0.0, 1.0, 0.0,
                       std::numeric_limits<double>::max(), 0.01, 0.0, InvalidationLevel::Valid,
                       PropertySemantics::Text)
    , dataRangeMode_("dataRangeMode", "Data Range",
                     {DataRangeMode::Diagonal, DataRangeMode::MinMax, DataRangeMode::Custom}, 0)
    , customDataRange_("customDataRange", "Custom Data Range", 0.0, 1.0, 0.0,
                       std::numeric_limits<double>::max(), 0.01, 0.0,
                       InvalidationLevel::InvalidOutput, PropertySemantics::Text) {

    addPort(volumePort_);
    addPort(outport_);

    addProperties(threshold_, flip_, normalize_, resultDistScale_, resultSquaredDist_,
                  uniformUpsampling_, upsampleFactorVec3_, upsampleFactorUniform_, dataRangeMode_,
                  customDataRange_, dataRangeOutput_);

    upsampleFactorVec3_.visibilityDependsOn(uniformUpsampling_,
                                            [](const auto& p) { return !p.get(); });
    upsampleFactorUniform_.visibilityDependsOn(uniformUpsampling_,
                                               [](const auto& p) { return p.get(); });

    dataRangeOutput_.setSerializationMode(PropertySerializationMode::None);
    dataRangeOutput_.setReadOnly(true);

    customDataRange_.readonlyDependsOn(dataRangeMode_, [](const auto& p) {
        return p.getSelectedValue() != DataRangeMode::Custom;
    });
}

DistanceTransformRAM::~DistanceTransformRAM() = default;

void DistanceTransformRAM::process() {
    auto calc = [upsample = uniformUpsampling_.get() ? size3_t(upsampleFactorUniform_.get())
                                                     : upsampleFactorVec3_.get(),
                 threshold = threshold_.get(), normalize = normalize_.get(), flip = flip_.get(),
                 square = resultSquaredDist_.get(), scale = resultDistScale_.get(),
                 dataRangeMode = dataRangeMode_.get(), customDataRange = customDataRange_.get(),
                 volume =
                     volumePort_.getData()](pool::Progress fprogress) -> std::shared_ptr<Volume> {
        auto volDim = glm::max(volume->getDimensions(), size3_t(1u));
        auto dstRepr = std::make_shared<VolumeRAMPrecision<float>>(upsample * volDim);

        const auto progress = [&](double f) { fprogress(static_cast<float>(f)); };
        util::volumeDistanceTransform(volume.get(), dstRepr.get(), upsample, threshold, normalize,
                                      flip, square, scale, progress);

        auto dstVol = std::make_shared<Volume>(dstRepr);
        // pass meta data on
        dstVol->setModelMatrix(volume->getModelMatrix());
        dstVol->setWorldMatrix(volume->getWorldMatrix());
        dstVol->copyMetaDataFrom(*volume);

        switch (dataRangeMode) {
            case DistanceTransformRAM::DataRangeMode::Diagonal: {
                const auto basis = volume->getBasis();
                const auto diagonal = basis[0] + basis[1] + basis[2];
                const auto maxDist = square ? glm::length2(diagonal) : glm::length(diagonal);
                dstVol->dataMap_.dataRange = dvec2(0.0, maxDist);
                dstVol->dataMap_.valueRange = dvec2(0.0, maxDist);
                break;
            }
            case DistanceTransformRAM::DataRangeMode::MinMax: {
                auto minmax = util::dataMinMax(dstRepr->getDataTyped(),
                                               glm::compMul(dstRepr->getDimensions()));

                dstVol->dataMap_.dataRange = dvec2(minmax.first[0], minmax.second[0]);
                dstVol->dataMap_.valueRange = dvec2(minmax.first[0], minmax.second[0]);
                break;
            }
            case DistanceTransformRAM::DataRangeMode::Custom: {
                dstVol->dataMap_.dataRange = customDataRange;
                dstVol->dataMap_.valueRange = customDataRange;
                break;
            }
            default:
                break;
        }
        return dstVol;
    };

    outport_.setData(nullptr);
    dispatchOne(calc, [this](std::shared_ptr<Volume> result) {
        outport_.setData(result);
        newResults();
    });
}

}  // namespace inviwo
