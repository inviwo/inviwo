/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2012-2016 Inviwo Foundation
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
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/buffer/framebufferobject.h>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace inviwo {

class Texture2D;

/**
 * \class TextRenderer
 *
 * \brief Render text using the FreeType font library
 *
 * The origin is at the top-left corner. The first line starts at origin - ascent (getBaseLineOffset()).
 * The distance between two lines is governed by either setting the line height or the line spacing.
 * The line height defines the distance between two consecutive baselines. In contrast, the line
 * spacing (or line gap) is given by line height minus ascend minus descend. Typically, a line
 * spacing of 20% of the font size is used, which corresponds to a line height of 1.2 * font size.
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
 *     g::::::ggggg::::::gg f::::::::::::f          │        Ascent
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
 *     g:::::gg   gg:::::g                          │       Descent
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
    * @param str    input string
    * @param color  color of rendered text
    */
    void renderToTexture(std::shared_ptr<Texture2D> texture, const std::string &str, const vec4 &color);

    /**
    * \brief computes the bounding box of a given string in normalized device coordinates using the
    * scaling factor. The vertical height of the bounding box will be equal to (ascend + descend) +
    * (number of lines - 1) times line height.
    *
    * @param str    input string
    * @param scale  scaling factor from screen space (pixel) to normalized device coordinates
    * ([-1,1])
    * @return size of the bounding box enclosing the input string in normalized device coordinates
    */
    vec2 computeTextSize(const std::string& str, const vec2& scale);

    /**
     * \brief computes the bounding box of a given string in pixels (screen space). The vertical
     * height of the bounding box will be equal to (ascend + descend) + (number of lines - 1) times
     * line height.
     *
     * @param str  input string
     * @return size of the bounding box enclosing the input string in pixel
     */
    vec2 computeTextSize(const std::string &str);

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
    int getBaseLineDescent() const;

protected:
    void initMesh();

    double getFontAscent() const;
    double getFontDescent() const;

    FT_Library fontlib_;
    FT_Face fontface_;

    int fontSize_; //<! font size in pixel
    double lineSpacing_; //!< spacing between two lines in percent (default = 0.2)

    Shader textShader_;
    GLuint texCharacter_;
    std::unique_ptr<Mesh> mesh_;
    std::unique_ptr<MeshDrawerGL> drawer_;

    FrameBufferObject fbo_;
    std::shared_ptr<Texture2D> prevTexture_; //<! 2D texture handle which was used previously in renderToTexture()
};

}  // namespace

#endif  // IVW_TEXTRENDERER_H
