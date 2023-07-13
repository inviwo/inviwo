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

#include <modules/basegl/processors/volumeprocessing/volumeshader.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/filesystem.h>

#include <string_view>

namespace inviwo {

namespace {

constexpr std::string_view defaultFrag = R"(
#include "utils/sampler3d.glsl" 

uniform sampler3D volume;
uniform VolumeParameters volumeParameters;

in vec4 texCoord_;

void main() {
    vec4 v1 = getVoxel(volume , volumeParameters , texCoord_.xyz);
    FragData0 = v1;
}

)";

struct Helper {
    template <typename Format>
    void operator()(std::vector<OptionPropertyOption<DataFormatId>>& formats) {
        formats.emplace_back(Format::str(), Format::str(), Format::id());
    }
};
}  // namespace

const ProcessorInfo VolumeShader::processorInfo_{
    "org.inviwo.VolumeShader",  // Class identifier
    "Volume Shader",            // Display name
    "Volume Operation",         // Category
    CodeState::Stable,          // Code state
    Tags::GL,                   // Tags
};

void VolumeShader::initializeResources() {
    VolumeGLProcessor::initializeResources();
    shader_.build();
}

const ProcessorInfo VolumeShader::getProcessorInfo() const { return processorInfo_; }

VolumeShader::VolumeShader()
    : VolumeShader(std::make_shared<StringShaderResource>("volume_shader.frag", defaultFrag)) {}

VolumeShader::VolumeShader(std::shared_ptr<StringShaderResource> fragmentShader)
    : VolumeGLProcessor(fragmentShader, false)
    , fragmentShader_(fragmentShader)
    , fragmentSrc_("shader", "Shader", std::string(defaultFrag),
                   InvalidationLevel::InvalidResources, PropertySemantics::ShaderEditor)
    , differentOutputFormat_("differentOutputFormat", "Different Output Format", false)
    , outputFormat_(
          "outputFormat", "Output Format",
          [&]() {
              std::vector<OptionPropertyOption<DataFormatId>> formats;
              util::for_each_type<DefaultDataFormats>{}(Helper(), formats);
              return formats;
          }(),
          1)
    , differentOutputSize_("differentOutputSize", "Different Output Size", false)
    , outputSize_("outputSize", "Output Size", {512, 512, 512},
                  {{32, 32, 32}, ConstraintBehavior::Immutable},
                  {{1024, 1024, 1024}, ConstraintBehavior::Ignore}) {
    outputFormat_.onChange([&]() { internalInvalid_ = true; });
    outputSize_.onChange([&]() { internalInvalid_ = true; });
    differentOutputFormat_.onChange([&]() {
        outputFormat_.setReadOnly(!differentOutputFormat_);
        internalInvalid_ = true;
    });
    differentOutputSize_.onChange([&]() {
        outputSize_.setReadOnly(!differentOutputSize_);
        internalInvalid_ = true;
    });
    differentOutputFormat_.addProperty(outputFormat_);
    differentOutputSize_.addProperty(outputSize_);
    addProperties(fragmentSrc_, differentOutputFormat_, differentOutputSize_);

    fragmentSrc_.onChange([&]() { fragmentShader_->setSource(fragmentSrc_.get()); });
}

void VolumeShader::process() {
    if (differentOutputFormat_) {
        dataFormat_ = DataFormatBase::get(outputFormat_.get());
    } else {
        dataFormat_ = nullptr;
    }
    if (differentOutputSize_) {
        dataSize_ = outputSize_.get();
    } else {
        dataSize_ = {};
    }
    VolumeGLProcessor::process();
}

VolumeShader::~VolumeShader() = default;

}  // namespace inviwo
