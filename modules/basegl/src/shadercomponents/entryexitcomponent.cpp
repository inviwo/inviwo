/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2022 Inviwo Foundation
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

#include <modules/basegl/shadercomponents/entryexitcomponent.h>

#include <inviwo/core/datastructures/image/imagetypes.h>                // for ImageType, ImageT...
#include <inviwo/core/datastructures/image/layer.h>                     // for Layer
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/ports/imageport.h>                                // for ImageInport
#include <inviwo/core/util/stringconversion.h>                          // for trim
#include <modules/basegl/shadercomponents/shadercomponent.h>            // for ShaderComponent::...
#include <modules/opengl/image/layergl.h>                               // for LayerGL
#include <modules/opengl/shader/shader.h>                               // for Shader
#include <modules/opengl/texture/texture2d.h>                           // for Texture2D
#include <modules/opengl/texture/textureutils.h>                        // for bindAndSetUniforms

#include <memory>         // for shared_ptr, uniqu...
#include <type_traits>    // for remove_extent_t
#include <unordered_set>  // for unordered_set

#include <fmt/core.h>    // for format
#include <fmt/format.h>  // for literals

namespace inviwo {
class Inport;
class TextureUnitContainer;

EntryExitComponent::EntryExitComponent()
    : ShaderComponent(), entryPort_("entry"), exitPort_("exit") {}

std::string_view EntryExitComponent::getName() const { return "entryexit"; }

void EntryExitComponent::process(Shader& shader, TextureUnitContainer& cont) {
    utilgl::bindAndSetUniforms(shader, cont, entryPort_, ImageType::ColorDepthPicking);
    utilgl::bindAndSetUniforms(shader, cont, exitPort_, ImageType::ColorDepthPicking);
    if (auto surfaceNormals = entryPort_.getData()->getColorLayer(1)) {
        utilgl::bindAndSetUniforms(shader, cont,
                                   *surfaceNormals->getRepresentation<LayerGL>()->getTexture(),
                                   std::string_view{"surfaceNormal"});
        shader.setUniform("useSurfaceNormals", true);
    } else {
        shader.setUniform("useSurfaceNormals", false);
    }
}

std::vector<std::tuple<Inport*, std::string>> EntryExitComponent::getInports() {
    return {{&entryPort_, std::string{"images"}}, {&exitPort_, std::string{"images"}}};
}

namespace {

constexpr std::string_view uniforms = util::trim(R"(
uniform ImageParameters {0}Parameters;
uniform sampler2D {0}Color;
uniform sampler2D {0}Depth;
)");

constexpr std::string_view surfaceNormalUniforms = util::trim(R"(
uniform sampler2D surfaceNormal;
uniform bool useSurfaceNormals;
)");

constexpr std::string_view setup = util::trim(R"(
vec3 entryPoint = texture(entryColor, texCoords).rgb;
vec3 exitPoint = texture(exitColor, texCoords).rgb;
float entryPointDepth = texture(entryDepth, texCoords).x;
float exitPointDepth = texture(exitDepth, texCoords).x;

// The length of the ray in texture space
float rayLength = length(exitPoint - entryPoint);

// The normalized direction of the ray
vec3 rayDirection = normalize(exitPoint - entryPoint);
)");

};  // namespace

auto EntryExitComponent::getSegments() -> std::vector<Segment> {
    using namespace fmt::literals;
    return {{fmt::format(uniforms, entryPort_.getIdentifier()), placeholder::uniform, 100},
            {fmt::format(uniforms, exitPort_.getIdentifier()), placeholder::uniform, 101},
            {std::string{surfaceNormalUniforms}, placeholder::uniform, 102},
            {std::string{setup}, placeholder::setup, 100}};
}

}  // namespace inviwo
