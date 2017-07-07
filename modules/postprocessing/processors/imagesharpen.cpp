/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#include <modules/postprocessing/processors/imagesharpen.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ImageSharpen::processorInfo_{
    "org.inviwo.ImageSharpen",  // Class identifier
    "Image Sharpen",            // Display name
    "Image Operation",          // Category
    CodeState::Experimental,    // Code state
    Tags::None,                 // Tags
};
const ProcessorInfo ImageSharpen::getProcessorInfo() const { return processorInfo_; }

ImageSharpen::ImageSharpen()
    : ImageGLProcessor("imagesharpen.frag")
    , passes_("passes", "Passes", 1, 1, 10, 1)
    , sharpen_("sharpen", "Sharpen", true)
    , filter_("filter", "Filter") {

    filter_.addOption("filter1", "Filter 1", 0);
    filter_.addOption("filter2", "Filter 2", 1);
    filter_.setSelectedValue(0);
    filter_.setCurrentStateAsDefault();

    addProperty(sharpen_);
    addProperty(filter_);
}

void ImageSharpen::preProcess(TextureUnitContainer &cont) {
    mat3 kernel;

    if (filter_.get() == 0) {
        kernel[0] = vec3(-1, -1, -1);
        kernel[1] = vec3(-1, 8, -1);
        kernel[2] = vec3(-1, -1, -1);
    }
    if (filter_.get() == 1) {
        kernel[0] = vec3(0, -1, 0);
        kernel[1] = vec3(-1, 4, -1);
        kernel[2] = vec3(0, -1, 0);
    }

    utilgl::setUniforms(shader_, sharpen_);
    shader_.setUniform("kernel", kernel);
}
}  // namespace
