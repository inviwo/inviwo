/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#pragma once

#include <modules/discretedata/discretedatamoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/util/stringconversion.h>
#include <modules/discretedata/ports/datasetport.h>
#include <modules/discretedata/properties/datachannelproperty.h>
#include <modules/discretedata/sampling/celltree.h>
#include <modules/discretedata/sampling/interpolant.h>
#include <modules/discretedata/sampling/datasetspatialsampler.h>
#include <fmt/format.h>

namespace inviwo {
namespace discretedata {

/** \docpage{org.inviwo.DataSetToSpatialSampler, Data Set To Spatial Sampler}
 * ![](org.inviwo.DataSetToSpatialSampler.png?classIdentifier=org.inviwo.DataSetToSpatialSampler)
 * Creates a SpatialSampler from a DatasetSampler already created for a DataSet.
 * This processor basically just combines the weighted sampler with a Channel to sampler from.
 * This allows several SpatialSamplers to use the same data structure, for example on velocity and
 * pressure in the same DataSet.
 */
template <unsigned int SpatialDims, unsigned int DataDims, typename T>
class IVW_MODULE_DISCRETEDATA_API DataSetToSpatialSampler : public Processor {
public:
    DataSetToSpatialSampler();
    virtual ~DataSetToSpatialSampler() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    void fillInterpolationTypes(const DataSetSamplerBase& sampler);
    DataSetInport dataIn_;
    DataOutport<SpatialSampler<SpatialDims, DataDims, T>> sampler_;

    OptionPropertyString datasetSamplerName_;
    TemplateOptionProperty<InterpolationType> interpolationType_;
    DataChannelProperty dataChannel_;

    bool firstProcess_ = true;
    // bool interpolationChanged_;
};

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
template <unsigned int SpatialDims, unsigned int DataDims, typename T>
const ProcessorInfo DataSetToSpatialSampler<SpatialDims, DataDims, T>::processorInfo_{
    fmt::format("org.inviwo.DataSetToSpatialSampler{}to{}", SpatialDims,
                DataDims),                                                      // Class identifier
    fmt::format("DataSet To Spatial Sampler {} to {}", SpatialDims, DataDims),  // Display name
    "Spatial Sampler",                                                          // Category
    CodeState::Experimental,                                                    // Code state
    Tags::None,                                                                 // Tags
};

template <unsigned int SpatialDims, unsigned int DataDims, typename T>
const ProcessorInfo DataSetToSpatialSampler<SpatialDims, DataDims, T>::getProcessorInfo() const {
    return processorInfo_;
}

template <unsigned int SpatialDims, unsigned int DataDims, typename T>
DataSetToSpatialSampler<SpatialDims, DataDims, T>::DataSetToSpatialSampler()
    : Processor()
    , dataIn_("dataset")
    , sampler_("sampler")
    , datasetSamplerName_("datasetSampler", "DataSet Sampler")
    , interpolationType_("interpolationType", "Interpolation Type",
                         {{"nearest", "Nearest Neighbor", InterpolationType::Nearest},
                          {"squared", "Squared Distance", InterpolationType::SquaredDistance},
                          {"linear", "Linear", InterpolationType::Linear}},
                         2)
    , dataChannel_("dataChannel", "Data Channel", &dataIn_,
                   [](const std::shared_ptr<const Channel> ch) -> bool {
                       //    return std::dynamic_pointer_cast<const DataChannel<T,
                       //    DataDims>>(ch).get();
                       return ch->getNumComponents() == DataDims;
                   })
// , interpolationChanged_(false)
{
    addPort(dataIn_);
    addPort(sampler_);
    addProperties(datasetSamplerName_, dataChannel_, interpolationType_);
    interpolationType_.setCurrentStateAsDefault();
    interpolationType_.onChange([this]() {
        // interpolationChanged_ = true;
        invalidate(InvalidationLevel::InvalidOutput);
    });
}

template <unsigned int SpatialDims, unsigned int DataDims, typename T>
void DataSetToSpatialSampler<SpatialDims, DataDims, T>::process() {
    if (!firstProcess_ && !dataIn_.isChanged() && !dataIn_.hasData()) return;

    // No samplers in the dataset??
    if (dataIn_.isChanged() &&
        (!dataIn_.hasData() || dataIn_.getData()->getSamplers().size() == 0)) {
        firstProcess_ = false;
        datasetSamplerName_.clearOptions();
        sampler_.setData(nullptr);
        return;
    }

    const auto& samplerMap = dataIn_.getData()->getSamplers();

    // Refresh the sampler options.
    if ((firstProcess_ || dataIn_.isChanged()) && dataIn_.hasData()) {

        std::string selected =
            datasetSamplerName_.size() ? datasetSamplerName_.getSelectedIdentifier() : "";

        datasetSamplerName_.clearOptions();
        for (auto& sampler : samplerMap) {
            datasetSamplerName_.addOption(sampler.first, sampler.first);
            // removeFromString(sampler.first, ' '), sampler.first);
        }
        datasetSamplerName_.setSelectedValue(selected);
    }
    firstProcess_ = false;

    // Something missing to create a SpatialSampler?
    if (!datasetSamplerName_.size()) {
        LogError("No sampler available.");
        sampler_.setData(nullptr);
        return;
    }

    // Actually build a SpatialSampler.
    auto dataTN = dataChannel_.getCurrentChannelTyped<T, DataDims>();
    if (!dataTN) {
        LogError("The given data channel is not of the expected type.");
        sampler_.setData(nullptr);
        return;
    }

    auto samplerIt = samplerMap.find(datasetSamplerName_.getSelectedValue());
    if (samplerIt == samplerMap.end()) {
        return;
    }

    InterpolationType interpolation = InterpolationType::Nearest;
    if (interpolationType_.size() &&
        interpolationType_.getSelectedIndex() < interpolationType_.size()) {
        interpolation = interpolationType_.get();
        LogWarn("Interpolation found!");
    }

    auto datasetSampler =
        std::dynamic_pointer_cast<const DataSetSampler<SpatialDims>>(samplerIt->second);

    auto spatialSampler = std::make_shared<DataSetSpatialSampler<SpatialDims, DataDims, T>>(
        datasetSampler, interpolation, dataTN);

    // Replace interpolation type options
    fillInterpolationTypes(*(samplerIt->second));
    spatialSampler->interpolationType_ = interpolation;
    sampler_.setData(spatialSampler);
}

template <unsigned int SpatialDims, unsigned int DataDims, typename T>
void DataSetToSpatialSampler<SpatialDims, DataDims, T>::fillInterpolationTypes(
    const DataSetSamplerBase& sampler) {
    if (interpolationType_.size() < 3)
        interpolationType_.replaceOptions(
            {{"nearest", "Nearest Neighbor", InterpolationType::Nearest},
             {"squared", "Squared Distance", InterpolationType::SquaredDistance},
             {"linear", "Linear", InterpolationType::Linear}});

    // Remove interpolation options that are not supported by the selected interpolant.
    for (auto& opt : interpolationType_.getOptions()) {
        if (!sampler.getInterpolantBase().supportsInterpolationType(opt.value_))
            interpolationType_.removeOption(opt.id_);
    }
}

using DataSetToSpatialSampler2D = DataSetToSpatialSampler<2, 2, double>;
using DataSetToSpatialSampler3D = DataSetToSpatialSampler<3, 3, double>;

}  // namespace discretedata
}  // namespace inviwo

// #include "datasettospatialsampler.inl"
