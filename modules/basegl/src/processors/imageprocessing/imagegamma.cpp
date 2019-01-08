/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2018 Inviwo Foundation
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

#include <modules/basegl/processors/imageprocessing/imagegamma.h>
#include <modules/opengl/shader/shader.h>

namespace inviwo {

const ProcessorInfo ImageGamma::processorInfo_{
    "org.inviwo.ImageGamma",  // Class identifier
    "Image Gamma",            // Display name
    "Image Operation",        // Category
    CodeState::Stable,        // Code state
    Tags::GL,                 // Tags
};
const ProcessorInfo ImageGamma::getProcessorInfo() const { return processorInfo_; }

ImageGamma::ImageGamma()
    : ImageGLProcessor("img_gamma.frag")
    , gamma_("gammaFactor", "Gamma Correction", 1.0f, 0.0f, 2.0f, 0.01f) {
    addProperty(gamma_);
}

ImageGamma::~ImageGamma() = default;

void ImageGamma::preProcess(TextureUnitContainer &) { shader_.setUniform("gamma_", gamma_.get()); }

}  // namespace inviwo
