/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#include <modules/postprocessing/processors/imagefilter.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ImageFilter::processorInfo_{
    "org.inviwo.ImageFilter",  // Class identifier
    "Image Filter",            // Display name
    "Image Operation",         // Category
    CodeState::Experimental,   // Code state
    Tags::GL,                  // Tags
};
const ProcessorInfo ImageFilter::getProcessorInfo() const { return processorInfo_; }

ImageFilter::ImageFilter()
    : Processor()
    , inport_("inputImage")
    , outport_("outputImage")
    , enable_("enable", "Enable", true)
    , filter_("filter", "Filter",
              {{"ssharpen", "Strong sharpen", 0},
               {"lsharpen", "Light sharpen", 1},
               {"blur", "Blur", 2},
               {"emboss", "Emboss", 3},
               {"outline", "Outline", 4},
               {"hsobel", "Sobel (horizontal)", 5},
               {"vsobel", "Sobel (vertical)", 6},
               {"hvlaplacian", "Laplacian (excl. diagonals)", 7},
               {"dlaplacian", "Laplacian (incl. diagonals)", 8},
               {"log", "Laplacian of Gaussian", 9}},
              0)
    , passes_("passes", "Passes", 1, 0, 5, 1)
    , convolution_([&]() { this->invalidate(InvalidationLevel::InvalidOutput); }) {

    addPort(inport_);
    addPort(outport_);

    addProperty(enable_);
    addProperty(filter_);
    addProperty(passes_);
}

void ImageFilter::process() {
    auto img = inport_.getData();

    const auto passes = passes_.get();

    if (!passes || !enable_.get()) {
        outport_.setData(img);
        return;
    }

    const auto filter = filter_.get();

    for (auto i{0}; i < passes; ++i) {
        img = convolution_.convolution(*img->getColorLayer(), filter < 9 ? 3 : 5,
                                       filter < 9 ? 3 : 5, kernels_[filter], 1.0f);
    }

    outport_.setData(img);
}
}  // namespace inviwo
