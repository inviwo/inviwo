/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/basegl/processors/volumeprocessing/volumemask.h>

#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/shader/stringshaderresource.h>

namespace inviwo {

namespace {

constexpr std::string_view mask_frag = R"(
#include "utils/sampler3d.glsl"

in vec4 texCoord_;

uniform sampler3D volume;
uniform VolumeParameters volumeParameters;

uniform float maskValue;

void main() {{
    float value = getVoxel(volume, volumeParameters, texCoord_.xyz).r;
    float mask = ({}) ? 1.0 : 0.0;
    FragData0 = vec4(vec3(mask), 1.0);
}}

)";

std::string makeFragmentShader(bool maskMissingValue, bool maskZero, bool maskNaN, bool maskInf) {

    const auto mask_frag_conditions = std::to_array<std::pair<bool, std::string_view>>({
        {maskMissingValue, "value == maskValue"},
        {maskZero, "value == 0.0"},
        {maskNaN, "isnan(value)"},
        {maskInf, "isinf(value)"},
    });

    auto active = mask_frag_conditions | std::views::filter([](const auto& c) { return c.first; }) |
                  std::views::transform([](const auto& c) { return c.second; });

    if (std::ranges::empty(active)) {
        return fmt::format(mask_frag, "false");
    } else {
        return fmt::format(mask_frag, fmt::join(active, " || "));
    }
}

}  // namespace

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeMask::processorInfo_{
    "org.inviwo.VolumeMask",  // Class identifier
    "Volume Mask",            // Display name
    "Undefined",              // Category
    CodeState::Experimental,  // Code state
    Tags::GL,                 // Tags
    R"(Construct a binary mask volume base on a input volume)"_unindentHelp,
};

const ProcessorInfo& VolumeMask::getProcessorInfo() const { return processorInfo_; }

VolumeMask::VolumeMask()
    : Processor{}
    , inport_{"inport", ""_help}
    , outport_{"outport", ""_help}

    , maskMissingValue_{"maskMissingValue", "Mask missing values", false}
    , maskZero_{"maskZero", "Mask zero values", false}
    , maskNaN_{"maskNaN", "Mask NaN values", false}
    , maskInf_{"maskInf", "Mask Inf values", false}
    , frag_{std::make_shared<StringShaderResource>(
          "mask.frag", makeFragmentShader(maskMissingValue_.get(), maskZero_.get(), maskNaN_.get(),
                                          maskInf_.get()))}
    , shader_{{{ShaderType::Vertex, utilgl::findShaderResource("volume_gpu.vert")},
               {ShaderType::Geometry, utilgl::findShaderResource("volume_gpu.geom")},
               {ShaderType::Fragment, std::static_pointer_cast<const ShaderResource>(frag_)}},
              Shader::Build::No}
    , fbo_{}

{

    addPorts(inport_, outport_);
    addProperties(maskMissingValue_, maskZero_, maskNaN_, maskInf_);
}

void VolumeMask::process() {
    auto data = inport_.getData();
    auto mask = std::make_shared<Volume>(VolumeConfig{.dimensions = data->getDimensions(),
                                                      .format = DataUInt8::get(),
                                                      .dataRange = vec2(0, 255),
                                                      .valueRange = vec2(0, 255),
                                                      .model = data->getModelMatrix(),
                                                      .world = data->getWorldMatrix()});

    if (maskZero_.isModified() || maskMissingValue_.isModified() || maskNaN_.isModified() ||
        maskInf_.isModified()) {
        frag_->setSource(makeFragmentShader(maskMissingValue_.get(), maskZero_.get(),
                                            maskNaN_.get(), maskInf_.get()));
    }

    if (!shader_.isReady()) {
        shader_.build();
    }

    const utilgl::Activate aShader{&shader_};

    const TextureUnit unit;
    utilgl::bindTexture(*data, unit);
    shader_.setUniform("volume", unit.getUnitNumber());
    utilgl::setShaderUniforms(shader_, *mask, "volumeParameters");

    if (maskMissingValue_.get()) {
        const auto* missingDouble = data->getMetaData<MetaDataType<double>>("missing_value");
        const auto* missingInt = data->getMetaData<MetaDataType<std::int64_t>>("missing_value");
        if (missingDouble) {
            shader_.setUniform("maskValue", static_cast<float>(missingDouble->get()));
        } else if (missingInt) {
            shader_.setUniform("maskValue", static_cast<float>(missingInt->get()));
        } else {
            throw Exception("Missing value masking enabled but no 'missing_value' metadata found");
        }
    }

    const auto dim = static_cast<ivec3>(mask->getDimensions());

    const utilgl::Activate aFbo{&fbo_};
    const utilgl::ViewportState vp{0, 0, dim.x, dim.y};
    auto* maskGL = mask->getEditableRepresentation<VolumeGL>();
    fbo_.attachColorTexture(maskGL->getTexture().get(), 0);
    utilgl::multiDrawImagePlaneRect(dim.z);

    outport_.setData(mask);
}

}  // namespace inviwo
