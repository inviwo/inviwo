/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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

#include <modules/basegl/shadercomponents/positionindicatorcomponent.h>

#include <inviwo/core/properties/planeproperty.h>             // for PlaneProperty
#include <inviwo/core/properties/volumeindicatorproperty.h>   // for VolumeIndicatorProperty
#include <inviwo/core/util/stringconversion.h>                // for StrBuffer, trim
#include <modules/basegl/shadercomponents/shadercomponent.h>  // for ShaderComponent::Segment
#include <modules/opengl/shader/shaderutils.h>                // for setUniforms

#include <string>       // for string
#include <string_view>  // for string_view

#include <fmt/core.h>  // for format, basic_string_view

namespace inviwo {
class Property;
class Shader;
class TextureUnitContainer;

PositionIndicatorComponent::PositionIndicatorComponent()
    : ShaderComponent(), positionIndicator_("positionindicator", "Position Indicator") {}

std::string_view PositionIndicatorComponent::getName() const {
    return positionIndicator_.getIdentifier();
}

void PositionIndicatorComponent::process(Shader& shader, TextureUnitContainer&) {
    utilgl::setUniforms(shader, positionIndicator_);
}

std::vector<Property*> PositionIndicatorComponent::getProperties() { return {&positionIndicator_}; }

namespace {

constexpr std::string_view uniforms = util::trim(R"(
uniform VolumeIndicatorParameters {};
)");

constexpr std::string_view code = util::trim(R"(
result = drawPlanes(result, samplePosition, rayDirection, rayStep, 
                    {}.plane{}, rayPosition, rayDepth);
)");

}  // namespace
auto PositionIndicatorComponent::getSegments() -> std::vector<Segment> {
    StrBuffer buff;

    if (positionIndicator_) {
        if (positionIndicator_.plane1_) buff.append(code, getName(), 1);
        if (positionIndicator_.plane2_) buff.append(code, getName(), 2);
        if (positionIndicator_.plane3_) buff.append(code, getName(), 3);

        return {{R"(#include "utils/raycastgeometry.glsl")", placeholder::include, 1100},
                {fmt::format(uniforms, getName()), placeholder::uniform, 1100},
                {std::string{buff}, placeholder::first, 1100},
                {std::string{buff}, placeholder::loop, 1100}};
    } else {
        return {};
    }
}

}  // namespace inviwo
