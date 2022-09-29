/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2022 Inviwo Foundation
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

#include <modules/base/processors/volumetospatialsampler.h>

#include <inviwo/core/ports/dataoutport.h>          // for DataOutport
#include <inviwo/core/ports/outportiterable.h>      // for OutportIterableImpl<>::const_iterator
#include <inviwo/core/ports/volumeport.h>           // for VolumeInport
#include <inviwo/core/processors/processor.h>       // for Processor
#include <inviwo/core/processors/processorinfo.h>   // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>  // for CodeState, CodeState::Experimental
#include <inviwo/core/processors/processortags.h>   // for Tags, Tags::None
#include <inviwo/core/util/glmutils.h>              // for Vector
#include <inviwo/core/util/spatialsampler.h>        // for SpatialSampler
#include <inviwo/core/util/volumesampler.h>         // for VolumeDoubleSampler

#include <functional>                               // for __base
#include <memory>                                   // for make_shared, shared_ptr
#include <string_view>                              // for string_view

#include <fmt/core.h>                               // for format
#include <glm/common.hpp>                           // for mix

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeToSpatialSampler::processorInfo_{
    "org.inviwo.VolumeToSpatialSampler",  // Class identifier
    "Volume To Spatial Sampler",          // Display name
    "Spatial Sampler",                    // Category
    CodeState::Experimental,              // Code state
    Tags::None,                           // Tags
};
const ProcessorInfo VolumeToSpatialSampler::getProcessorInfo() const { return processorInfo_; }

VolumeToSpatialSampler::VolumeToSpatialSampler()
    : Processor(), volume_("volume"), sampler_("sampler") {
    addPort(volume_);
    addPort(sampler_);
}

void VolumeToSpatialSampler::process() {
    auto sampler = std::make_shared<VolumeDoubleSampler<3>>(volume_.getData());
    sampler_.setData(sampler);
}

}  // namespace inviwo
