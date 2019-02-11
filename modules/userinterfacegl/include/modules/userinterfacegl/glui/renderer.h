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

#ifndef IVW_GLUIRENDERER_H
#define IVW_GLUIRENDERER_H

#include <modules/userinterfacegl/userinterfaceglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/fontrendering/textrenderer.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/rendering/texturequadrenderer.h>

#include <map>

namespace inviwo {

class Mesh;
class MeshDrawerGL;
class Texture2D;
class Texture2DArray;

namespace glui {

/**
 * \class Renderer
 * \brief provides the basic rendering functionality required to render UI elements. Also provides a
 * texture cache for different widgets. Each set of widget textures is represented by a 2D texture
 * array consisting of six textures (widget state normal, pressed, checked plus corresponding
 * halos).
 *
 * \see glui::Element
 */
class IVW_MODULE_USERINTERFACEGL_API Renderer {
public:
    Renderer();
    virtual ~Renderer() = default;

    /**
     * \brief create a UI texture object representing the normal, pressed, and checked state
     * for both the UI widget and its halo.
     *
     * @param name    internal name of the texture object
     * @param files   list of file names representing the following states:
     *                  texture:   normal, pressed, checked
     *                  halo:      normal, pressed, checked
     * @return pointer to texture array
     * @throws Exception if not successful.
     */
    Texture2DArray* createUITextures(const std::string& name, const std::vector<std::string>& files,
                                     const std::string& sourcePath);

    Shader& getShader();
    const Shader& getShader() const;

    TextRenderer& getTextRenderer(bool bold = false);
    const TextRenderer& getTextRenderer(bool bold = false) const;

    int getDefaultFontSize() const;

    TextureQuadRenderer& getTextureQuadRenderer();
    const TextureQuadRenderer& getTextureQuadRenderer() const;

    MeshDrawerGL* getMeshDrawer() const;

    Texture2DArray* getUITextures(const std::string& name) const;

    void setTextColor(const vec4& color);
    const vec4& getTextColor() const;

    void setUIColor(const vec4& color);
    const vec4& getUIColor() const;

    void setSecondaryUIColor(const vec4& color);
    const vec4& getSecondaryUIColor() const;

    void setBorderColor(const vec4& color);
    const vec4& getBorderColor() const;

    void setHoverColor(const vec4& color);
    const vec4& getHoverColor() const;

    void setDisabledColor(const vec4& color);
    const vec4& getDisabledColor() const;

protected:
    void setupRectangleMesh();

    std::shared_ptr<Texture2DArray> createUITextureObject(
        const std::vector<std::string>& textureFiles, const std::string& sourcePath) const;

    const int defaultFontSize_ = 13;

    Shader uiShader_;
    TextRenderer textRenderer_;
    TextRenderer textRendererBold_;
    TextureQuadRenderer quadRenderer_;

    std::shared_ptr<MeshDrawerGL> meshDrawer_;
    std::shared_ptr<Mesh> rectangleMesh_;

    std::map<std::string, std::shared_ptr<Texture2DArray>> uiTextureMap_;

    vec4 colorUI_;
    vec4 colorSecondaryUI_;
    vec4 colorBorder_;
    vec4 colorText_;
    vec4 colorHover_;
    vec4 colorDisabled_;

    double scaling_;
};

}  // namespace glui

}  // namespace inviwo

#endif  // IVW_GLUIRENDERER_H
