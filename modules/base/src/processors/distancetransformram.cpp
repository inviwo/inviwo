/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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

#include <inviwo/core/algorithm/markdown.h>                            // for operator""_help
#include <inviwo/core/datastructures/data.h>                           // for noData
#include <inviwo/core/datastructures/datamapper.h>                     // for DataMapper
#include <inviwo/core/datastructures/unitsystem.h>                     // for Axis, Unit
#include <inviwo/core/datastructures/volume/volume.h>                  // for Volume
#include <inviwo/core/datastructures/volume/volumeram.h>               // for VolumeRAMPrecision
#include <inviwo/core/ports/volumeport.h>                              // for VolumeOutport, Vol...
#include <inviwo/core/processors/poolprocessor.h>                      // for Progress, Option
#include <inviwo/core/processors/processorinfo.h>                      // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                     // for CodeState, CodeSta...
#include <inviwo/core/processors/processortags.h>                      // for Tags, Tags::CPU
#include <inviwo/core/properties/boolproperty.h>                       // for BoolProperty
#include <inviwo/core/properties/constraintbehavior.h>                 // for ConstraintBehavior
#include <inviwo/core/properties/invalidationlevel.h>                  // for InvalidationLevel
#include <inviwo/core/properties/minmaxproperty.h>                     // for DoubleMinMaxProperty
#include <inviwo/core/properties/optionproperty.h>                     // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>                    // for IntProperty, IntSi...
#include <inviwo/core/properties/propertysemantics.h>                  // for PropertySemantics
#include <inviwo/core/properties/valuewrapper.h>                       // for PropertySerializat...
#include <inviwo/core/util/glmvec.h>                                   // for dvec2, size3_t
#include <inviwo/core/util/staticstring.h>                             // for operator+
#include <modules/base/algorithm/dataminmax.h>                         // for dataMinMax
#include <modules/base/algorithm/volume/volumeramdistancetransform.h>  // for volumeDistanceTran...

#include <array>        // for array
#include <limits>       // for numeric_limits
#include <memory>       // for shared_ptr, make_s...
#include <ostream>      // for operator<<
#include <type_traits>  // for remove_extent_t
#include <utility>      // for pair

#include <glm/common.hpp>              // for max, min
#include <glm/geometric.hpp>           // for dot, length
#include <glm/gtx/component_wise.hpp>  // for compMax, compMul
#include <glm/gtx/norm.hpp>            // for length2
#include <glm/mat3x3.hpp>              // for operator*, mat
#include <glm/vec3.hpp>                // for operator/, operator*
#include <glm/vec4.hpp>                // for vec

namespace inviwo {

const ProcessorInfo DistanceTransformRAM::processorInfo_{
    "org.inviwo.DistanceTransformRAM",  // Class identifier
    "Volume Distance Transform",        // Display name
    "Volume Operation",                 // Category
    CodeState::Stable,                  // Code state
    Tags::CPU,                          // Tags
    R"(Computes the distance transform of a volume dataset using a threshold value
    The result is the distance from each voxel to the closest feature. It will only work correctly
    for volumes with an orthogonal basis. It uses the Saito's algorithm to compute the Euclidean
    distance.
    
    Example Network:
    [basegl/distance_transform.inv](file:~modulePath~/tests/regression/distance_transform.inv)
    )"_unindentHelp};
const ProcessorInfo DistanceTransformRAM::getProcessorInfo() const { return processorInfo_; }

DistanceTransformRAM::DistanceTransformRAM()
    : PoolProcessor(pool::Option::DelayDispatch)
    , volumePort_("inputVolume", "Input volume"_help)
    , outport_("outputVolume", "Scalar volume representing the distance transform (float)"_help)
    , threshold_("threshold", "Threshold",
                 "Voxels with a value  __larger___ than the threshold will be considered "
                 "as features, i.e. have a zero distance"_help,
                 0.5, {0.0, ConstraintBehavior::Ignore}, {1.0, ConstraintBehavior::Ignore})
    , flip_("flip", "Flip",
            "Consider features as voxels with a values __smaller__ than threshold instead"_help,
            false)
    , normalize_("normalize", "Use normalized threshold",
                 "Use normalized values when comparing to the threshold"_help, true)
    , resultDistScale_("distScale", "Scaling Factor",
                       "Scaling factor to apply to the output distance field."_help, 1.0f,
                       {0.0f, ConstraintBehavior::Ignore}, {1.0e3, ConstraintBehavior::Ignore},
                       0.05f)
    , resultSquaredDist_("distSquared", "Squared Distance",
                         "Output the squared distance field"_help, false)
    , uniformUpsampling_("uniformUpsampling", "Uniform Upsampling",
                         "Make the output volume have a higher resolution."_help, false)
    , upsampleFactorUniform_("upsampleFactorUniform", "Sampling Factor", 1, 1, 10)
    , upsampleFactorVec3_("upsampleFactorVec3", "Sampling Factor", size3_t(1), size3_t(1),
                          size3_t(10))
    , dataRangeOutput_("dataRange", "Output Range",
                       "The data range of the output volume. (ReadOnly)"_help, 0.0, 1.0, 0.0,
                       std::numeric_limits<double>::max(), 0.01, 0.0, InvalidationLevel::Valid,
                       PropertySemantics::Text)
    , dataRangeMode_("dataRangeMode", "Data Range", R"(Data range to use for the output volume:
        * __Diagonal__ use [0, volume diagonal].
        * __MinMax__ use the minimal and maximal distance from the result
        * __Custom__ specify a custom range.)"_unindentHelp,
                     {DataRangeMode::Diagonal, DataRangeMode::MinMax, DataRangeMode::Custom}, 0)
    , customDataRange_("customDataRange", "Custom Data Range", "Specify a custom output range"_help,
                       0.0, 1.0, 0.0, std::numeric_limits<double>::max(), 0.01, 0.0,
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

        auto dstVol = std::make_shared<Volume>(*volume, noData);
        dstVol->addRepresentation(dstRepr);
        dstVol->dataMap.valueAxis.name = "distance";
        dstVol->dataMap.valueAxis.unit = volume->axes[0].unit;

        switch (dataRangeMode) {
            case DistanceTransformRAM::DataRangeMode::Diagonal: {
                const auto basis = volume->getBasis();
                const auto diagonal = basis[0] + basis[1] + basis[2];
                const auto maxDist = square ? glm::length2(diagonal) : glm::length(diagonal);
                dstVol->dataMap.dataRange = dvec2(0.0, maxDist);
                dstVol->dataMap.valueRange = dvec2(0.0, maxDist);
                break;
            }
            case DistanceTransformRAM::DataRangeMode::MinMax: {
                auto minmax = util::dataMinMax(dstRepr->getDataTyped(),
                                               glm::compMul(dstRepr->getDimensions()));

                dstVol->dataMap.dataRange = dvec2(minmax.first[0], minmax.second[0]);
                dstVol->dataMap.valueRange = dvec2(minmax.first[0], minmax.second[0]);
                break;
            }
            case DistanceTransformRAM::DataRangeMode::Custom: {
                dstVol->dataMap.dataRange = customDataRange;
                dstVol->dataMap.valueRange = customDataRange;
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

std::ostream& operator<<(std::ostream& ss, DistanceTransformRAM::DataRangeMode m) {
    switch (m) {
        case DistanceTransformRAM::DataRangeMode::Diagonal:
            ss << "Diagonal";
            break;
        case DistanceTransformRAM::DataRangeMode::MinMax:
            ss << "MinMax";
            break;
        case DistanceTransformRAM::DataRangeMode::Custom:
            ss << "Custom";
            break;
        default:
            break;
    }
    return ss;
}

}  // namespace inviwo
