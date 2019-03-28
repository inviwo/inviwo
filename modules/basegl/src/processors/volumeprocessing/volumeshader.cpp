/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

namespace inviwo {

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
    : VolumeShader(std::make_shared<StringShaderResource>("volume_shader.frag",
                                                          getDefaultFragmentShader())) {}

VolumeShader::VolumeShader(std::shared_ptr<StringShaderResource> fragmentShader)
    : VolumeGLProcessor(fragmentShader, false)
    , fragmentShader_(fragmentShader)
    , fragmentSrc_("shader", "Shader", getDefaultFragmentShader(),
                   InvalidationLevel::InvalidResources, PropertySemantics::ShaderEditor) {
    addProperty(fragmentSrc_);

    fragmentSrc_.onChange([&]() { fragmentShader_->setSource(fragmentSrc_.get()); });
}

VolumeShader::~VolumeShader() {}

std::string VolumeShader::getDefaultFragmentShader() {
    std::ostringstream oss;

    oss << "#include \"utils/sampler3d.glsl\"" << std::endl << std::endl;
    oss << "uniform sampler3D volume;" << std::endl;
    oss << "uniform VolumeParameters volumeParameters;" << std::endl << std::endl;

    oss << "in vec4 texCoord_;" << std::endl << std::endl;

    oss << "void main() {" << std::endl;
    oss << "    vec4 v1 = getVoxel(volume , volumeParameters , texCoord_.xyz);" << std::endl;
    oss << "    FragData0 = v1;" << std::endl;
    oss << "}" << std::endl;
    oss << " " << std::endl;
    return oss.str();
}

}  // namespace inviwo
