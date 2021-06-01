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
#include <modules/discretedata/ports/datasetport.h>
#include <modules/discretedata/properties/datachannelproperty.h>
#include <modules/discretedata/sampling/celltree.h>
#include <modules/discretedata/interpolation/interpolant.h>
#include <inviwo/core/properties/optionproperty.h>
#include <modules/discretedata/sampling/datasetspatialsampler.h>
#include <fmt/format.h>

namespace inviwo {
namespace discretedata {

/** \docpage{org.inviwo.DataSetToSpatialSampler, Data Set To Spatial Sampler}
 * ![](org.inviwo.DataSetToSpatialSampler.png?classIdentifier=org.inviwo.DataSetToSpatialSampler)
 * Creates a SpatialSampler from a DataSet. The type of sampler should some day be chosen by the
 * user.
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
    // typedef Interpolant<SpatialDims> (*createInterpolantFunc)(
    //     std::shared_ptr<const DataChannel<double, SpatialDims>>);
    DataSetInport dataIn_;
    DataOutport<SpatialSampler<SpatialDims, DataDims, T>> sampler_;
    DataChannelProperty coordinateChannel_, dataChannel_;
    TemplateOptionProperty<InterpolationType> interpolationType_;
};

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
template <unsigned int SpatialDims, unsigned int DataDims, typename T>
const ProcessorInfo DataSetToSpatialSampler<SpatialDims, DataDims, T>::processorInfo_{
    fmt::format("org.inviwo.VolumeToSpatialSampler{}to{}", SpatialDims,
                DataDims),                                                     // Class identifier
    fmt::format("Volume To Spatial Sampler {} to {}", SpatialDims, DataDims),  // Display name
    "Spatial Sampler",                                                         // Category
    CodeState::Experimental,                                                   // Code state
    Tags::None,                                                                // Tags
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
    , coordinateChannel_(
          dataIn_, "coordChannel", "Coordinate Channel",
          [](const std::shared_ptr<const Channel> ch) -> bool {
              return std::dynamic_pointer_cast<const DataChannel<double, SpatialDims>>(ch).get();
          })
    , dataChannel_(dataIn_, "dataChannel", "Data Channel",
                   [](const std::shared_ptr<const Channel> ch) -> bool {
                       return std::dynamic_pointer_cast<const DataChannel<T, DataDims>>(ch).get();
                   })
    , interpolationType_("interpolationType", "Interpolation Type") {
    addPort(dataIn_);
    addPort(sampler_);
    addProperties(coordinateChannel_, dataChannel_);
}

template <unsigned int SpatialDims, unsigned int DataDims, typename T>
void DataSetToSpatialSampler<SpatialDims, DataDims, T>::process() {
    if (dataIn_.hasData()) {
        auto coordsTN = std::dynamic_pointer_cast<const DataChannel<double, SpatialDims>>(
            coordinateChannel_.getCurrentChannel());
        auto dataTN = std::dynamic_pointer_cast<const DataChannel<T, DataDims>>(
            dataChannel_.getCurrentChannel());
        if (!coordsTN || !dataTN) {
            sampler_.setData(nullptr);
            return;
        }
        auto sampler = std::make_shared<const CellTree<SpatialDims>>(dataIn_.getData()->getGrid(),
                                                                     coordsTN, dataTN);
        auto spatialSampler = std::make_shared<DataSetSpatialSampler<SpatialDims, DataDims, T>>(
            sampler, interpolationType_.get(), dataTN);
        sampler_.setData(spatialSampler);
    } else {
        sampler_.setData(nullptr);
    }
}

using DataSetToSpatialSampler2D = DataSetToSpatialSampler<2, 2, double>;
using DataSetToSpatialSampler3D = DataSetToSpatialSampler<3, 3, double>;

}  // namespace discretedata
}  // namespace inviwo

// #include "datasettospatialsampler.inl"
