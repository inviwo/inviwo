/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifndef IVW_TEXTUREATLAS_H
#define IVW_TEXTUREATLAS_H

#include <modules/fontrendering/fontrenderingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/fontrendering/textrenderer.h>
#include <modules/fontrendering/datastructures/texatlasentry.h>

namespace inviwo {

class Texture2D;

namespace util {

/**
 * \class TextureAtlas
 * \brief Texture atlas for a number of strings rendered with the TextRenderer
 *
 * \see TextRenderer
 */
class IVW_MODULE_FONTRENDERING_API TextureAtlas {
public:
    TextureAtlas() = default;
    virtual ~TextureAtlas() = default;

    bool valid() const;

    void clear();

    void fillAtlas(TextRenderer& textRenderer, std::vector<TexAtlasEntry>& atlas);
    void fillAtlas(TextRenderer& textRenderer, const std::vector<std::string>& strings, vec4 color);

    void fillAtlas(TextRenderer& textRenderer, std::vector<TexAtlasEntry>& entries,
                   std::vector<TextBoundingBox>& bboxes);

    std::shared_ptr<Texture2D> getTexture() const;
    const TexAtlasRenderInfo& getRenderInfo() const;

private:
    /**
     * \brief figure out texture atlas size to fit all entries given a specific width
     * using the Shelf First Fit algorithm. This function also updates the element positions
     * within the new atlas texture
     *
     * @param indices ordered indices of atlas entries
     * @param width   initial texture width
     * @param margin  margin around each entry in texel
     * @return minimum texture size required to accommodate atlas
     */
    ivec2 calcTexLayout(const std::vector<size_t> indices, std::vector<TexAtlasEntry>& entries,
                        double minArea);

    void initAtlas(TextRenderer& textRenderer, const std::vector<TexAtlasEntry>& entries,
                   std::vector<TextBoundingBox>&& bboxes);

    std::shared_ptr<Texture2D> atlasTex_;
    TexAtlasRenderInfo renderInfo_;
    static constexpr int maxTexSize_ = 8192;
    static constexpr int margin_ = 2;
};

}  // namespace util

}  // namespace inviwo

#endif  // IVW_TEXTUREATLAS_H
