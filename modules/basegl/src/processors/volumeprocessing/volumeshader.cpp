/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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

#include <inviwo/core/processors/processorinfo.h>                          // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                         // for CodeState, Cod...
#include <inviwo/core/processors/processortags.h>                          // for Tags, Tags::GL
#include <inviwo/core/properties/invalidationlevel.h>                      // for InvalidationLevel
#include <inviwo/core/properties/propertysemantics.h>                      // for PropertySemantics
#include <inviwo/core/properties/stringproperty.h>                         // for StringProperty
#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>  // for VolumeGLProcessor
#include <modules/opengl/shader/shader.h>                                  // for Shader
#include <modules/opengl/shader/stringshaderresource.h>                    // for StringShaderRe...

#include <functional>   // for __base
#include <string>       // for string, basic_...
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t

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
}

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

const ProcessorInfo& VolumeShader::getProcessorInfo() const { return processorInfo_; }

VolumeShader::VolumeShader()
    : VolumeShader(std::make_shared<StringShaderResource>("volume_shader.frag", defaultFrag)) {}

VolumeShader::VolumeShader(std::shared_ptr<StringShaderResource> fragmentShader)
    : VolumeGLProcessor(fragmentShader, false)
    , fragmentShader_(fragmentShader)
    , fragmentSrc_("shader", "Shader", std::string(defaultFrag),
                   InvalidationLevel::InvalidResources, PropertySemantics::ShaderEditor) {
    addProperty(fragmentSrc_);

    fragmentSrc_.onChange([&]() { fragmentShader_->setSource(fragmentSrc_.get()); });
}

VolumeShader::~VolumeShader() = default;

}  // namespace inviwo
