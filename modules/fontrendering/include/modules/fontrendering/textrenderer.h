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

#ifndef IVW_TEXTRENDERER_H
#define IVW_TEXTRENDERER_H

#include <modules/fontrendering/fontrenderingmoduledefine.h>
#include <modules/fontrendering/util/fontutils.h>
#include <modules/fontrendering/datastructures/texatlasentry.h>
#include <modules/fontrendering/datastructures/fontsettings.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/util/stdextensions.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/buffer/framebufferobject.h>
#include <modules/fontrendering/datastructures/textboundingbox.h>

#include <unordered_map>
#include <tuple>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace inviwo {

class Texture2D;

struct TextTextureObject {
    std::shared_ptr<Texture2D> texture;
    TextBoundingBox bbox;
};

/**
 * \class TextRenderer
 *
 * \brief Render text using the FreeType font library
 *
 * The origin is at the top-left corner. The first line starts at origin - ascender
 * (getBaseLineOffset()). The distance between two lines is governed by either setting the line
 * height or the line spacing. The line height defines the distance between two consecutive
 * baselines. In contrast, the line spacing (or line gap) is given by line height minus ascend minus
 * descend. Typically, a line spacing of 20% of the font size is used, which corresponds to a line
 * height of 1.2 * font size.
 *
 *
 * \verbatim
 * Origin
 * +───────────────────────────ffffffffffffffff──────────────────── Top line
 *                            f::::::::::::::::f    ▲          ▲
 *                           f::::::::::::::::::f   │          │
 *                           f::::::fffffff:::::f   │          │
 *        ggggggggg   ggggg  f:::::f       ffffff   │          │
 *       g:::::::::ggg::::g  f:::::f                │          │
 *      g:::::::::::::::::g f:::::::ffffff          │          │
 *     g::::::ggggg::::::gg f::::::::::::f          │        Ascender
 *     g:::::g     g:::::g  f::::::::::::f          │          │
 *     g:::::g     g:::::g  f:::::::ffffff          │          │
 *     g:::::g     g:::::g   f:::::f                │          │
 *     g::::::g    g:::::g   f:::::f                │          │
 *     g:::::::ggggg:::::g  f:::::::f               │          │
 *      g::::::::::::::::g  f:::::::f          Line height     │
 *       gg::::::::::::::g  f:::::::f               │          ▼
 * ────────gggggggg::::::g──fffffffff───────────────│────────────── Base line
 *                 g:::::g                          │          ▲
 *     gggggg      g:::::g                          │          │
 *     g:::::gg   gg:::::g                          │       Descender
 *      g::::::ggg:::::::g                          │          │
 *       gg:::::::::::::g                           │          │
 *         ggg::::::ggg                             │          ▼
 * ───────────gggggg────────────────────────────────│────────────── Bottom line
 *                                                  │          ▲
 *                                                  │          │
 *                                                  │     Line spacing
 *                                                  │          │
 *                                                  ▼          ▼
 * ──────────────────────────────────────────────────────────────── Top line (of subsequent line)
 * \endverbatim
 *
 */
class IVW_MODULE_FONTRENDERING_API TextRenderer {
public:
    TextRenderer(const std::string &fontPath = util::getDefaultFontPath() + "/arial.ttf");
    virtual ~TextRenderer();

    /**
     * \brief replace the currently loaded font face with a new one
     *
     * @param fontPath   full path to the new font face
     * @throws Exception      if the font file could not be opened
     * @throws FileException  if the font format is unsupported
     */
    void setFont(const std::string &fontPath);

    void render(const std::string &str, const vec2 &posf, const vec2 &scale, const vec4 &color);

    /**
     * \brief renders the given string with the specified color at position x, y in normalized
     * device coordinates [-1,1] using the scaling factor.
     *
     * @param str    input string
     * @param x      x component of position in normalized device coords
     * @param y      y component of position in normalized device coords
     * @param scale  scaling factor from screen space (pixel) to normalized device coords
     * @param color  color of rendered text
     */
    void render(const std::string &str, float x, float y, const vec2 &scale, const vec4 &color);

    /**
     * \brief renders the given string with the specified color into a texture.
     *
     * @param texture   the text will be rendered into this texture
     * @param str    input string
     * @param color  color of rendered text
     * @param clearTexture   if true, the texture is cleared before rendering the text
     */
    void renderToTexture(std::shared_ptr<Texture2D> texture, const std::string &str,
                         const vec4 &color, bool clearTexture = true);

    void renderToTexture(const TextTextureObject &texObject, const std::string &str,
                         const vec4 &color, bool clearTexture = true);
    /**
     * \brief renders the given string with the specified color into a subregion of the texture.
     *
     * @param texObject      the text will be rendered into this texture
     * @param origin         origin of sub region within the texture (lower left corner, in pixel)
     * @param size           extent of sub region (in pixel)
     * @param str            input string
     * @param color          color of rendered text
     * @param clearTexture   if true, the texture is cleared before rendering the text
     */
    void renderToTexture(const TextTextureObject &texObject, const size2_t &origin,
                         const size2_t &size, const std::string &str, const vec4 &color,
                         bool clearTexture = true);
    void renderToTexture(std::shared_ptr<Texture2D> texture, const size2_t &origin,
                         const size2_t &size, const std::string &str, const vec4 &color,
                         bool clearTexture = true);

    void renderToTexture(std::shared_ptr<Texture2D> texture, const std::vector<size2_t> &origin,
                         const std::vector<size2_t> &size, const std::vector<std::string> &str,
                         const vec4 &color, bool clearTexture = true);

    void renderToTexture(std::shared_ptr<Texture2D> texture,
                         const std::vector<TexAtlasEntry> &entries, bool clearTexture = true);

    /**
     * \brief computes the glyph bounding box of a given string in normalized device coordinates
     * using the scaling factor. The vertical height of the bounding box will be about (ascend +
     * descend) + (number of lines - 1) times line height.
     *
     * @param str    input string
     * @param scale  scaling factor from screen space (pixel) to normalized device coordinates
     * ([-1,1])
     * @return size of the bounding box enclosing all glyphs of the input string in normalized
     * device coordinates
     *
     * \see computeBoundingBox
     */
    vec2 computeTextSize(const std::string &str, const vec2 &scale);

    /**
     * \brief computes the glyph bounding box of a given string in pixels (screen space). The
     * vertical height of the bounding box will be about (ascend + descend) + (number of lines - 1)
     * times line height.
     *
     * @param str  input string
     * @return size of the bounding box enclosing all glyphs of the input string in pixel
     *
     * \see computeBoundingBox
     */
    size2_t computeTextSize(const std::string &str);

    /**
     * \brief computes the bounding boxes of both text and all glyphs for a given string in pixels
     * (screen space).
     *
     * The vertical height of the textual bounding box is equal to
     * (ascend + descend) + (number of lines - 1) times line height. The width corresponds to the
     * maximum sum of all glyph advances per row, which are given by the respective font.
     * Note: glyphs might extend beyond the text bounding box.
     *
     * @param str  input string
     * @return bounding box of the given string
     */
    TextBoundingBox computeBoundingBox(const std::string &str);

    void setFontSize(int val);
    int getFontSize() const { return fontSize_; }

    /**
     * \brief sets the line spacing relative to the font size (default 0.2 = 20%)
     *
     * @param lineSpacing   factor for line spacing
     */
    void setLineSpacing(double lineSpacing);
    double getLineSpacing() const;

    void setLineHeight(int lineHeight);
    int getLineHeight() const;

    /**
     * \brief returns the offset of the baseline, which corresponds to ascent
     *
     * @return baseline offset
     */
    int getBaseLineOffset() const;

    /**
     * \brief returns the size of the font part below the baseline, which corresponds to descent
     *
     * @return baseline offset
     */
    int getBaseLineDescender() const;

    void configure(const FontSettings &settings);

protected:
    static std::shared_ptr<Shader> getShader();

    struct GlyphEntry {

        /**
         * glyph specific metrics. The bearing defines the offset from the origin to
         * the top-left corner of the glyph.
         *
         * \see https://www.freetype.org/freetype2/docs/tutorial/step2.html
         */
        ivec2 advance;
        ivec2 size;     // corresponds to ivec2(bitmap.width, bitmap.rows)
        ivec2 bearing;  // corresponds to ivec2(bitmap_left, bitmap_top)

        ivec2 texAtlasPos;  //!< position in texture atlas
    };

    using FontFamilyStyle =
        std::tuple<std::string, std::string, int>;  // holds font family, style, and size
    using GlyphMap = std::map<unsigned int, GlyphEntry>;

    struct FontCache {
        std::shared_ptr<Texture2D> glyphTex;
        GlyphMap glyphMap;

        // current fill status of the texture
        std::vector<int> lineLengths;
        std::vector<int> lineHeights;
    };

    double getFontAscender() const;
    double getFontDescender() const;

    /**
     * \brief request glyph information from the texture atlas. The glyph will be added
     * to the atlas if it isn't registered yet and there is space left in the atlas.
     *
     * @param fc       font cache for the currently selected font, style, and size
     * @param glyph    character code of the glyph
     * @return std::pair<bool, GlyphEntry> where pair.first is true if the glyph exists
     *              and GlyphEntry refers to the respective glyph
     */
    std::pair<bool, GlyphEntry> requestGlyph(FontCache &fc, unsigned int glyph);

    std::pair<bool, GlyphEntry> addGlyph(FontCache &fc, unsigned int glyph);

    void uploadGlyph(FontCache &fc, unsigned int glyph);

    FontCache &getFontCache();

    void createDefaultGlyphAtlas();
    std::shared_ptr<Texture2D> createAtlasTexture(FontCache &fc);

    FontFamilyStyle getFontTuple() const;

    std::tuple<utilgl::DepthMaskState, utilgl::GlBoolState, utilgl::BlendModeState,
               utilgl::Activate<FrameBufferObject>>
    setupRenderState(std::shared_ptr<Texture2D> texture, bool clearTexture);

    std::string::const_iterator validateString(const std::string &str) const;

    static constexpr char lf = '\n';   // Line Feed Ascii for std::endl, \n
    static constexpr char tab = '\t';  // Tab Ascii

    std::unordered_map<FontFamilyStyle, FontCache> glyphAtlas_;

    FT_Library fontlib_;
    FT_Face fontface_;

    int fontSize_;        //<! font size in pixel
    double lineSpacing_;  //!< spacing between two lines in percent (default = 0.2)

    const int glyphMargin_;
    std::shared_ptr<Shader> shader_;

    FrameBufferObject fbo_;
    std::shared_ptr<Texture2D>
        prevTexture_;  //<! 2D texture handle which was used previously in renderToTexture()
};

namespace util {

/**
 * \brief Creates a texture with rendered text for a given string including its bounding box
 *
 * Creates a texture with rendered text for a string using the specified renderer and
 * color. May take an additional variable tex of an existing texture that can be reused to reduce
 * the number of times we need to allocating new textures. The resulting texture is returned along
 * with the respective bounding box.
 *
 * The size of the texture will be the smallest possible for the given text. All pixels
 * containing no text will have zero alpha.
 *
 * For correct alignment of the baseline, the position of where this texture will be rendered
 * must be adjusted by pos + TextTextureObject.glyphsOrigin.
 * See also TextOverlayGL::process() as an example.
 *
 * @param textRenderer The renderer that will be used to render the text
 * @param text         string to be rendered
 * @param fontColor    the final color of the text
 * @param tex          optional cache texture which will be reused if possible
 *                    (same texture will be returned)
 * @return text texture object referring to both texture and corresponding bounding box
 */
IVW_MODULE_FONTRENDERING_API TextTextureObject
createTextTextureObject(TextRenderer &textRenderer, std::string text, vec4 fontColor,
                        std::shared_ptr<Texture2D> tex = nullptr);

/**
 * \brief Creates a texture with rendered text for a given string
 *
 * Creates a texture with a text string using the specified renderer and color. May
 * take an additional variable tex of an existing texture that can be reused to reduce the number of
 * times we need to allocating new textures.
 *
 * The size of the texture will be the smallest possible for the given text and the pixels
 * containing no text will have zero alpha.
 *
 * For correct alignment of the baseline, the position of where this texture will be rendered
 * must be adjusted by pos + computeBoundingBox(text).glyphsOrigin.
 * See also TextOverlayGL::process() as an example.
 *
 * @param textRenderer The renderer that will be used to render the text
 * @param text text to be rendered
 * @param fontColor the final color of the text
 * @param tex optional cache texture which will be reused if possible
 * @return texture containing the text
 */
IVW_MODULE_FONTRENDERING_API std::shared_ptr<Texture2D> createTextTexture(
    TextRenderer &textRenderer, std::string text, vec4 fontColor,
    std::shared_ptr<Texture2D> tex = nullptr);

}  // namespace util

}  // namespace inviwo

#endif  // IVW_TEXTRENDERER_H
