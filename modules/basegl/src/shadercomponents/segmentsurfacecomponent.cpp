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

#include <modules/opengl/shader/shaderutils.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <modules/opengl/volume/volumeutils.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/volume/volumegl.h>
#include <inviwo/core/ports/volumeport.h>

#include <inviwo/core/datastructures/buffer/bufferram.h>

namespace inviwo {
SegmentSurfaceComponent::SegmentSurfaceComponent(VolumeInport& atlas)
    : ShaderComponent{}
    , name_("atlasboundary")
    , useAtlasBoundary_{"useAtlasBoundary", "Render ISO using atlas boundary", true}
    , applyBoundaryLight_{"applyBoundaryLight", "Apply lighting to boundary", true}
    , showHighlighted_("showHighlightedIndices", "Show Highlighted", true,
                       vec3(1.0f, 0.906f, 0.612f))
    , showSelected_("showSelectedIndices", "Show Selected", true, vec3(1.0f, 0.769f, 0.247f))
    , showFiltered_("showFilteredIndices", "Show Filtered", true, vec3(0.5f, 0.5f, 0.5f))
    , textureSpaceGradientSpacingScale_{"gradientScaling", "Scale gradient spacing", 1.0f, 0.0f,
                                        5.0f}
    , nearestSampler_{}
    , linearSampler_{}
    , atlas_{atlas} {

    useAtlasBoundary_.addProperties(applyBoundaryLight_, textureSpaceGradientSpacingScale_,
                                    showHighlighted_, showFiltered_, showSelected_);


    nearestSampler_.setFilterModeAll(GL_NEAREST);
    nearestSampler_.setWrapModeAll(GL_REPEAT);

    linearSampler_.setFilterModeAll(GL_LINEAR);
    linearSampler_.setWrapModeAll(GL_REPEAT);
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
    //linearSampler_.bind(linearUnit.getUnitNumber());
    shader.setUniform("linearAtlas", linearUnit);
    utilgl::bindTexture(atlas_, linearUnit);
}
std::vector<Property*> SegmentSurfaceComponent::getProperties() { return {&useAtlasBoundary_}; }

namespace {
constexpr std::string_view uniforms = util::trim(R"(
uniform sampler3D linearAtlas;
uniform bool useAtlasBoundary;
uniform bool applyBoundaryLight;
uniform float gradientSpacingScale;
uniform vec4 selectedColor;
uniform vec4 highlightedColor;
uniform vec4 filteredColor;
uniform bool showHighlighted; 
uniform bool showSelected;
uniform bool showFiltered;
)");

constexpr std::string_view first = util::trim(R"(
uint boundaryValue = 0;
uint prevBoundaryValue = 0;
)");

constexpr std::string_view loop = util::trim(R"(
prevBoundaryValue = boundaryValue;
boundaryValue = uint(atlasSegment*3.0f + 0.5f);
if(!useAtlasBoundary)
{
    // Do nothing
}
else if(useAtlasBoundary && (boundaryValue != prevBoundaryValue))
{
    vec4 bcolor = vec4(0);
    if(showHighlighted && (boundaryValue == 3 || prevBoundaryValue == 3))
    {
        bcolor = highlightedColor;
    }
    else if(showSelected && (boundaryValue == 1 || prevBoundaryValue == 1))
    {
        bcolor = selectedColor;
    }
    else if(showFiltered && (boundaryValue == 2 || prevBoundaryValue == 2))
    {
        bcolor = filteredColor;
    }
    #if defined(SHADING_ENABLED) && defined(GRADIENTS_ENABLED)
    if(applyBoundaryLight)
    {
        vec4 atlasGradient = vec4(1);
        VolumeParameters tempAtlasParameters = atlasParameters;
        tempAtlasParameters.textureSpaceGradientSpacing *= gradientSpacingScale; //setup
        tempAtlasParameters.worldSpaceGradientSpacing *= gradientSpacingScale; // setup
        atlasGradient.rgb = normalize(COMPUTE_GRADIENT_FOR_CHANNEL(getNormalizedVoxel(atlas, atlasParameters, samplePosition), linearAtlas, tempAtlasParameters, samplePosition, channel));
     
        vec3 worldSpacePosition = (atlasParameters.textureToWorldNormalMatrix * vec4(samplePosition, 1.0)).xyz;
        bcolor.rgb = APPLY_LIGHTING(lighting, bcolor.rgb, bcolor.rgb, vec3(1.0), worldSpacePosition, -atlasGradient.rgb, cameraDir);  
    }   
    #endif
    if(bcolor.a > 0)
    {
        bcolor.rgb *= bcolor.a; 
        result += (1.0f - result.a) * bcolor;
    }
}
)");

}  // namespace

auto SegmentSurfaceComponent::getSegments() -> std::vector<Segment> {
    using namespace fmt::literals;

    return {{R"(#include "utils/compositing.glsl")", placeholder::include, 900},
            {std::string(uniforms), placeholder::uniform, 900},
            {std::string(first), placeholder::first, 900},
            {std::string(loop), placeholder::loop, 900}};
}
}  // namespace inviwo
