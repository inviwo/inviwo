/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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
#include <modules/opengl/shader/shader.h>

namespace inviwo {

const ProcessorInfo ImageResample::processorInfo_{
    "org.inviwo.ImageResample",  // Class identifier
    "Image Resample",            // Display name
    "Image Operation",           // Category
    CodeState::Stable,           // Code state
    Tags::GL,                    // Tags
};
const ProcessorInfo ImageResample::getProcessorInfo() const { return processorInfo_; }

ImageResample::ImageResample()
    : ImageGLProcessor("img_resample.frag")
    , interpolationType_("interpolationType", "Interpolation Type")
    , outputSizeMode_("outputSizeMode", "Output Size Mode")
    , targetResolution_("targetResolution", "Target Resolution", ivec2(256, 256), ivec2(32, 32),
                        ivec2(4096, 4096), ivec2(1, 1)) {

    interpolationType_.addOption("bilinear", "Bilinear", 0);
    interpolationType_.addOption("bicubic", "Bicubic", 1);
    interpolationType_.set(0);
    interpolationType_.onChange([this]() { interpolationTypeChanged(); });
    interpolationType_.setCurrentStateAsDefault();
    addProperty(interpolationType_);

    outputSizeMode_.addOption("inportDimension", "Inport Dimensions", 0);
    outputSizeMode_.addOption("resizeEvents", "Resize Events", 1);
    outputSizeMode_.addOption("custom", "Custom Dimensions", 2);
    outputSizeMode_.set(0);
    outputSizeMode_.setCurrentStateAsDefault();
    outputSizeMode_.onChange([this]() { dimensionSourceChanged(); });
    addProperty(outputSizeMode_);

    targetResolution_.onChange([this]() { dimensionChanged(); });
    addProperty(targetResolution_);
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
