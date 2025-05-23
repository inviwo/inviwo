/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <modules/basegl/processors/imageprocessing/jacobian2d.h>

#include <inviwo/core/processors/processorinfo.h>                        // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                       // for CodeState, CodeS...
#include <inviwo/core/processors/processortags.h>                        // for Tags, Tags::GL
#include <inviwo/core/properties/boolproperty.h>                         // for BoolProperty
#include <inviwo/core/properties/invalidationlevel.h>                    // for InvalidationLevel
#include <inviwo/core/util/formats.h>                                    // for DataFormat, Data...
#include <modules/basegl/processors/imageprocessing/imageglprocessor.h>  // for ImageGLProcessor
#include <modules/opengl/shader/shader.h>                                // for Shader
#include <modules/opengl/shader/shaderobject.h>                          // for ShaderObject

#include <string>       // for string
#include <string_view>  // for string_view

namespace inviwo {
class TextureUnitContainer;

const ProcessorInfo Jacobian2D::processorInfo_{
    "org.inviwo.Jacobian2D",  // Class identifier
    "Jacobian2D",             // Display name
    "Image Operation",        // Category
    CodeState::Stable,        // Code state
    Tags::GL,                 // Tags
    "Computes the Jacobian of a two channel image."_help};
const ProcessorInfo& Jacobian2D::getProcessorInfo() const { return processorInfo_; }

Jacobian2D::Jacobian2D()
    : ImageGLProcessor("img_jacobian.frag")
    , renormalization_("renormalization", "Renormalization",
                       "Re-normalize results by taking the grid spacing into account"_help, true)
    , inverse_("inverse", "Invert Jacobian (J^-1)",
               "If enabled, the processor outputs the inverse of the Jacobian"_help, false,
               InvalidationLevel::InvalidResources) {
    dataFormat_ = DataVec4Float32::get();

    inport_.setHelp("Input image (only first two channels are used)"_help);
    outport_.setHelp("Resulting Jacobian (du, dv) or its inverse (if enabled)"_help);

    addProperty(renormalization_);
}

void Jacobian2D::initializeResources() {
    if (inverse_.get()) {
        shader_.getFragmentShaderObject()->addShaderDefine("INVERT_JACOBIAN");
    } else {
        shader_.getFragmentShaderObject()->removeShaderDefine("INVERT_JACOBIAN");
    }
    ImageGLProcessor::initializeResources();
}

void Jacobian2D::preProcess(TextureUnitContainer&) {
    shader_.setUniform("renormalization", renormalization_.get());
}

}  // namespace inviwo
