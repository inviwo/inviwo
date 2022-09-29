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

#pragma once

#include <modules/base/basemoduledefine.h>         // for IVW_MODULE_BASE_API

#include <inviwo/core/ports/dataoutport.h>         // for DataOutport
#include <inviwo/core/ports/imageport.h>           // for ImageInport
#include <inviwo/core/processors/processor.h>      // for Processor
#include <inviwo/core/processors/processorinfo.h>  // for ProcessorInfo
#include <inviwo/core/util/spatialsampler.h>       // for SpatialSampler

#include <string>                                  // for operator+, string

#include <fmt/core.h>                              // for format
#include <glm/mat3x3.hpp>                          // for operator*
#include <glm/vec2.hpp>                            // for operator/

namespace inviwo {

/** \docpage{org.inviwo.ImageToSpatialSampler, Image To Spatial Sampler}
 * ![](org.inviwo.ImageToSpatialSampler.png?classIdentifier=org.inviwo.ImageToSpatialSampler)
 *
 * Creates a Spatial Sampler for the given input image.
 *
 *
 * ### Inports
 *   * __image__ The input image.
 *
 * ### Outports
 *   * __sampler__ The created sampler.
 *
 */

class IVW_MODULE_BASE_API ImageToSpatialSampler : public Processor {
public:
    ImageToSpatialSampler();
    virtual ~ImageToSpatialSampler() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    ImageInport image_;
    DataOutport<SpatialSampler<2, 2, double>> sampler_;
};

}  // namespace inviwo
