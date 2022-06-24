/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2021 Inviwo Foundation
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

#include <modules/basegl/processors/imageprocessing/imageshader.h>

namespace inviwo {
namespace {
constexpr std::string_view defaultFrag = R"(
#include "utils/structs.glsl"

uniform ImageParameters outportParameters_;

uniform sampler2D inport_;

void main() {
    vec2 texCoords = gl_FragCoord.xy * outportParameters_.reciprocalDimensions;
    vec4 inputColor = texture(inport_, texCoords);
    FragData0 = inputColor;
}

)";

struct Helper {
    template <typename Format>
    void operator()(std::vector<OptionPropertyOption<DataFormatId>>& formats) {
        formats.emplace_back(Format::str(), Format::str(), Format::id());
    }
};
}

const ProcessorInfo ImageShader::processorInfo_{
    "org.inviwo.ImageShader",  // Class identifier
    "Image Shader",            // Display name
    "Image Operation",         // Category
    CodeState::Stable,         // Code state
    Tags::GL,                  // Tags
};

void ImageShader::initializeResources() {
    ImageGLProcessor::initializeResources();
    shader_.build();
}

const ProcessorInfo ImageShader::getProcessorInfo() const { return processorInfo_; }

ImageShader::ImageShader()
    : ImageShader(std::make_shared<StringShaderResource>("image_shader.frag", defaultFrag)) {}

ImageShader::ImageShader(std::shared_ptr<StringShaderResource> fragmentShader)
    : ImageGLProcessor(fragmentShader, false)
    , fragmentShader_(fragmentShader)
    , fragmentSrc_("shader", "Shader", std::string(defaultFrag),
                   InvalidationLevel::InvalidResources, PropertySemantics::ShaderEditor)
    , differentOutputFormat_("differentOutputFormat", "Different Output Format", false)
    , outputFormat_("outputFormat", "Output Format",
              [&]() {
                  std::vector<OptionPropertyOption<DataFormatId>> formats;
                  util::for_each_type<DefaultDataFormats>{}(
                      Helper(), formats);
                  return formats;
              }(),
              1){
    
    fragmentSrc_.onChange([&]() { fragmentShader_->setSource(fragmentSrc_.get()); });
    outputFormat_.onChange([&](){ internalInvalid_ = true; });
    differentOutputFormat_.onChange([&](){
        outputFormat_.setReadOnly(!differentOutputFormat_.get());
        internalInvalid_ = true;
    });
    addProperties(fragmentSrc_, differentOutputFormat_, outputFormat_);
}

void ImageShader::process() {
    if (differentOutputFormat_.get()) {
        dataFormat_ = DataFormatBase::get(outputFormat_.get());
    } else {
        dataFormat_ = nullptr;
    }
    ImageGLProcessor::process();
}

ImageShader::~ImageShader() {}

}  // namespace inviwo
