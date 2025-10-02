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

#pragma once

#include <modules/opactopt/opactoptmoduledefine.h>

#include <modules/opactopt/rendering/approximation.h>
#include <modules/oit/processors/rasterizationrenderer.h>  // for RasterizationRenderer

#include <inviwo/core/interaction/cameratrackball.h>  // for CameraTrackball
#include <inviwo/core/processors/processor.h>         // for Processor
#include <modules/opengl/shader/shader.h>             // for Shader
#include <modules/opengl/texture/texture2d.h>         // for Texture2D
#include <inviwo/core/ports/imageport.h>              // for ImageInport, ImageOu...
#include <inviwo/core/properties/cameraproperty.h>    // for CameraProperty
#include <inviwo/core/properties/optionproperty.h>    // for OptionProperty
#include <inviwo/core/properties/simplelightingproperty.h>
#include <modules/oit/ports/rasterizationport.h>  // for RasterizationInport

namespace inviwo {

class IVW_MODULE_OPACTOPT_API OpacityOptimiser : public RasterizationRenderer {
public:
    OpacityOptimiser();
    virtual const ProcessorInfo getProcessorInfo() const override;
    virtual void process() override;

    static const ProcessorInfo processorInfo_;

private:
    FloatProperty q_, r_, lambda_;
    OptionPropertyInt opacityOptimisationRenderer_;
    CompositeProperty approximationProperties_;
    OptionProperty<std::string> approximationMethod_;
    IntProperty approximationCoefficients_;
    BoolCompositeProperty smoothing_;
    IntProperty gaussianKernelRadius_;
    FloatProperty gaussianKernelSigma_;    
};

}  // namespace inviwo
