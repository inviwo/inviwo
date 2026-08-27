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

#include <modules/basegl/shadercomponents/maskcomponent.h>

#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/shader/stringshaderresource.h>

#include <ranges>

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

constexpr std::string_view uniforms = util::trim(R"(
uniform VolumeParameters {0}Parameters;
uniform sampler3D {0};
)");

constexpr std::string_view first = util::trim(R"(
float {0}Value = getNormalizedVoxel({0}, {0}Parameters, {1}SamplePosition).x;
float {0}ValuePrev = {0}Value;
if ({0}Value > 0.0) {{
   color = vec4(0);
}}
)");

constexpr std::string_view loop = util::trim(R"(
{0}Value = getNormalizedVoxel({0}, {0}Parameters, {1}SamplePosition).x;
if ({0}Value > 0.0) {{
   {0}ValuePrev = {0}Value;
   continue;
}}
)");

constexpr std::string_view loop2 = util::trim(R"(
if ({0}ValuePrev > 0.0) {{
    // We just left a masked region.
    // Reset the prev values to the current value.
    {1}VoxelPrev = {1}Voxel;
    {1}GradientPrev = {1}Gradient;
}}
{0}ValuePrev = {0}Value;
)");

}  // namespace

namespace {

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

MaskComponent::MaskComponent(VolumeInport& port)
    : ShaderComponent{}
    , name_{fmt::format("{}Mask", port.getIdentifier())}
    , port_{&port}
    , options_{fmt::format("enable{}", name_),
               fmt::format("Enable Masking for {}", port.getIdentifier()), false,
               InvalidationLevel::InvalidResources}
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
    , fbo_{} {

    options_.addProperties(maskMissingValue_, maskZero_, maskNaN_, maskInf_);
}

std::string_view MaskComponent::getName() const { return name_; }

void MaskComponent::initializeResources(Shader&) {}

void MaskComponent::process(Shader& shader, TextureUnitContainer& cont) {
    if (options_.isChecked() && mask_) {
        utilgl::bindAndSetUniforms(shader, cont, *mask_, name_);
    }
}

auto MaskComponent::getSegments() -> std::vector<Segment> {
    if (options_.isChecked()) {
        return {{.snippet = fmt::format(uniforms, name_),
                 .placeholder = placeholder::uniform,
                 .priority = 410},
                {.snippet = fmt::format(first, name_, port_->getIdentifier()),
                 .placeholder = placeholder::first,
                 .priority = 950},
                {.snippet = fmt::format(loop, name_, port_->getIdentifier()),
                 .placeholder = placeholder::loop,
                 .priority = 410},
                {.snippet = fmt::format(loop2, name_, port_->getIdentifier()),
                 .placeholder = placeholder::loop,
                 .priority = 550}};
    } else {
        return {};
    }
}

std::vector<Property*> MaskComponent::getProperties() { return {&options_}; }

void MaskComponent::preprocess() {
    if (!options_.isChecked()) return;

    if (!port_) return;

    bool dirty = port_->isChanged();
    auto data = port_->getData();
    if (!mask_ || data->getDimensions() != mask_->getDimensions()) {
        mask_ = std::make_shared<Volume>(VolumeConfig{.dimensions = data->getDimensions(),
                                                      .format = DataUInt8::get(),
                                                      .dataRange = vec2(0, 255),
                                                      .valueRange = vec2(0, 255),
                                                      .model = data->getModelMatrix(),
                                                      .world = data->getWorldMatrix()});
        dirty = true;
    }

    if (maskZero_.isModified() || maskMissingValue_.isModified() || maskNaN_.isModified() ||
        maskInf_.isModified()) {
        frag_->setSource(makeFragmentShader(maskMissingValue_.get(), maskZero_.get(),
                                            maskNaN_.get(), maskInf_.get()));
        // The shader will rebuild itself.
        dirty = true;
    }

    if (!shader_.isReady()) {
        shader_.build();
    }

    if (!dirty) return;

    const utilgl::Activate aShader{&shader_};

    TextureUnit unit;
    utilgl::bindTexture(*data, unit);
    shader_.setUniform("volume", unit.getUnitNumber());
    utilgl::setShaderUniforms(shader_, *mask_, "volumeParameters");

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

    const auto dim = static_cast<ivec3>(mask_->getDimensions());

    const utilgl::Activate aFbo{&fbo_};
    const utilgl::ViewportState vp{0, 0, dim.x, dim.y};
    auto* maskGL = mask_->getEditableRepresentation<VolumeGL>();
    fbo_.attachColorTexture(maskGL->getTexture().get(), 0);
    utilgl::multiDrawImagePlaneRect(dim.z);
}

}  // namespace inviwo
