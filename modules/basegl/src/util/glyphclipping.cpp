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

#include <modules/basegl/util/glyphclipping.h>

#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

GlyphClipping::GlyphClipping()
    : clipping{"clipping", "Clipping"}
    , mode{"mode",
           "Mode",
           "defines the handling of spheres clipped at the camera"_help,
           {{"discard", "Discard Glyph", Mode::Discard}, {"cut", "Cut Glypyh", Mode::Cut}},
           0,
           InvalidationLevel::InvalidResources}
    , shadingFactor{"clipShadingFactor", "Clip Surface Adjustment",
                    util::ordinalScale(0.9f, 2.0f)
                        .set("brighten/darken glyph color on clip surface"_help)}
    , shadeClippedArea{"shadeClippedArea", "Shade Clipped Area",
                       "enable illumination computations for the clipped surface"_help, false,
                       InvalidationLevel::InvalidResources} {

    clipping.addProperties(mode, shadingFactor, shadeClippedArea);
    shadingFactor.readonlyDependsOn(mode, [](const auto& p) { return p == Mode::Discard; });
    shadeClippedArea.readonlyDependsOn(mode, [](const auto& p) { return p == Mode::Discard; });
}

void GlyphClipping::addDefines(Shader& shader) const {
    shader[ShaderType::Fragment]->setShaderDefine("SHADE_CLIPPED_AREA", shadeClippedArea);
    shader[ShaderType::Fragment]->setShaderDefine("DISCARD_CLIPPED_GLYPHS",
                                                  mode.get() == Mode::Discard);
    shader[ShaderType::Geometry]->setShaderDefine("DISCARD_CLIPPED_GLYPHS",
                                                  mode.get() == Mode::Discard);
}
void GlyphClipping::setUniforms(Shader& shader) const {
    utilgl::setUniforms(shader, shadingFactor);
}

}  // namespace inviwo
