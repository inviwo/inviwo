/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#include <modules/fontrendering/util/textureatlas.h>

#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/openglutils.h>

#include <array>
#include <algorithm>
#include <numeric>

namespace inviwo {

namespace util {

bool TextureAtlas::valid() const {
    return (atlasTex_ != nullptr);
}

bool TextureAtlas::empty() const {
    return entries_.empty();
}

void TextureAtlas::clear() {
    atlasTex_ = nullptr;
    entries_.clear();
}

void TextureAtlas::fillAtlas(TextRenderer &textRenderer, std::vector<TexAtlasEntry>& atlas) {
    entries_ = atlas;

    std::vector<size_t> indices(entries_.size());
    std::iota(indices.begin(), indices.end(), 0u);

    // sort labels according to width then height
    std::sort(indices.begin(), indices.end(), [&](auto a, auto b) {
        const auto& extA(entries_[a].texExtent);
        const auto& extB(entries_[b].texExtent);

        return ((extA.x > extB.x) || ((extA.x == extA.x) && (extA.y > extA.y)));
    });

    const int maxTexSize = 8192;
    const int margin = 2;
    // begin with an initial texture width of 1024 texel
    int width = 1024;
    ivec2 texSize = calcTexLayout(indices, width, margin);
    // if the texture is too large, try again with wider atlas texture
    while (texSize.y > maxTexSize) {
        width *= 2;
        if (width > maxTexSize) {
            throw Exception(
                "Axis Renderer: axis labels too large or too many labels (max size for texture "
                "atlas exceeded)");
        }

        texSize = calcTexLayout(indices, width, margin);
    }

    if (!atlasTex_ || (size2_t(texSize) != atlasTex_->getDimensions())) {
        initTexture(size2_t(texSize));
    }

    initAtlas(textRenderer);
}

void TextureAtlas::initTexture(const size2_t& dims) {
    auto tex = std::make_shared<Texture2D>(dims, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_LINEAR);
    std::vector<unsigned char> data(dims.x * dims.y * 4, 0);
    tex->initialize(data.data());

    atlasTex_ = tex;
}

std::shared_ptr<Texture2D> TextureAtlas::getTexture() const {
    return atlasTex_;
}

const std::vector<TexAtlasEntry>& TextureAtlas::getEntries() const {
    return entries_;
}

const TexAtlasRenderInfo& TextureAtlas::getRenderInfo() const {
    return renderInfo_;
}

ivec2 TextureAtlas::calcTexLayout(const std::vector<size_t> indices, const int width, const int margin) {
    std::vector<int> lineLengths;
    std::vector<int> lineHeights;
    lineLengths.push_back(0);
    lineHeights.push_back(0);
    // Fill each line by putting each element after the previous one.
    // If an element does not fit, start new line.
    for (auto i : indices) {
        const auto& extent = entries_[i].texExtent + 2 * margin;
        size_t line = 0;
        while (line < lineLengths.size()) {
            if (lineLengths[line] + extent.x < width) {
                // found a position with enough space, for now we only know the x coord,
                // use y component to store current line
                entries_[i].texPos = ivec2(lineLengths[line] + margin, line);
                lineLengths[line] += extent.x;
                lineHeights[line] = std::max(extent.y, lineHeights[line]);
                break;
            }
            ++line;
        }
        if (line == lineLengths.size()) {
            // no space found, create new line
            entries_[i].texPos = ivec2(margin, line);
            lineLengths.push_back(extent.x);
            lineHeights.push_back(extent.y);
        }
    }
    // update y positions of all elements
    std::partial_sum(lineHeights.begin(), lineHeights.end(), lineHeights.begin());
    lineHeights.insert(lineHeights.begin(), 0);
    for (auto& elem : entries_) {
        elem.texPos.y = lineHeights[elem.texPos.y] + margin;
    }

    return ivec2(width, lineHeights.back());
}

void TextureAtlas::initAtlas(TextRenderer &textRenderer) {
    // render labels into texture       
    {
        utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        textRenderer.renderToTexture(atlasTex_, entries_);
    }

    // prepare render info for all elements of the atlas
    //
    // compute transformation matrix to map texCoords [0,1] to atlas entry
    renderInfo_.texTransform.clear();
    renderInfo_.texTransform.reserve(entries_.size());
    const vec2 texDim(atlasTex_->getDimensions());

    std::transform(entries_.begin(), entries_.end(),
                   std::back_inserter(renderInfo_.texTransform), [&](auto& elem) {
        mat4 m(glm::scale(vec3(vec2(elem.texExtent) / texDim, 1.0f)));
        m[3] = vec4(vec2(elem.texPos) / texDim, 0.0f, 1.0f);
        return m;
    });

    renderInfo_.size.clear();
    renderInfo_.size.reserve(entries_.size());
    std::transform(entries_.begin(), entries_.end(),
                   std::back_inserter(renderInfo_.size), [](auto& a) { return a.texExtent; });
}

} // namespace util

} // namespace inviwo
