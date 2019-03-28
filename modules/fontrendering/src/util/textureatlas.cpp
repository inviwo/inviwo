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

#include <modules/fontrendering/util/textureatlas.h>

#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/openglutils.h>

#include <inviwo/core/util/zip.h>

#include <array>
#include <algorithm>
#include <numeric>
#include <cmath>

namespace inviwo {

namespace util {

bool TextureAtlas::valid() const { return (atlasTex_ != nullptr); }

void TextureAtlas::clear() { atlasTex_ = nullptr; }

void TextureAtlas::fillAtlas(TextRenderer& textRenderer, std::vector<TexAtlasEntry>& entries) {

    std::vector<TextBoundingBox> bboxes;
    std::transform(entries.begin(), entries.end(), std::back_inserter(bboxes),
                   [&](const auto& s) { return textRenderer.computeBoundingBox(s.value); });

    fillAtlas(textRenderer, entries, bboxes);
}

void TextureAtlas::fillAtlas(TextRenderer& textRenderer, const std::vector<std::string>& strings,
                             vec4 color) {

    std::vector<TextBoundingBox> bboxes;
    std::transform(strings.begin(), strings.end(), std::back_inserter(bboxes),
                   [&](const std::string& s) { return textRenderer.computeBoundingBox(s); });

    std::vector<TexAtlasEntry> entries;
    for (auto&& item : util::zip(strings, bboxes)) {
        entries.push_back({item.first(), ivec2{0}, item.second().glyphsExtent, color});
    }

    fillAtlas(textRenderer, entries, bboxes);
}

void TextureAtlas::fillAtlas(TextRenderer& textRenderer, std::vector<TexAtlasEntry>& entries,
                             std::vector<TextBoundingBox>& bboxes) {

    const auto minArea = std::accumulate(
        bboxes.begin(), bboxes.end(), 0.0, [&](double sum, const TextBoundingBox& bbox) {
            return sum + (bbox.glyphsExtent.x + margin_) * (bbox.glyphsExtent.y + margin_);
        });

    if (minArea > static_cast<double>(maxTexSize_ * maxTexSize_)) {
        throw Exception("Max size for texture atlas exceeded");
    }

    std::vector<size_t> indices(bboxes.size());
    std::iota(indices.begin(), indices.end(), 0u);

    // sort labels according to width then height
    std::sort(indices.begin(), indices.end(), [&](auto a, auto b) {
        const auto& extA(bboxes[a].glyphsExtent);
        const auto& extB(bboxes[b].glyphsExtent);

        return ((extA.x > extB.x) || ((extA.x == extA.x) && (extA.y > extA.y)));
    });

    const auto texSize = calcTexLayout(indices, entries, minArea);

    if (!atlasTex_) {
        atlasTex_ = std::make_shared<Texture2D>(static_cast<size2_t>(texSize), GL_RGBA, GL_RGBA,
                                                GL_UNSIGNED_BYTE, GL_LINEAR);
        atlasTex_->initialize(nullptr);

    } else if (glm::any(glm::lessThan(atlasTex_->getDimensions(), size2_t(texSize)))) {
        atlasTex_->resize(texSize);
    }

    initAtlas(textRenderer, entries, std::move(bboxes));
}

std::shared_ptr<Texture2D> TextureAtlas::getTexture() const { return atlasTex_; }

const TexAtlasRenderInfo& TextureAtlas::getRenderInfo() const { return renderInfo_; }

ivec2 TextureAtlas::calcTexLayout(const std::vector<size_t> indices,
                                  std::vector<TexAtlasEntry>& entries, double minArea) {
    if (entries.empty()) return {1, 1};

    const int width = [&](int w) {
        const auto maxEntryWidth = entries[indices.front()].texExtent.x;
        const int minWidth = static_cast<int>(std::sqrt(minArea));
        while ((w < minWidth || w < maxEntryWidth) && w < maxTexSize_) {
            w *= 2;
        }
        return w;
    }(32);

    std::vector<int> lineLengths;
    std::vector<int> lineHeights;
    lineLengths.push_back(0);
    lineHeights.push_back(0);
    int conservativeHeight = 0;
    // Fill each line by putting each element after the previous one.
    // If an element does not fit, start new line.
    for (auto i : indices) {
        const auto& extent = entries[i].texExtent + 2 * margin_;
        size_t line = 0;
        while (line < lineLengths.size()) {
            if (lineLengths[line] + extent.x < width) {
                // found a position with enough space, for now we only know the x coord,
                // use y component to store current line
                entries[i].texPos = ivec2(lineLengths[line] + margin_, line);
                lineLengths[line] += extent.x;
                lineHeights[line] = std::max(extent.y, lineHeights[line]);
                break;
            }
            ++line;
        }
        if (line == lineLengths.size()) {
            // no space found, create new line
            entries[i].texPos = ivec2(margin_, line);
            lineLengths.push_back(extent.x);
            lineHeights.push_back(extent.y);
            conservativeHeight += extent.y;
            if (conservativeHeight > maxTexSize_) {
                throw Exception("Max size for texture atlas exceeded");
            }
        }
    }
    // update y positions of all elements
    std::partial_sum(lineHeights.begin(), lineHeights.end(), lineHeights.begin());
    lineHeights.insert(lineHeights.begin(), 0);
    for (auto& elem : entries) {
        elem.texPos.y = lineHeights[elem.texPos.y] + margin_;
    }

    const int height = [&](int h) {
        while (h < lineHeights.back()) {
            h *= 2;
        }
        return h;
    }(32);

    if (height > maxTexSize_) {
        throw Exception("Max size for texture atlas exceeded");
    }
    return {width, height};
}

void TextureAtlas::initAtlas(TextRenderer& textRenderer, const std::vector<TexAtlasEntry>& entries,
                             std::vector<TextBoundingBox>&& bboxes) {
    // render labels into texture
    {
        utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        textRenderer.renderToTexture(atlasTex_, entries);
    }

    // prepare render info for all elements of the atlas
    // compute transformation matrix to map texCoords [0,1] to atlas entry
    renderInfo_.texTransform.clear();
    renderInfo_.texTransform.reserve(entries.size());
    const vec2 texDim(atlasTex_->getDimensions());

    std::transform(entries.begin(), entries.end(), std::back_inserter(renderInfo_.texTransform),
                   [&](auto& elem) {
                       mat4 m(glm::scale(vec3(vec2(elem.texExtent) / texDim, 1.0f)));
                       m[3] = vec4(vec2(elem.texPos) / texDim, 0.0f, 1.0f);
                       return m;
                   });

    renderInfo_.size.clear();
    renderInfo_.size.reserve(entries.size());
    std::transform(entries.begin(), entries.end(), std::back_inserter(renderInfo_.size),
                   [](auto& a) { return a.texExtent; });

    renderInfo_.boundingBoxes = std::move(bboxes);
}

}  // namespace util

}  // namespace inviwo
