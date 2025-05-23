/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

#include <modules/basegl/processors/imageprocessing/imageresample.h>

#include <inviwo/core/ports/imageport.h>                                 // for ImageOutport
#include <inviwo/core/processors/processorinfo.h>                        // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                       // for CodeState, CodeS...
#include <inviwo/core/processors/processortags.h>                        // for Tags, Tags::GL
#include <inviwo/core/properties/optionproperty.h>                       // for OptionPropertyInt
#include <inviwo/core/properties/ordinalproperty.h>                      // for IntVec2Property
#include <inviwo/core/util/glmvec.h>                                     // for ivec2
#include <modules/basegl/processors/imageprocessing/imageglprocessor.h>  // for ImageGLProcessor
#include <modules/opengl/shader/shader.h>                                // for Shader
#include <modules/opengl/shader/shaderobject.h>                          // for ShaderObject

#include <functional>   // for __base
#include <string>       // for string
#include <string_view>  // for string_view

#include <glm/vec2.hpp>  // for operator!=, vec

namespace inviwo {

const ProcessorInfo ImageResample::processorInfo_{
    "org.inviwo.ImageResample",  // Class identifier
    "Image Resample",            // Display name
    "Image Operation",           // Category
    CodeState::Stable,           // Code state
    Tags::GL,                    // Tags
    R"(Resamples the input image, which corresponds to upscaling or 
    downscaling to the respective target resolution.)"_unindentHelp,
};
const ProcessorInfo& ImageResample::getProcessorInfo() const { return processorInfo_; }

ImageResample::ImageResample()
    : ImageGLProcessor("img_resample.frag")
    , interpolationType_("interpolationType", "Interpolation Type",
                         "Determines the interpolation for resampling (bilinear or bicubic)"_help,
                         {{"bilinear", "Bilinear", 0}, {"bicubic", "Bicubic", 1}}, 0)
    , outputSizeMode_("outputSizeMode", "Output Size Mode",
                      "Determines the size of the resampled image (set by inport, resize"_help,
                      {{"inportDimension", "Inport Dimensions", 0},
                       {"resizeEvents", "Resize Events", 1},
                       {"custom", "Custom Dimensions", 2}},
                      0)
    , targetResolution_("targetResolution", "Target Resolution", ivec2(256, 256), ivec2(32, 32),
                        ivec2(4096, 4096), ivec2(1, 1)) {

    interpolationType_.onChange([this]() { interpolationTypeChanged(); });
    outputSizeMode_.onChange([this]() { dimensionSourceChanged(); });
    targetResolution_.onChange([this]() { dimensionChanged(); });

    addProperties(interpolationType_, outputSizeMode_, targetResolution_);
}

ImageResample::~ImageResample() = default;

void ImageResample::initializeResources() {
    interpolationTypeChanged();
    dimensionSourceChanged();
    ImageGLProcessor::initializeResources();
}

void ImageResample::interpolationTypeChanged() {
    switch (interpolationType_.get()) {
        case 1:
            shader_.getFragmentShaderObject()->addShaderDefine("BICUBIC_INTERPOLATION", "1");
            break;
        default:
            shader_.getFragmentShaderObject()->removeShaderDefine("BICUBIC_INTERPOLATION");
    }
    shader_.build();
}

void ImageResample::dimensionChanged() {
    if (targetResolution_.get() != ivec2(outport_.getDimensions())) {
        outport_.setDimensions(targetResolution_.get());
        internalInvalid_ = true;
    }
}

void ImageResample::dimensionSourceChanged() {
    switch (outputSizeMode_.get()) {
        case 0:  // inportDimension
            inport_.setOutportDeterminesSize(true);
            outport_.setHandleResizeEvents(false);
            targetResolution_.setVisible(false);
            internalInvalid_ = true;
            break;
        case 1:  // resizeEvents
            inport_.setOutportDeterminesSize(false);
            outport_.setHandleResizeEvents(true);
            targetResolution_.setVisible(false);
            break;
        case 2:  // custom
            inport_.setOutportDeterminesSize(false);
            outport_.setHandleResizeEvents(false);
            targetResolution_.setVisible(true);
            internalInvalid_ = true;
            targetResolution_.set(outport_.getDimensions());
            break;
    }
}

}  // namespace inviwo
