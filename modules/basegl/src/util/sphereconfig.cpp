/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2025 Inviwo Foundation
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

#include <modules/basegl/util/sphereconfig.h>

#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {
SphereConfig::SphereConfig(std::string_view identifier, std::string_view displayName)
    : config{identifier, displayName}
    , overrideRadius{"overrideRadius", "Override Radius",
                     "enable a fixed user-defined radius for all spheres"_help, false,
                     InvalidationLevel::InvalidResources}
    , radius{"radius", "Default Radius",
             util::ordinalLength(0.05f, 2.0f)
                 .setMin(0.00001f)
                 .set("radius of the rendered spheres (in world coordinates)"_help)}
    , overrideColor{"overrideColor", "Override Color",
                    "if enabled, all spheres will share the same custom color"_help, false,
                    InvalidationLevel::InvalidResources}
    , color{"color", "Default Color",
            util::ordinalColor(vec3(0.7f, 0.7f, 0.7f))
                .set("custom color when overriding the input colors"_help)}
    , overrideAlpha("useUniformAlpha", "Override Alpha", false, InvalidationLevel::InvalidResources)
    , alpha("alpha", "Default Alpha", 1.0f, 0.0f, 1.0f, 0.1f)
    , useMetaColor{"useMetaColor", "Use meta color mapping", false,
                   InvalidationLevel::InvalidResources}
    , metaColor{"metaColor", "Meta Color Mapping"}
    , unitNumber{0} {

    config.addProperties(overrideRadius, radius, overrideColor, color, overrideAlpha, alpha,
                         useMetaColor, metaColor);
}

void SphereConfig::bind(TextureUnitContainer& cont) {
    if (useMetaColor) {
        auto& unit = cont.emplace_back();
        utilgl::bindTexture(metaColor, unit);
        unitNumber = unit.getUnitNumber();
    } else {
        unitNumber = 0;
    }
}

void SphereConfig::addDefines(Shader& shader) const {
    shader[ShaderType::Vertex]->setShaderDefine("OVERRIDE_COLOR", overrideColor);
    shader[ShaderType::Vertex]->setShaderDefine("OVERRIDE_ALPHA", overrideAlpha);
    shader[ShaderType::Vertex]->setShaderDefine("OVERRIDE_RADIUS", overrideRadius);
    shader[ShaderType::Vertex]->setShaderDefine("USE_SCALARMETACOLOR", useMetaColor);
}
void SphereConfig::setUniforms(Shader& shader) const {
    StrBuffer buff;
    shader.setUniform(buff.replace("config.{}", color.getIdentifier()), color.get());
    shader.setUniform(buff.replace("config.{}", alpha.getIdentifier()), alpha.get());
    shader.setUniform(buff.replace("config.{}", radius.getIdentifier()), radius.get());
    shader.setUniform(metaColor.getIdentifier(), unitNumber);
}

}  // namespace inviwo
