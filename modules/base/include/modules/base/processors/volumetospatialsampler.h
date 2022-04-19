/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2021 Inviwo Foundation
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

#ifndef IVW_VOLUMETOSPATIALSAMPLER_H
#define IVW_VOLUMETOSPATIALSAMPLER_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/util/spatialsampler.h>
#include <inviwo/core/util/volumesampler.h>

namespace inviwo {

/** \docpage{org.inviwo.VolumeToSpatialSampler, Volume To Spatial Sampler}
 * ![](org.inviwo.VolumeToSpatialSampler.png?classIdentifier=org.inviwo.VolumeToSpatialSampler)
 * Create a SpatialSampler from a volume, for example for stream line integration.
 */
template <unsigned int DataDims>
class IVW_MODULE_BASE_API VolumeToSpatialSampler : public Processor {
public:
    VolumeToSpatialSampler();
    virtual ~VolumeToSpatialSampler() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeInport volume_;
    DataOutport<SpatialSampler<3, DataDims, double>> sampler_;
};

template <unsigned int DataDims>
const ProcessorInfo VolumeToSpatialSampler<DataDims>::processorInfo_{
    fmt::format("org.inviwo.VolumeToSpatialSampler{}", DataDims),  // Class identifier
    fmt::format("Volume To Spatial Sampler (dvec{})", DataDims),   // Display name
    "Spatial Sampler",                                             // Category
    CodeState::Experimental,                                       // Code state
    Tags::None,                                                    // Tags
};

template <unsigned int DataDims>
const ProcessorInfo VolumeToSpatialSampler<DataDims>::getProcessorInfo() const {
    return processorInfo_;
}

template <unsigned int DataDims>
VolumeToSpatialSampler<DataDims>::VolumeToSpatialSampler()
    : Processor(), volume_("volume"), sampler_("sampler") {
    addPort(volume_);
    addPort(sampler_);
}

template <unsigned int DataDims>
void VolumeToSpatialSampler<DataDims>::process() {
    auto sampler = std::make_shared<VolumeDoubleSampler<DataDims>>(volume_.getData());
    sampler_.setData(sampler);
}

using VolumeToSpatialSampler1D = VolumeToSpatialSampler<1>;
using VolumeToSpatialSampler2D = VolumeToSpatialSampler<2>;
using VolumeToSpatialSampler3D = VolumeToSpatialSampler<3>;  // Most useful for integration.
using VolumeToSpatialSampler4D = VolumeToSpatialSampler<4>;

}  // namespace inviwo

#endif  // IVW_VOLUMETOSPATIALSAMPLER_H
