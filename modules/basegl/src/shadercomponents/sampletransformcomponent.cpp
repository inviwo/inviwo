/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2023 Inviwo Foundation
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

#include <modules/basegl/shadercomponents/sampletransformcomponent.h>

#include <inviwo/core/properties/compositeproperty.h>         // for CompositeProperty
#include <inviwo/core/properties/constraintbehavior.h>        // for ConstraintBehavior, Constra...
#include <inviwo/core/properties/ordinalproperty.h>           // for FloatVec3Property, IntVec3P...
#include <inviwo/core/util/glmvec.h>                          // for ivec3, vec3
#include <inviwo/core/util/stringconversion.h>                // for trim
#include <modules/basegl/shadercomponents/shadercomponent.h>  // for ShaderComponent::Segment
#include <modules/opengl/shader/shaderutils.h>                // for setUniforms

#include <string>  // for string

#include <fmt/core.h>    // for format
#include <fmt/format.h>  // for operator""_a, udl_arg, lite...

namespace inviwo {
class Property;
class Shader;
class TextureUnitContainer;

SampleTransformComponent::SampleTransformComponent()
    : ShaderComponent()
    , sampleTransform_{"sampleTransform", "Sample Transform"}
    , shift_{"shift",
             "Shift",
             vec3{0.0f},
             {vec3{-1.0f}, ConstraintBehavior::Ignore},
             {vec3{1.0f}, ConstraintBehavior::Ignore}}
    , repeat_{"repeat",
              "Repeat",
              ivec3{1},
              {ivec3{1}, ConstraintBehavior::Immutable},
              {ivec3{10}, ConstraintBehavior::Ignore}} {

    sampleTransform_.addProperties(shift_, repeat_);
}

std::string_view SampleTransformComponent::getName() const { return shift_.getIdentifier(); }

void SampleTransformComponent::process(Shader& shader, TextureUnitContainer&) {
    utilgl::setUniforms(shader, shift_, repeat_);
}

std::vector<Property*> SampleTransformComponent::getProperties() { return {&sampleTransform_}; }

namespace {

constexpr std::string_view uniforms = util::trim(R"(
uniform vec3 {shift};
uniform ivec3 {repeat};
)");

constexpr std::string_view first = util::trim(R"(
samplePosition *= {repeat};
samplePosition += {shift};
)");

}  // namespace

auto SampleTransformComponent::getSegments() -> std::vector<Segment> {
    using namespace fmt::literals;

    return {{fmt::format(uniforms, "shift"_a = shift_.getIdentifier(),
                         "repeat"_a = repeat_.getIdentifier()),
             placeholder::uniform, 300},
            {fmt::format(first, "shift"_a = shift_.getIdentifier(),
                         "repeat"_a = repeat_.getIdentifier()),
             placeholder::first, 300},
            {fmt::format(first, "shift"_a = shift_.getIdentifier(),
                         "repeat"_a = repeat_.getIdentifier()),
             placeholder::loop, 300}};
}

}  // namespace inviwo
