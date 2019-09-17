/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/zip.h>

#include <modules/opengl/buffer/buffergl.h>
#include <modules/opengl/buffer/bufferobjectarray.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/sharedopenglresources.h>

#include <modules/fontrendering/textrenderer.h>

#include <utf8.h>

namespace inviwo {

TextRenderer::TextRenderer(const std::string& fontPath)
    : fontface_(nullptr), fontSize_(10), lineSpacing_(0.2), glyphMargin_(2), shader_{getShader()} {

    if (FT_Init_FreeType(&fontlib_)) {
        throw Exception("Could not initialize FreeType library");
    }

    setFont(fontPath);

    fbo_.activate();
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    fbo_.deactivate();
}

TextRenderer::~TextRenderer() { FT_Done_Face(fontface_); }

void TextRenderer::setFont(const std::string& fontPath) {
    // free previous font face
    FT_Done_Face(fontface_);
    fontface_ = nullptr;

    int error = FT_New_Face(fontlib_, fontPath.c_str(), 0, &fontface_);
    if (error == FT_Err_Unknown_File_Format) {
        throw Exception(std::string("Unsupported font format: \"") + fontPath + "\"");
    } else if (error) {
        throw FileException(std::string("Could not open font file: \"") + fontPath + "\"");
    }

    FT_Select_Charmap(fontface_, ft_encoding_unicode);
    FT_Set_Pixel_Sizes(fontface_, 0, fontSize_);
}

std::string::const_iterator TextRenderer::validateString(const std::string& str) const {
    // check input string for invalid utf8 encoding, process only valid part
    auto end = utf8::find_invalid(str.begin(), str.end());
    if (end != str.end()) {
        LogWarn("Invalid UTF-8 encoding detected. This part is fine: " << std::string(str.begin(),
                                                                                      end));
    }
    return end;
}

TextBoundingBox TextRenderer::computeBoundingBox(const std::string& str) {
    if (str.empty()) return {};  // empty string, return empty bounding box

    // the pen position defines where the current glyph is positioned
    ivec2 penPos(0, getBaseLineOffset());

    // textual bounding box contains at least one line, calculate height of first line
    // For most fonts descender is negative (see FreeType documentation for details)
    ivec2 textBoxExtent(0, getBaseLineOffset() + std::max(-getBaseLineDescender(), 0));

    // glyph bounding box enclosing all individual glyph bounding boxes
    // start glyph bounding box at baseline
    ivec2 glyphsTopLeft(std::numeric_limits<int>::max(), getBaseLineOffset());
    ivec2 glyphsBottomRight(std::numeric_limits<int>::min());

    auto& fc = getFontCache();

    // the vertical offset is increased for each additional line
    int verticalOffset = 0;

    // check input string for invalid utf8 encoding
    auto end = validateString(str);

    for (utf8::iterator<std::string::const_iterator> it{str.begin(), str.begin(), end};
         it != utf8::iterator<std::string::const_iterator>{end, str.begin(), end}; ++it) {
        const uint32_t charCode = *it;

        // query font cache for glyph matching the character code
        auto p = requestGlyph(fc, charCode);
        if (!p.first) {
            // glyph not found, skip it
            penPos += p.second.advance;
            continue;
        }

        GlyphEntry& glyph = p.second;

        if (charCode == lf) {
            verticalOffset += getLineHeight();
            // reset pen position to begin of the next line
            penPos.x = 0;
            penPos.y += glyph.advance.y;
            continue;
        } else if (charCode == tab) {
            penPos += glyph.advance;
            penPos.x += (4 * glyph.size.x);  // 4 times glyph character width

            glyphsBottomRight.x = std::max(glyphsBottomRight.x, penPos.x);
            textBoxExtent.x = std::max(textBoxExtent.x, penPos.x);
            continue;
        }

        // compute top-left position of glyph based on current pen position
        ivec2 pos(penPos.x + glyph.bearing.x, verticalOffset + penPos.y - glyph.bearing.y);

        // update global glyphs bounding box
        glyphsTopLeft = glm::min(glyphsTopLeft, pos);
        glyphsBottomRight = glm::max(glyphsBottomRight, pos + glyph.size);

        // advance pen to next glyph
        penPos += glyph.advance;

        // textual bounding box only considers maximum pen position
        textBoxExtent.x = std::max(textBoxExtent.x, penPos.x);
    }

    // update vertical extent of textual bounding box
    textBoxExtent.y += verticalOffset;

    // determine glyphs bounding box relative to bottom left corner of text box
    ivec2 glyphsBottomLeft(glyphsTopLeft.x, textBoxExtent.y - glyphsBottomRight.y);
    ivec2 glyphsExtent(glyphsBottomRight - glyphsTopLeft);

    return {textBoxExtent, glyphsBottomLeft, glyphsExtent, getBaseLineOffset()};
}

void TextRenderer::render(const std::string& str, const vec2& posf, const vec2& scaling,
                          const vec4& color) {

    auto& fc = getFontCache();

    TextureUnit texUnit;
    texUnit.activate();
    fc.glyphTex->bind();

    const vec2 texDims(fc.glyphTex->getDimensions());

    shader_->activate();
    shader_->setUniform("tex", texUnit);
    shader_->setUniform("color", color);

    auto rect = SharedOpenGLResources::getPtr()->imagePlaneRect();
    utilgl::Enable<MeshGL> enable(rect);

    ivec2 glyphPos{0};
    int verticalOffset = 0;

    // check input string for invalid utf8 encoding
    auto end = validateString(str);

    for (utf8::iterator<std::string::const_iterator> it{str.begin(), str.begin(), end};
         it != utf8::iterator<std::string::const_iterator>{end, str.begin(), end}; ++it) {
        const uint32_t charCode = *it;

        auto p = requestGlyph(fc, charCode);
        if (!p.first) {
            // glyph not found, skip it
            glyphPos += p.second.advance;
            continue;
        }

        GlyphEntry& glyph = p.second;

        if (charCode == lf) {
            verticalOffset += getLineHeight();
            glyphPos.x = 0;
            glyphPos.y += glyph.advance.y;
            continue;
        } else if (charCode == tab) {
            glyphPos += glyph.advance;
            glyphPos.x += (4 * glyph.size.x);  // 4 times glyph character width
            continue;
        }

        // compute floating point position
        vec3 pos(posf, 0.f);
        pos.x += (glyphPos.x + glyph.bearing.x) * scaling.x;
        pos.y -= (verticalOffset - glyphPos.y - glyph.bearing.y) * scaling.y;

        // Translate quad to correct position and render
        const mat4 dataToWorld(
            glm::translate(pos) *
            glm::scale(vec3(glyph.size.x * scaling.x, -glyph.size.y * scaling.y, 1.f)));

        const mat4 texTransform(glm::translate(vec3(vec2(glyph.texAtlasPos) / texDims, 0.0f)) *
                                glm::scale(vec3(vec2(glyph.size) / texDims, 1.0f)));

        shader_->setUniform("geometry_.dataToWorld", dataToWorld);
        shader_->setUniform("texCoordTransform", texTransform);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glyphPos += glyph.advance;
    }
    shader_->deactivate();
}

void TextRenderer::render(const std::string& str, float x, float y, const vec2& scale,
                          const vec4& color) {
    render(str, vec2(x, y), scale, color);
}

void TextRenderer::renderToTexture(std::shared_ptr<Texture2D> texture, const std::string& str,
                                   const vec4& color, bool clearTexture) {
    renderToTexture(texture, size2_t(0u), texture->getDimensions(), str, color, clearTexture);
}

void TextRenderer::renderToTexture(const TextTextureObject& texObject, const std::string& str,
                                   const vec4& color, bool clearTexture) {
    renderToTexture(texObject, size2_t(0u), texObject.texture->getDimensions(), str, color,
                    clearTexture);
}

void TextRenderer::renderToTexture(std::shared_ptr<Texture2D> texture, const size2_t& origin,
                                   const size2_t& size, const std::string& str, const vec4& color,
                                   bool clearTexture) {
    renderToTexture({texture, computeBoundingBox(str)}, origin, size, str, color, clearTexture);
}

std::tuple<utilgl::DepthMaskState, utilgl::GlBoolState, utilgl::BlendModeState,
           utilgl::Activate<FrameBufferObject>>
TextRenderer::setupRenderState(std::shared_ptr<Texture2D> texture, bool clearTexture) {
    // disable depth test and writing depth
    utilgl::DepthMaskState depthMask(GL_FALSE);
    utilgl::GlBoolState depth(GL_DEPTH_TEST, GL_FALSE);

    utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    utilgl::Activate<FrameBufferObject> fbo(&fbo_);

    if (prevTexture_ != texture) {
        // detach previous texture and attach new texture as a render target, no depth texture
        fbo_.detachTexture(GL_COLOR_ATTACHMENT0);
        fbo_.attachTexture(texture.get(), GL_COLOR_ATTACHMENT0);
        prevTexture_ = texture;
    }
    if (clearTexture) {
        glClear(GL_COLOR_BUFFER_BIT);
    }

    return std::make_tuple(std::move(depthMask), std::move(depth), std::move(blending),
                           std::move(fbo));
}

void TextRenderer::renderToTexture(const TextTextureObject& texObject, const size2_t& origin,
                                   const size2_t& size, const std::string& str, const vec4& color,
                                   bool clearTexture) {
    const auto state = setupRenderState(texObject.texture, clearTexture);

    // set up viewport
    ivec2 pos(origin);
    ivec2 dim(size);
    utilgl::ViewportState viewport(pos.x, pos.y, dim.x, dim.y);

    // adjust text position, i.e. the pen position, to match the first baseline
    vec2 textPos(texObject.bbox.glyphPenOffset);

    // render text into texture
    vec2 scale(2.f / vec2(dim));
    render(str, vec2(-1.0f, 1.0f) - textPos * scale, scale, color);
}

void TextRenderer::renderToTexture(std::shared_ptr<Texture2D> texture,
                                   const std::vector<size2_t>& origin,
                                   const std::vector<size2_t>& size,
                                   const std::vector<std::string>& str, const vec4& color,
                                   bool clearTexture) {

    auto state = setupRenderState(texture, clearTexture);

    for (auto&& elem : util::zip(origin, size, str)) {
        // set up viewport
        ivec2 pos(get<0>(elem));
        ivec2 dim(get<1>(elem));
        utilgl::ViewportState viewport(pos.x, pos.y, dim.x, dim.y);

        // render text into texture
        vec2 scale(2.f / vec2(dim));
        // adjust text position, i.e. the pen position, to match the first baseline
        vec2 textPos(computeBoundingBox(get<2>(elem)).glyphPenOffset);
        render(get<2>(elem), vec2(-1.0f, 1.0f) - textPos * scale, scale, color);
    }

    fbo_.deactivate();
}

void TextRenderer::renderToTexture(std::shared_ptr<Texture2D> texture,
                                   const std::vector<TexAtlasEntry>& entries, bool clearTexture) {

    auto state = setupRenderState(texture, clearTexture);

    for (auto& elem : entries) {
        // set up viewport
        utilgl::ViewportState viewport(elem.texPos.x, elem.texPos.y, elem.texExtent.x,
                                       elem.texExtent.y);

        // render text into texture
        vec2 scale(2.f / vec2(elem.texExtent));
        // adjust text position, i.e. the pen position, to match the first baseline
        vec2 textPos(computeBoundingBox(elem.value).glyphPenOffset);
        render(elem.value, vec2(-1.0f, 1.0f) - textPos * scale, scale, elem.color);
    }

    fbo_.deactivate();
}

vec2 TextRenderer::computeTextSize(const std::string& str, const vec2& scale) {
    return vec2(computeTextSize(str)) * scale;
}

size2_t TextRenderer::computeTextSize(const std::string& str) {
    return computeBoundingBox(str).glyphsExtent;
}

void TextRenderer::setFontSize(int val) {
    if (fontSize_ != val) {
        fontSize_ = val;
        FT_Set_Pixel_Sizes(fontface_, 0, val);
    }
}

void TextRenderer::setLineSpacing(double lineSpacing) { lineSpacing_ = lineSpacing; }

double TextRenderer::getLineSpacing() const { return lineSpacing_; }

void TextRenderer::setLineHeight(int lineHeight) {
    lineSpacing_ = static_cast<double>(lineHeight) / static_cast<double>(fontSize_) - 1.0;
}

int TextRenderer::getLineHeight() const {
    return static_cast<int>(fontSize_ * (1.0 + lineSpacing_));
}

int TextRenderer::getBaseLineOffset() const { return static_cast<int>(getFontAscender() + 0.5); }

int TextRenderer::getBaseLineDescender() const {
    return static_cast<int>(getFontDescender() + 0.5);
}

double TextRenderer::getFontAscender() const {
    return (fontface_->ascender * fontSize_ / static_cast<double>(fontface_->units_per_EM));
}

double TextRenderer::getFontDescender() const {
    return (fontface_->descender * fontSize_ / static_cast<double>(fontface_->units_per_EM));
}

std::pair<bool, TextRenderer::GlyphEntry> TextRenderer::requestGlyph(FontCache& fc,
                                                                     unsigned int glyph) {
    // try to find the glyph
    auto it = fc.glyphMap.find(glyph);
    if (it != fc.glyphMap.end()) {
        return std::make_pair(true, it->second);
    } else {
        // glyph doesn't exist yet. Try to add glyph
        return addGlyph(fc, glyph);
    }
}

std::pair<bool, TextRenderer::GlyphEntry> TextRenderer::addGlyph(FontCache& fc,
                                                                 unsigned int glyph) {
    if (FT_Load_Char(fontface_, glyph, FT_LOAD_RENDER)) {
        LogWarn("FreeType: could not load char: '" << static_cast<char>(glyph) << "' (0x"
                                                   << std::hex << glyph << ")");
        return std::make_pair(false, GlyphEntry());
    }

    // create glyph entry with glyph specific params like advance, bearing (bitmap_left and
    // bitmap_top), and size (bitmap.width, bitmap.height)
    //
    // \see
    // https://www.freetype.org/freetype2/docs/reference/ft2-base_interface.html#FT_GlyphSlotRec
    //
    const ivec2 advance(fontface_->glyph->advance.x >> 6, fontface_->glyph->advance.y >> 6);
    const ivec2 bitmapSize(fontface_->glyph->bitmap.width, fontface_->glyph->bitmap.rows);
    const ivec2 bitmapPos(fontface_->glyph->bitmap_left, fontface_->glyph->bitmap_top);

    GlyphEntry glyphEntry = {advance, bitmapSize, bitmapPos, ivec2(-1)};

    const ivec2 glyphExtent(glyphEntry.size + 2 * glyphMargin_);
    const ivec2 texDims(fc.glyphTex->getDimensions());

    size_t line = 0;
    while (line < fc.lineLengths.size()) {
        if ((fc.lineLengths[line] + glyphExtent.x < texDims.x) &&
            (fc.lineHeights[line + 1] - fc.lineHeights[line] >= glyphExtent.y)) {
            // found some space in the current line, store the glyph here
            glyphEntry.texAtlasPos = ivec2(fc.lineLengths[line], fc.lineHeights[line]);

            fc.lineLengths[line] += glyphExtent.x;
            break;
        }
        ++line;
    }
    // check remaining vertical space inside atlas texture
    if (line == fc.lineLengths.size()) {
        if (fc.lineHeights.back() + glyphExtent.y > texDims.y) {
            LogWarn("Could not cache char '" << static_cast<char>(glyph) << "' (0x" << std::hex
                                             << glyph << ") (max size for texture atlas exceeded)");
            return std::make_pair(false, glyphEntry);
        }
        // create a new, empty line in the texture and store the glyph there
        glyphEntry.texAtlasPos = ivec2(0, fc.lineHeights.back());

        fc.lineLengths.push_back(glyphExtent.x);
        fc.lineHeights.push_back(glyphExtent.y + fc.lineHeights.back());
    }

    fc.glyphMap.insert({glyph, glyphEntry});
    uploadGlyph(fc, glyph);

    return std::make_pair(true, glyphEntry);
}

void TextRenderer::uploadGlyph(FontCache& fc, unsigned int glyph) {
    fc.glyphTex->bind();

    const auto it = fc.glyphMap.find(glyph);
    if (it == fc.glyphMap.end()) {
        // glyph is not registered
        return;
    }

    if (FT_Load_Char(fontface_, glyph, FT_LOAD_RENDER)) return;

    const auto& elem = it->second;
    glTexSubImage2D(GL_TEXTURE_2D, 0, elem.texAtlasPos.x, elem.texAtlasPos.y, elem.size.x,
                    elem.size.y, GL_RED, GL_UNSIGNED_BYTE, fontface_->glyph->bitmap.buffer);
}

TextRenderer::FontCache& TextRenderer::getFontCache() {
    const auto font = getFontTuple();

    auto fontCacheIt = glyphAtlas_.find(font);
    if (fontCacheIt == glyphAtlas_.end()) {
        // texture atlas doesn't exist for the current font/style/size combination
        //
        // create a new atlas texture
        createDefaultGlyphAtlas();
        fontCacheIt = glyphAtlas_.find(font);
        if (fontCacheIt == glyphAtlas_.end()) {
            throw Exception("Could not create font atlas");
        }
    }
    return fontCacheIt->second;
}

void TextRenderer::createDefaultGlyphAtlas() {
    if (glyphAtlas_.find(getFontTuple()) != glyphAtlas_.end()) {
        // glyph atlas already exists
        return;
    }

    FontCache fc;

    // create glyphs for all ascii characters between 32 and 128
    for (unsigned int c = 32u; c < 128u; ++c) {
        if (FT_Load_Char(fontface_, c, FT_LOAD_RENDER)) {
            LogWarn("FreeType: could not load char: '" << static_cast<char>(c) << "' (0x"
                                                       << std::hex << c << ")");
            continue;
        }

        const ivec2 advance(fontface_->glyph->advance.x >> 6, fontface_->glyph->advance.y >> 6);
        const ivec2 bitmapSize(fontface_->glyph->bitmap.width, fontface_->glyph->bitmap.rows);
        const ivec2 bitmapPos(fontface_->glyph->bitmap_left, fontface_->glyph->bitmap_top);

        fc.glyphMap[c] = {advance, bitmapSize, bitmapPos, ivec2(-1)};
    }

    fc.glyphTex = createAtlasTexture(fc);

    // upload all glyphs
    for (unsigned int c = 32u; c < 128u; ++c) {
        if (FT_Load_Char(fontface_, c, FT_LOAD_RENDER)) {
            continue;
        }

        const auto it = fc.glyphMap.find(c);
        if (it == fc.glyphMap.end()) {
            // glyph is not registered
            continue;
        }
        const auto& elem = it->second;
        glTexSubImage2D(GL_TEXTURE_2D, 0, elem.texAtlasPos.x, elem.texAtlasPos.y, elem.size.x,
                        elem.size.y, GL_RED, GL_UNSIGNED_BYTE, fontface_->glyph->bitmap.buffer);
    }

    // insert font cache into global map
    glyphAtlas_.insert({getFontTuple(), std::move(fc)});
}

std::shared_ptr<Texture2D> TextRenderer::createAtlasTexture(FontCache& fc) {
    std::vector<unsigned int> indices(fc.glyphMap.size());
    std::iota(indices.begin(), indices.end(), 32u);

    // sort labels according to width then height
    std::sort(indices.begin(), indices.end(), [&](auto a, auto b) {
        const auto& extA(fc.glyphMap[a].size);
        const auto& extB(fc.glyphMap[b].size);

        return ((extA.x > extB.x) || ((extA.x == extA.x) && (extA.y > extA.y)));
    });

    // figure out texture size to fit all glyph given a specific width using the Shelf First Fit
    // algorithm this function also updates the glyph positions within the new atlas texture
    auto calcTexLayout = [&](const int width, const int margin) {
        std::vector<int> lineLengths;
        std::vector<int> lineHeights;
        lineLengths.push_back(0);
        lineHeights.push_back(0);
        // Fill each line by putting each element after the previous one.
        // If an element does not fit, start new line.
        for (auto i : indices) {
            const auto& extent = fc.glyphMap[i].size + 2 * margin;
            size_t line = 0;
            while (line < lineLengths.size()) {
                if (lineLengths[line] + extent.x < width) {
                    // found a position with enough space, for now we only know the x coord,
                    // use y component to store current line
                    fc.glyphMap[i].texAtlasPos = ivec2(lineLengths[line] + margin, line);
                    lineLengths[line] += extent.x;
                    lineHeights[line] = std::max(extent.y, lineHeights[line]);
                    break;
                }
                ++line;
            }
            if (line == lineLengths.size()) {
                // no space found, create new line
                fc.glyphMap[i].texAtlasPos = ivec2(margin, line);
                lineLengths.push_back(extent.x);
                lineHeights.push_back(extent.y);
            }
        }
        // update y positions of all elements
        std::partial_sum(lineHeights.begin(), lineHeights.end(), lineHeights.begin());
        lineHeights.insert(lineHeights.begin(), 0);
        for (auto& elem : fc.glyphMap) {
            elem.second.texAtlasPos.y = lineHeights[elem.second.texAtlasPos.y] + margin;
        }

        fc.lineLengths = std::move(lineLengths);
        fc.lineHeights = std::move(lineHeights);

        return ivec2(width, fc.lineHeights.back());
    };

    // figure out conservative texture size
    const int maxTexSize = 8192;

    int width = 256;
    ivec2 texSize = calcTexLayout(width, glyphMargin_);
    while (texSize.y > width) {
        width *= 2;
        if (width > maxTexSize) {
            throw Exception("TextRenderer: font size too large (max size for font atlas exceeded)");
        }

        texSize = calcTexLayout(width, glyphMargin_);
    }

    // add space for another three rows of glyphs
    const int additionalRows = 3;
    const int maxRowHeight = fc.lineHeights[1];

    texSize.y =
        glm::min(maxTexSize, texSize.y + (maxRowHeight + 2 * glyphMargin_) * additionalRows);

    auto tex =
        std::make_shared<Texture2D>(size2_t(texSize), GL_RED, GL_RED, GL_UNSIGNED_BYTE, GL_LINEAR);
    // clear texture
    std::vector<unsigned char> data(texSize.x * texSize.y, 0);
    tex->initialize(data.data());

    return tex;
}

TextRenderer::FontFamilyStyle TextRenderer::getFontTuple() const {
    return std::make_tuple(std::string(fontface_->family_name), std::string(fontface_->style_name),
                           fontSize_);
}

void TextRenderer::configure(const FontSettings& settings) {
    setFont(settings.getFontFace());
    setFontSize(settings.getFontSize());
    setLineSpacing(settings.getLineSpacing());
}

std::shared_ptr<Shader> TextRenderer::getShader() {
    static std::weak_ptr<Shader> shader_;

    if (auto shader = shader_.lock()) {
        return shader;
    } else {
        shader = std::make_shared<Shader>("textrenderer.vert", "textrenderer.frag");
        shader_ = shader;
        return shader;
    }
}

namespace util {

TextTextureObject createTextTextureObject(TextRenderer& textRenderer, std::string text,
                                          vec4 fontColor, std::shared_ptr<Texture2D> tex) {

    auto bbox = textRenderer.computeBoundingBox(text);
    // Prevent OpenGL errors due to 0 size
    bbox.glyphsExtent = glm::max(bbox.glyphsExtent, size2_t(1));

    if (!tex || tex->getDimensions() != bbox.glyphsExtent) {
        tex = std::make_shared<Texture2D>(bbox.glyphsExtent, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE,
                                          GL_LINEAR);
        tex->initialize(nullptr);
    }
    textRenderer.renderToTexture({tex, bbox}, size2_t(0u), bbox.glyphsExtent, text, fontColor);
    return {tex, bbox};
}

std::shared_ptr<Texture2D> createTextTexture(TextRenderer& textRenderer, std::string text,
                                             vec4 fontColor, std::shared_ptr<Texture2D> tex) {
    auto texObj = createTextTextureObject(textRenderer, text, fontColor, tex);
    return texObj.texture;
}

}  // namespace util

}  // namespace inviwo
