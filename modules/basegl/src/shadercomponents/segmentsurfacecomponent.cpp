/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <modules/basegl/shadercomponents/segmentsurfacecomponent.h>

#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

SegmentSurfaceComponent::SegmentSurfaceComponent(VolumeInport& atlas)
    : ShaderComponent{}
    , name_("atlasboundary")
    , useAtlasBoundary_{"useAtlasBoundary", "Render ISO using atlas boundary", false}
    , applyBoundaryLight_{"applyBoundaryLight", "Apply lighting to boundary", true}
    , showHighlighted_("showHighlightedIndices", "Show Highlighted", true,
                       vec3(1.0f, 0.906f, 0.612f))
    , showSelected_("showSelectedIndices", "Show Selected", true, vec3(1.0f, 0.769f, 0.247f))
    , showFiltered_("showFilteredIndices", "Show Filtered", true, vec3(0.5f, 0.5f, 0.5f))
    , nearestSampler_{}
    , linearSampler_{}
    , textureSpaceGradientSpacingScale_{"gradientScaling", "Scale gradient spacing", 1.0f, 0.0f,
                                        5.0f}
    , atlas_{atlas} {

    useAtlasBoundary_.addProperties(applyBoundaryLight_, textureSpaceGradientSpacingScale_,
                                    showHighlighted_, showFiltered_, showSelected_);

    nearestSampler_.setFilterModeAll(InterpolationType::Nearest);
    nearestSampler_.setWrapModeAll(Wrapping::Repeat);
    linearSampler_.setFilterModeAll(InterpolationType::Linear);
    linearSampler_.setWrapModeAll(Wrapping::Repeat);
}
std::string_view SegmentSurfaceComponent::getName() const { return name_; }

void SegmentSurfaceComponent::process(Shader& shader, TextureUnitContainer& cont) {
    shader.setUniform("useAtlasBoundary", useAtlasBoundary_.getBoolProperty()->get());
    shader.setUniform("applyBoundaryLight", applyBoundaryLight_.get());
    shader.setUniform("gradientSpacingScale", textureSpaceGradientSpacingScale_.get());
    shader.setUniform("selectedColor", showSelected_.getColor());
    shader.setUniform("showSelected", showSelected_.getBoolProperty()->get());
    shader.setUniform("highlightedColor", showHighlighted_.getColor());
    shader.setUniform("showHighlighted", showHighlighted_.getBoolProperty()->get());
    shader.setUniform("filteredColor", showFiltered_.getColor());
    shader.setUniform("showFiltered", showFiltered_.getBoolProperty()->get());

    auto& linearUnit = cont.emplace_back();
    linearUnit.bindSampler(linearSampler_);
    shader.setUniform("linearAtlas", linearUnit);
    utilgl::bindTexture(atlas_, linearUnit);
}
std::vector<Property*> SegmentSurfaceComponent::getProperties() { return {&useAtlasBoundary_}; }

namespace {
constexpr std::string_view uniforms = util::trim(R"(
uniform sampler3D linearAtlas;
uniform bool useAtlasBoundary;
uniform bool applyBoundaryLight;
uniform bool showHighlighted; 
uniform bool showSelected;
uniform bool showFiltered;
uniform float gradientSpacingScale;
uniform vec4 selectedColor;
uniform vec4 highlightedColor;
uniform vec4 filteredColor;
)");

constexpr std::string_view setup = util::trim(R"(
VolumeParameters scaledAtlasParameters = atlasParameters;
scaledAtlasParameters.textureSpaceGradientSpacing *= gradientSpacingScale;
scaledAtlasParameters.worldSpaceGradientSpacing *= gradientSpacingScale;
)");

constexpr std::string_view first = util::trim(R"(
uint boundaryValue = 0;
uint prevBoundaryValue = 0;
)");

constexpr std::string_view loop = util::trim(R"(
prevBoundaryValue = boundaryValue;
boundaryValue = uint(atlasSegment*3.0f + 0.5f);

if(useAtlasBoundary && (boundaryValue != prevBoundaryValue))
{
    vec4 boundaryColor = vec4(0);
    if(showHighlighted && (boundaryValue == 3 || prevBoundaryValue == 3))
    {
        boundaryColor = highlightedColor;
    }
    else if(showSelected && (boundaryValue == 1 || prevBoundaryValue == 1))
    {
        boundaryColor = selectedColor;
    }
    else if(showFiltered && (boundaryValue == 2 || prevBoundaryValue == 2))
    {
        boundaryColor = filteredColor;
    }
    #if defined(SHADING_ENABLED) && defined(GRADIENTS_ENABLED)
    if(applyBoundaryLight)
    {
        vec3 atlasGradient = vec3(1);
        atlasGradient = normalize(gradientCentralDiff(vec4(0),
            linearAtlas, scaledAtlasParameters, samplePosition, channel));
        vec3 worldSpacePosition = 
            (atlasParameters.textureToWorldNormalMatrix * vec4(samplePosition, 1.0)).xyz;
        boundaryColor.rgb = APPLY_LIGHTING(lighting, boundaryColor.rgb, boundaryColor.rgb,
            vec3(1.0), worldSpacePosition, -atlasGradient, cameraDir);  
    }   
    #endif
    if(boundaryColor.a > 0)
    {
        boundaryColor.rgb *= boundaryColor.a; 
        result += (1.0f - result.a) * boundaryColor;
    }
}
)");

}  // namespace

auto SegmentSurfaceComponent::getSegments() -> std::vector<Segment> {
    using namespace fmt::literals;

    return {{R"(#include "utils/compositing.glsl")", placeholder::include, 900},
            {std::string(uniforms), placeholder::uniform, 900},
            {std::string(setup), placeholder::setup, 900},
            {std::string(first), placeholder::first, 900},
            {std::string(loop), placeholder::loop, 900}};
}
}  // namespace inviwo
