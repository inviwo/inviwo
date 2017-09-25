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
    , kernel_("kernel", "Kernel", kernels_[0])
    , sharpen_("sharpen", "Sharpen", true)
    , filter_("filter", "Filter", {{"filter1", "Kernel 1", 0}, {"filter2", "Kernel 2", 1}}, 0) {

    addProperty(sharpen_);
    addProperty(filter_);

    kernel_.setReadOnly(true);
    kernel_.setCurrentStateAsDefault();
    addProperty(kernel_);
}

void ImageSharpen::preProcess(TextureUnitContainer &) {
    kernel_.set(kernels_[filter_.get()]);
    utilgl::setUniforms(shader_, sharpen_);
    shader_.setUniform("kernel", kernels_[filter_.get()]);
}
}  // namespace inviwo
