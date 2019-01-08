/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2018 Inviwo Foundation
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

#include <modules/basegl/processors/imageprocessing/imagecompositeprocessorgl.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/texture/textureutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ImageCompositeProcessorGL::processorInfo_{
    "org.inviwo.ImageCompositeProcessorGL",  // Class identifier
    "Image Composite",                       // Display name
    "Image Operation",                       // Category
    CodeState::Experimental,                 // Code state
    Tags::GL,                                // Tags
};
const ProcessorInfo ImageCompositeProcessorGL::getProcessorInfo() const { return processorInfo_; }

ImageCompositeProcessorGL::ImageCompositeProcessorGL()
    : Processor()
    , imageInport1_("imageInport1")
    , imageInport2_("imageInport2")
    , outport_("outport") {
    addPort(imageInport1_);
    addPort(imageInport2_);
    addPort(outport_);
}

void ImageCompositeProcessorGL::process() {
    utilgl::activateTargetAndCopySource(outport_, imageInport1_);
    utilgl::deactivateCurrentTarget();

    compositor_.composite(imageInport2_, outport_, ImageType::ColorDepth);
}

}  // namespace inviwo
