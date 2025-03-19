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

#include <modules/basegl/util/meshtexturing.h>

#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureutils.h>

namespace inviwo {

MeshTexturing::MeshTexturing(std::string_view identifier, Document help)
    : inport{identifier, std::move(help), OutportDeterminesSize::Yes}
    , unitNumber{0}
    , texture{"texture", "Enable Texture", false}
    , blendMode{"blendMode",
                "Blend Mode",
                {{"source", "Source Blending", BlendMode::Source},
                 {"destination", "Destination Blending", BlendMode::Destination},
                 {"alpha", "Alpha Blending", BlendMode::Alpha},
                 {"additive", "Additive Blending", BlendMode::Additive},
                 {"multiply", "Multiplicative Blending", BlendMode::Multiply},
                 {"screen", "Screen Blending", BlendMode::Screen},
                 {"subtractive", "Subtractive Blending", BlendMode::Subtractive},
                 {"premultiplied", "Premultiplied Alpha Blending", BlendMode::Premultiplied},
                 {"overlay", "Overlay Blending", BlendMode::Overlay}},
                2,
                InvalidationLevel::InvalidResources}
    , swap{"swap", "Swap src, dst", false, InvalidationLevel::InvalidResources}
    , mix{"textureMixing",
          "Mixing",
          "Blending factor for mixing the texture with the item's original color."_help,
          0.7f,
          {0.0f, ConstraintBehavior::Immutable},
          {1.0f, ConstraintBehavior::Immutable},
          0.001f} {

    texture.getBoolProperty()->setInvalidationLevel(InvalidationLevel::InvalidResources);
    texture.addProperties(blendMode, swap, mix);
}

void MeshTexturing::bind(TextureUnitContainer& cont) {
    if (inport.hasData()) {
        auto& unit = cont.emplace_back();
        utilgl::bindColorTexture(*inport.getData(), unit);
        unitNumber = unit.getUnitNumber();
    } else {
        unitNumber = 0;
    }
}

void MeshTexturing::setUniforms(Shader& shader) const {
    shader.setUniform(inport.getIdentifier(), unitNumber);
    utilgl::setUniforms(shader, mix);
}

void MeshTexturing::addDefines(Shader& shader) const {
    const auto func = [&]() -> std::string_view {
        switch (blendMode.get()) {
            case BlendMode::Source:
                return "sourceBlend";
            case BlendMode::Destination:
                return "destinationBlend";
            case BlendMode::Alpha:
                return "alphaBlend";
            case BlendMode::Additive:
                return "additiveBlend";
            case BlendMode::Multiply:
                return "multiplyBlend";
            case BlendMode::Screen:
                return "screenBlend";
            case BlendMode::Subtractive:
                return "subtractiveBlend";
            case BlendMode::Premultiplied:
                return "premultipliedAlphaBlend";
            case BlendMode::Overlay:
                return "overlayBlend";
            default:
                throw Exception{SourceContext{}, "Invalid blend mode"};
        }
    }();
    shader.getFragmentShaderObject()->addShaderDefine(
        "TEXTURING_BLEND_FUNC(src, dst)",
        fmt::format("{}({}, {})", func, swap.get() ? "dst" : "src", swap.get() ? "src" : "dst"));
}

MeshShaderCache::Requirement MeshTexturing::getRequirement() const {
    return {[this](const Mesh&, Mesh::MeshInfo) -> int {
                return inport.hasData() && texture.isChecked() ? 1 : 0;
            },
            [](int mode, Shader& shader) {
                shader[ShaderType::Fragment]->setShaderDefine("ENABLE_TEXTURING", mode == 1);
            }};
}

}  // namespace inviwo
