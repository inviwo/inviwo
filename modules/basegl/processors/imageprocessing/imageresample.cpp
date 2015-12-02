/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include "imageresample.h"
#include <modules/opengl/shader/shader.h>

namespace inviwo {

const ProcessorInfo ImageResample::processorInfo_{
    "org.inviwo.ImageResample",  // Class identifier
    "Image Resample",            // Display name
    "Image Operation",           // Category
    CodeState::Stable,           // Code state
    Tags::GL,                    // Tags
};
const ProcessorInfo ImageResample::getProcessorInfo() const {
    return processorInfo_;
}

ImageResample::ImageResample()
    : ImageGLProcessor("img_resample.frag")
    , interpolationType_("interpolationType", "Interpolation Type")
    , dimensionSource_("dimensionSource", "Dimension Source")
    , dimensions_("dimensions", "Outport dimensions", ivec2(256, 256), ivec2(128, 128),
                  ivec2(4096, 4096), ivec2(1, 1)) {
        
    interpolationType_.addOption("bilinear", "Bilinear", 0);
    interpolationType_.addOption("bicubic", "Bicubic", 1);
    interpolationType_.set(0);
    interpolationType_.onChange(this, &ImageResample::interpolationTypeChanged);
    interpolationType_.setCurrentStateAsDefault();
    addProperty(interpolationType_);

    dimensionSource_.addOption("inportDimension", "Use Inport for Outport Dimensions", 0);
    dimensionSource_.addOption("resizeEvents", "Use Resize Events for Outport Dimensions", 1);
    dimensionSource_.addOption("custom", "Use Custom Dimension for Outport Dimensions", 2);
    dimensionSource_.set(0);
    dimensionSource_.setCurrentStateAsDefault();
    dimensionSource_.onChange(this, &ImageResample::dimensionSourceChanged);
    addProperty(dimensionSource_);

    dimensions_.onChange(this, &ImageResample::dimensionChanged);
    addProperty(dimensions_);
}

ImageResample::~ImageResample() {}

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
    if (dimensions_.get() != ivec2(outport_.getDimensions())) {
        outport_.setDimensions(dimensions_.get());
        internalInvalid_ = true;
    }
}

void ImageResample::dimensionSourceChanged() {
    switch (dimensionSource_.get()) {
        case 0:  // inportDimension
            inport_.setOutportDeterminesSize(true);
            outport_.setHandleResizeEvents(false);
            dimensions_.setVisible(false);
            internalInvalid_ = true;
            break;
        case 1:  // resizeEvents
            inport_.setOutportDeterminesSize(false);
            outport_.setHandleResizeEvents(true);
            dimensions_.setVisible(false);
            break;
        case 2:  // custom
            inport_.setOutportDeterminesSize(false);
            outport_.setHandleResizeEvents(false);
            dimensions_.setVisible(true);
            internalInvalid_ = true;
            dimensions_.set(outport_.getDimensions());
            break;
    }
}

}  // namespace

