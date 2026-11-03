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
    , maskTool_{} {

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

    const auto data = port_->getData();
    if (!mask_ || port_->isChanged() || data->getDimensions() != mask_->getDimensions() ||
        maskZero_.isModified() || maskMissingValue_.isModified() || maskNaN_.isModified() ||
        maskInf_.isModified()) {

        mask_ = maskTool_.process(
            mask::Config{
                .maskMissingValue = maskMissingValue_.get(),
                .maskZero = maskZero_.get(),
                .maskNaN = maskNaN_.get(),
                .maskInf = maskInf_.get(),
            },
            *data, mask_);
    }
}

}  // namespace inviwo
