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

#include <modules/basegl/algorithm/masking.h>

#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/shader/stringshaderresource.h>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <ranges>

namespace inviwo::mask {

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

}  // namespace

std::string makeFragmentShader(Config config) {
    const auto mask_frag_conditions = std::to_array<std::pair<bool, std::string_view>>({
        {config.maskMissingValue, "value == maskValue"},
        {config.maskZero, "value == 0.0"},
        {config.maskNaN, "isnan(value)"},
        {config.maskInf, "isinf(value)"},
    });

    auto active = mask_frag_conditions | std::views::filter([](const auto& c) { return c.first; }) |
                  std::views::transform([](const auto& c) { return c.second; });

    if (std::ranges::empty(active)) {
        return fmt::format(mask_frag, "false");
    } else {
        return fmt::format(mask_frag, fmt::join(active, " || "));
    }
}

MaskTool::MaskTool()
    : current{}
    , frag{std::make_shared<StringShaderResource>("mask.frag", makeFragmentShader(current))}
    , shader{{{ShaderType::Vertex, utilgl::findShaderResource("volume_gpu.vert")},
              {ShaderType::Geometry, utilgl::findShaderResource("volume_gpu.geom")},
              {ShaderType::Fragment, std::static_pointer_cast<const ShaderResource>(frag)}},
             Shader::Build::No}
    , fbo{} {}

std::shared_ptr<Volume> MaskTool::process(const Config& config, const Volume& volume,
                                          std::shared_ptr<Volume> reuse) {
    const VolumeConfig volumeConfig = {
        .dimensions = volume.getDimensions(),
        .formatId = DataFormatId::UInt8,
        .dataRange = vec2(0, 255),
        .valueRange = vec2(0, 255),
        .model = volume.getModelMatrix(),
        .world = volume.getWorldMatrix(),
    };

    auto mask =
        (reuse && reuse->config() == volumeConfig) ? reuse : std::make_shared<Volume>(volumeConfig);

    if (config != current) {
        frag->setSource(makeFragmentShader(config));
        current = config;
    }

    if (!shader.isReady()) {
        shader.build();
    }

    const utilgl::Activate aShader{&shader};

    const TextureUnit unit;
    utilgl::bindTexture(volume, unit);
    shader.setUniform("volume", unit.getUnitNumber());
    utilgl::setShaderUniforms(shader, *mask, "volumeParameters");

    if (current.maskMissingValue) {
        const auto* missingDouble = volume.getMetaData<MetaDataType<double>>("missing_value");
        const auto* missingInt = volume.getMetaData<MetaDataType<std::int64_t>>("missing_value");
        if (missingDouble) {
            shader.setUniform("maskValue", static_cast<float>(missingDouble->get()));
        } else if (missingInt) {
            shader.setUniform("maskValue", static_cast<float>(missingInt->get()));
        } else {
            throw Exception("Missing value masking enabled but no 'missing_value' metadata found");
        }
    }

    const auto dim = static_cast<ivec3>(mask->getDimensions());

    const utilgl::Activate aFbo{&fbo};
    const utilgl::ViewportState vp{0, 0, dim.x, dim.y};
    const auto* maskGL = mask->getEditableRepresentation<VolumeGL>();
    fbo.attachColorTexture(maskGL->getTexture().get(), 0);
    utilgl::multiDrawImagePlaneRect(dim.z);

    return mask;
}

}  // namespace inviwo::mask
