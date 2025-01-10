/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2025 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/ports/imageport.h>                  // for ImageInport, ImageOutport
#include <inviwo/core/processors/poolprocessor.h>         // for PoolProcessor
#include <inviwo/core/processors/processorinfo.h>         // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>          // for BoolProperty
#include <inviwo/core/properties/ordinalproperty.h>       // for DoubleProperty, IntProperty
#include <modules/base/datastructures/imagereusecache.h>  // for ImageReuseCache

namespace inviwo {

class IVW_MODULE_BASE_API ImageDistanceTransform : public PoolProcessor {
public:
    ImageDistanceTransform();
    virtual ~ImageDistanceTransform();

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    ImageInport imagePort_;
    ImageOutport outport_;

    ImageReuseCache imageCache_;

    DoubleProperty threshold_;
    BoolProperty flip_;
    BoolProperty normalize_;
    DoubleProperty resultDistScale_;  // scaling factor for distances
    BoolProperty resultSquaredDist_;  // determines whether output uses squared euclidean distances
    BoolProperty uniformUpsampling_;
    IntProperty upsampleFactorUniform_;    // uniform upscaling of the output field
    IntSize2Property upsampleFactorVec2_;  // non-uniform upscaling of the output field
};

}  // namespace inviwo
