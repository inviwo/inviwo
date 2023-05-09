/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
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

#include <modules/basegl/util/uniformlabelatlasgl.h>

#include <inviwo/core/util/glmvec.h>
#include <glm/gtx/scalar_multiplication.hpp>

#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/glformats.h>

namespace inviwo {

UniformLabelAtlasGL::UniformLabelAtlasGL()
    : labels{"labels", "Enable Labels", false}
    , font{"font", "Font", font::FontType::Default, InvalidationLevel::InvalidResources}
    , fontSize{"fontSize", "Font Size",
               util::ordinalCount<int>(14, 144)
                   .set(PropertySemantics{"Fontsize"})
                   .set(InvalidationLevel::InvalidResources)}
    , color{"color", "Color", util::ordinalColor(vec4(0.1f, 0.1f, 0.1f, 1.0f))}
    , size{"size", "Size", util::ordinalLength(0.3f, 1.0f)}
    , aspect{1.0f}
    , atlas{}
    , renderer{}
    , unitNumber{0} {

    labels.addProperties(font, color, size, fontSize);
    labels.getBoolProperty()->setInvalidationLevel(InvalidationLevel::InvalidResources);
}

void UniformLabelAtlasGL::initializeResources() {
    if (!labels) return;

    renderer.setFont(font.getSelectedValue());
    renderer.setFontSize(fontSize.get());

    size2_t charSize{0};
    for (int i = 0; i < 10; ++i) {
        charSize = glm::max(charSize, renderer.computeBoundingBox(fmt::to_string(i)).glyphsExtent);
    }

    const auto labelSize = charSize * size2_t{3, 1} + size2_t{2, 2};
    aspect = float(labelSize.y) / float(labelSize.x);
    const auto atlasSize = size2_t{30, 30};
    if (!atlas || labelSize * atlasSize != atlas->getDimensions()) {
        atlas = std::make_shared<Texture2D>(labelSize * atlasSize,
                                            GLFormats::get(DataVec4UInt8::id()), GL_LINEAR);
        atlas->initialize(nullptr);
    }

    utilgl::BlendModeState blendModeStateGL(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    {
        auto state = renderer.setupRenderState(atlas, vec4{0.0});
        for (size_t i = 0; i < atlasSize.y; ++i) {
            for (size_t j = 0; j < atlasSize.x; ++j) {
                auto str = fmt::to_string(i * atlasSize.x + j);
                const auto ttb = renderer.computeBoundingBox(str);
                const size2_t offset = (labelSize - ttb.glyphsExtent) / 2;
                const auto pos = static_cast<ivec2>(labelSize * size2_t{i, j} + offset);
                renderer.render(ttb, pos, str, vec4{1, 1, 1, 1});
            }
        }
    }
}

void UniformLabelAtlasGL::bind(TextureUnitContainer& cont) {
    if (labels) {
        auto& unit = cont.emplace_back();
        utilgl::bindTexture(*atlas, unit);
        unitNumber = unit.getUnitNumber();
    } else {
        unitNumber = 0;
    }
}

void UniformLabelAtlasGL::addDefines(Shader& shader) const {
    shader[ShaderType::Fragment]->setShaderDefine("ENABLE_LABELS", labels);
}
void UniformLabelAtlasGL::setUniforms(Shader& shader) const {
    StrBuffer buff;
    shader.setUniform("label.aspect", aspect);
    shader.setUniform(buff.replace("label.{}", color.getIdentifier()), color.get());
    shader.setUniform(buff.replace("label.{}", size.getIdentifier()), size.get());
    shader.setUniform("label.tex", unitNumber);
}

}  // namespace inviwo
