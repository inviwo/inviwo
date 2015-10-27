/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include "imagegrayscale.h"
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shader.h>

namespace inviwo {

const ProcessorInfo ImageGrayscale::processorInfo_{
    "org.inviwo.ImageGrayscale",  // Class identifier
    "Image Grayscale",            // Display name
    "Image Operation",            // Category
    CodeState::Stable,            // Code state
    Tags::GL,                     // Tags
};
const ProcessorInfo ImageGrayscale::getProcessorInfo() const {
    return processorInfo_;
}

ImageGrayscale::ImageGrayscale()
    : ImageGLProcessor("img_graysc.frag")
    , luminanceModel_("luminanceModel", "Luminance Model")
{
    luminanceModel_.addOption("perceived", "Perceived", LuminanceModels::PerceivedLum);
    luminanceModel_.addOption("relative", "Relative", LuminanceModels::RelativeLum);
    luminanceModel_.addOption("average", "Average", LuminanceModels::AverageLum);
    luminanceModel_.addOption("red", "Red only", LuminanceModels::RedOnly);
    luminanceModel_.addOption("green", "Green only", LuminanceModels::GreenOnly);
    luminanceModel_.addOption("blue", "Blue only", LuminanceModels::BlueOnly);
    luminanceModel_.setSelectedValue(LuminanceModels::PerceivedLum);
    luminanceModel_.setCurrentStateAsDefault();

    addProperty(luminanceModel_);
}

ImageGrayscale::~ImageGrayscale() {}

void ImageGrayscale::preProcess() {
    vec3 lumScale(1.0f);
    switch (luminanceModel_.get()) {
    case LuminanceModels::PerceivedLum:
        lumScale = vec3(0.299f, 0.587f, 0.114f);
        break;
    case LuminanceModels::RelativeLum:
        lumScale = vec3(0.2126, 0.7152, 0.0722);
        break;
    case LuminanceModels::AverageLum:
        lumScale = vec3(1.0f/3.0f);
        break;
    case LuminanceModels::RedOnly:
        lumScale = vec3(1.0f, 0.0f, 0.0f);
        break;
    case LuminanceModels::GreenOnly:
        lumScale = vec3(0.0f, 1.0f, 0.0f);
        break;
    case LuminanceModels::BlueOnly:
        lumScale = vec3(0.0f, 0.0f, 1.0f);
        break;
    default:
        break;
    }

    shader_.setUniform("lumScale_", lumScale);
}

}  // namespace

