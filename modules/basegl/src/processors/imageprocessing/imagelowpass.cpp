/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2024 Inviwo Foundation
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

#include <modules/basegl/processors/imageprocessing/imagelowpass.h>

#include <inviwo/core/ports/imageport.h>                // for ImageInport, ImageOutport
#include <inviwo/core/processors/processor.h>           // for Processor
#include <inviwo/core/processors/processorinfo.h>       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>      // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>       // for Tags, Tags::GL
#include <inviwo/core/properties/boolproperty.h>        // for BoolProperty
#include <inviwo/core/properties/invalidationlevel.h>   // for InvalidationLevel, InvalidationLe...
#include <inviwo/core/properties/ordinalproperty.h>     // for IntProperty, FloatProperty
#include <modules/basegl/algorithm/imageconvolution.h>  // for ImageConvolution

#include <functional>   // for __base
#include <memory>       // for shared_ptr
#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t

namespace inviwo {
// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ImageLowPass::processorInfo_{
    "org.inviwo.ImageLowPass",  // Class identifier
    "Image Low Pass",           // Display name
    "Image Operation",          // Category
    CodeState::Stable,          // Code state
    Tags::GL,                   // Tags
};
const ProcessorInfo& ImageLowPass::getProcessorInfo() const { return processorInfo_; }

ImageLowPass::ImageLowPass()
    : Processor()
    , inport_("inputImage")
    , outport_("outputImage")
    , kernelSize_("kernelSize", "Kernel Size", 3, 1, 25, 1)
    , gaussian_("gaussian", "Use Gaussian weights", true)
    , sigma_("sigma", "Sigma", 1.f, 1.f, 100.f, 0.01f)
    , convolution_([&]() { this->invalidate(InvalidationLevel::InvalidOutput); }) {

    addPort(inport_);
    addPort(outport_);

    addProperty(kernelSize_);
    addProperty(sigma_);
    addProperty(gaussian_);
    kernelSize_.setVisible(false);
    gaussian_.onChange([&]() {
        kernelSize_.setVisible(!gaussian_.get());
        sigma_.setVisible(gaussian_.get());
    });
}

void ImageLowPass::process() {
    if (gaussian_) {
        outport_.setData(
            convolution_.gaussianLowpass(*inport_.getData()->getColorLayer(), sigma_.get()));

    } else {
        outport_.setData(
            convolution_.lowpass(*inport_.getData()->getColorLayer(), kernelSize_.get()));
    }
}

}  // namespace inviwo
