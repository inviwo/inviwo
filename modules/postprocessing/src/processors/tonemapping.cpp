/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <modules/postprocessing/processors/tonemapping.h>

#include <inviwo/core/processors/processorinfo.h>                        // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                       // for CodeState, CodeS...
#include <inviwo/core/processors/processortags.h>                        // for Tags, Tags::GL
#include <inviwo/core/properties/invalidationlevel.h>                    // for InvalidationLevel
#include <inviwo/core/properties/optionproperty.h>                       // for OptionPropertyOp...
#include <inviwo/core/properties/ordinalproperty.h>                      // for FloatProperty
#include <modules/basegl/processors/imageprocessing/imageglprocessor.h>  // for ImageGLProcessor
#include <modules/opengl/shader/shader.h>                                // for Shader
#include <modules/opengl/shader/shaderobject.h>                          // for ShaderObject
#include <modules/opengl/shader/shaderutils.h>                           // for setUniforms

#include <functional>   // for __base
#include <string>       // for string, to_string
#include <string_view>  // for string_view

namespace inviwo {
class TextureUnitContainer;

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo Tonemapping::processorInfo_{
    "org.inviwo.Tonemapping",  // Class identifier
    "Tonemapping",             // Display name
    "Image Operation",         // Category
    CodeState::Stable,         // Code state
    Tags::GL,                  // Tags
};
const ProcessorInfo Tonemapping::getProcessorInfo() const { return processorInfo_; }

enum Method { None, Gamma, Reinhard, Uncharted2 };

Tonemapping::Tonemapping()
    : ImageGLProcessor("tonemapping.frag")
    , method_("method", "Method",
              {{"none", "None", 0},
               {"gamma", "Gamma", 1},
               {"reinhard", "Reinhard", 2},
               {"uncharted2", "Uncharted 2", 3}},
              0, InvalidationLevel::InvalidResources)
    , exposure_("exposure", "Exposure", 1.f, 0.01f, 64.f)
    , gamma_("gamma", "Gamma", 2.2f, 0.5f, 8.0f) {
    addProperty(method_);
    addProperty(exposure_);
    addProperty(gamma_);

    method_.onChange([this]() {
        // TODO add parameters
        switch (method_.get()) {
            case Method::None:
                exposure_.setVisible(true);
                gamma_.setVisible(false);
                break;
            case Method::Gamma:
                exposure_.setVisible(true);
                gamma_.setVisible(true);
                break;
            case Method::Reinhard:
                exposure_.setVisible(true);
                gamma_.setVisible(true);
                break;
            case Method::Uncharted2:
                exposure_.setVisible(true);
                gamma_.setVisible(true);
                break;
        }
    });
}

void Tonemapping::initializeResources() {
    shader_.getFragmentShaderObject()->addShaderDefine("METHOD", std::to_string(method_.get()));
    ImageGLProcessor::initializeResources();
}

void Tonemapping::preProcess(TextureUnitContainer&) {
    utilgl::setUniforms(shader_, exposure_, gamma_);
}

}  // namespace inviwo
