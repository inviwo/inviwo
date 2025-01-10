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
#pragma once

#include <modules/basegl/baseglmoduledefine.h>

#include <inviwo/core/properties/boolcompositeproperty.h>
#include <modules/fontrendering/properties/fontfaceoptionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <modules/fontrendering/textrenderer.h>

#include <inviwo/core/ports/datainport.h>

namespace inviwo {

class Shader;
class TextureUnitContainer;
class Texture2D;

class IVW_MODULE_BASEGL_API UniformLabelAtlasGL {
public:
    UniformLabelAtlasGL();

    void bind(TextureUnitContainer& cont);

    void addDefines(Shader& shader) const;
    void setUniforms(Shader& shader) const;
    void update();

    void fillAltas();
    bool createAltas();

    size2_t charSize;
    size2_t atlasSize;
    DataInport<std::vector<std::string>> strings;
    BoolCompositeProperty labels;
    FontFaceOptionProperty font;
    IntProperty fontSize;
    FloatVec4Property color;
    FloatProperty size;
    std::shared_ptr<Texture2D> atlas;
    TextRenderer renderer;
    GLint unitNumber;

    static constexpr size2_t chars = {3, 1};
    static constexpr size2_t margin = {2, 2};
};

}  // namespace inviwo
