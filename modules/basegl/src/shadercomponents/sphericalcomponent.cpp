/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2025 Inviwo Foundation
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

#include <modules/basegl/shadercomponents/sphericalcomponent.h>

#include <inviwo/core/properties/invalidationlevel.h>         // for InvalidationLevel, Invalida...
#include <inviwo/core/properties/minmaxproperty.h>            // for FloatMinMaxProperty
#include <inviwo/core/properties/optionproperty.h>            // for OptionPropertyString
#include <inviwo/core/util/stringconversion.h>                // for trim
#include <modules/basegl/shadercomponents/shadercomponent.h>  // for ShaderComponent::Segment
#include <modules/opengl/shader/shaderutils.h>                // for setUniforms

#include <string>  // for basic_string, string

#include <fmt/core.h>    // for format
#include <fmt/format.h>  // for operator""_a, udl_arg, lite...

namespace inviwo {
class Property;
class Shader;
class TextureUnitContainer;

SphericalComponent::SphericalComponent()
    : ShaderComponent()
    , comp0_{"comp0", "Component 1", {"r", "theta", "phi"}, 0, InvalidationLevel::InvalidResources}
    , comp1_{"comp1", "Component 2", {"r", "theta", "phi"}, 1, InvalidationLevel::InvalidResources}
    , comp2_{"comp2", "Component 3", {"r", "theta", "phi"}, 2, InvalidationLevel::InvalidResources}
    , rRange_{"rRange", "Radial range", 0, 1, 0, 100} {}

std::string_view SphericalComponent::getName() const { return "Spherical"; }

void SphericalComponent::process(Shader& shader, TextureUnitContainer&) {
    utilgl::setUniforms(shader, rRange_);
}

std::vector<Property*> SphericalComponent::getProperties() {
    return {&comp0_, &comp1_, &comp2_, &rRange_};
}

namespace {

constexpr std::string_view uniforms = util::trim(R"(
uniform vec2 {rRange};
#define VOLUME_PI      3.14159265358979323846  /* pi */
#define VOLUME_SQRT1_3 0.57735026919           /* 1/sqrt(3) */
vec3 c2s(vec3 zeroToOneCoords) {{
    // Put cartesian in [-1..1] range first
    vec3 cartesian =  zeroToOneCoords * 2.0 - vec3(1.0);

    float r = length(cartesian);
    float theta = 0.0;
    float phi = 0.0;

    if (r != 0.0) {{
        theta = acos(cartesian.z / r) / VOLUME_PI;
        phi = (VOLUME_PI + atan(cartesian.y, cartesian.x)) / (2.0 * VOLUME_PI );
    }}

    r = (rRange.y-rRange.x)*r - rRange.x;    

    return vec3({x},{y},{z});
}}
)");

constexpr std::string_view first = util::trim(R"(
samplePosition = c2s(samplePosition);
)");

}  // namespace

auto SphericalComponent::getSegments() -> std::vector<Segment> {
    using namespace fmt::literals;

    return {
        {fmt::format(uniforms, "x"_a = comp0_.getSelectedValue(), "y"_a = comp1_.getSelectedValue(),
                     "z"_a = comp2_.getSelectedValue(), "rRange"_a = rRange_.getIdentifier()),
         placeholder::uniform, 280},
        {std::string{first}, placeholder::first, 280},
        {std::string{first}, placeholder::loop, 280}};
}

}  // namespace inviwo
