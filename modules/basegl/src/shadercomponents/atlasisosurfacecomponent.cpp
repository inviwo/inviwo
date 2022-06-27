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

#include <modules/basegl/shadercomponents/atlasisosurfacecomponent.h>

#include <modules/opengl/shader/shaderutils.h>
#include <inviwo/core/datastructures/volume/volume.h>

namespace inviwo {

atlasisosurfacecomponent::atlasisosurfacecomponent(Processor* p, std::string_view volume)
    : ShaderComponent()
    , atlas_{"atlas"}
    , volume_(volume)
    , showHighlighted_("showHighlightedIndices", "Show Highlighted", true,
                       vec3(1.0f, 0.906f, 0.612f))
    , showSelected_("showSelectedIndices", "Show Selected", true, vec3(1.0f, 0.769f, 0.247f))
    , showFiltered_("showFilteredIndices", "Show Filtered", true, vec3(0.5f, 0.5f, 0.5f))
    , sampleRate_{"sampleRate", "Sampling Rate", 2.0f,
                  std::pair{1.0f, ConstraintBehavior::Immutable},
                  std::pair{20.0f, ConstraintBehavior::Editable}} {}

std::string_view atlasisosurfacecomponent::getName() const { return atlas_.getIdentifier(); }

void atlasisosurfacecomponent::process(Shader& shader, TextureUnitContainer& cont) {
    utilgl::bindAndSetUniforms(shader, cont, atlas_);
    shader.setUniform("selectedColor", showSelected_.getColor());
    shader.setUniform("showSelected", showSelected_.getBoolProperty()->get());
    shader.setUniform("highlightedColor", showHighlighted_.getColor());
    shader.setUniform("showHighlighted", showHighlighted_.getBoolProperty()->get());
    shader.setUniform("filteredColor", showFiltered_.getColor());
    shader.setUniform("showFiltered", showFiltered_.getBoolProperty()->get());
}

void atlasisosurfacecomponent::initializeResources(Shader& shader) {
    auto fso = shader.getFragmentShaderObject();
}

std::vector<Property*> atlasisosurfacecomponent::getProperties() {
    return {&sampleRate_, &showHighlighted_, &showSelected_, &showFiltered_};
}

namespace {
constexpr std::string_view uniforms = util::trim(R"(
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
float v = getNormalizedVoxel({atlas}, {atlas}Parameters, samplePosition).x;
value = uint(v*3.0f + 0.5f);

if(boundaryValue != prevBoundaryValue)
{
    vec4 color = vec4(0);
    if(showHighlighted && (boundaryValue == 3 || prevBoundaryValue == 3))
    {
        color = highlightedColor;
    }
    else if(showSelected && (boundaryValue == 2 || prevBoundaryValue == 2))
    {
        color = selectedColor;
    }
    else if(showFiltered && (boundaryValue == 1 || prevBoundaryValue == 1))
    {
        color = filteredColor;
    }
    color.rgb *= color.a;
    result += (1.0f - result.a) * color;
}
)");

}  // namespace

auto atlasisosurfacecomponent::getSegments() -> std::vector<Segment> {
    using namespace fmt::literals;

    return {
        {fmt::format(uniforms, "atlas"_a = getName()), placeholder::uniform, 700},
        {fmt::format(first, "atlas"_a = getName(), "color"_a = volume_), placeholder::first, 700},
        {fmt::format(loop, "atlas"_a = getName(), "color"_a = volume_), placeholder::loop, 700}};
}

}  // namespace inviwo
