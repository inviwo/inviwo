/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2024 Inviwo Foundation
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

#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/selectioncolorproperty.h>

#include <modules/opengl/texture/buffertexture.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>  // for BrushingAndLinkin...
#include <modules/basegl/datastructures/meshshadercache.h>

namespace inviwo {

class Shader;
class TextureUnitContainer;

class IVW_MODULE_BASEGL_API MeshBnLGL {
public:
    MeshBnLGL();

    /// return the smallest power of two not smaller than \p v
    static constexpr std::uint32_t bit_ceil(std::uint32_t v) noexcept {
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v++;
        return v;
    }

    void update();
    void bind(TextureUnitContainer& cont);
    void setUniforms(Shader& shader) const;
    MeshShaderCache::Requirement getRequirement() const;

    BrushingAndLinkingInport inport;
    SelectionColorProperty highlight;
    SelectionColorProperty select;
    SelectionColorProperty filter;
    BufferTexture<std::uint8_t, GL_R8UI> buffer;
    GLint unitNumber;
};

}  // namespace inviwo
