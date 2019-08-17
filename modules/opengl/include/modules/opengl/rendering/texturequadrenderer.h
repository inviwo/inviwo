/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_TEXTUREQUADRENDERER_H
#define IVW_TEXTUREQUADRENDERER_H

#include <modules/opengl/openglmoduledefine.h>
#include <modules/opengl/shader/shader.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/datastructures/camera.h>

namespace inviwo {
class Image;
class Layer;
class Texture2D;

/**
 * \class TextureQuadRenderer
 * \brief This class provides functionality for rendering an image, a layer, or a texture
 * onto the current render target.
 */
class IVW_MODULE_OPENGL_API TextureQuadRenderer {
public:
    TextureQuadRenderer();
    TextureQuadRenderer(const Shader& shader);
    TextureQuadRenderer(Shader&& shader);
    virtual ~TextureQuadRenderer();

    Shader& getShader();
    const Shader& getShader() const;

    /**
     * \brief renders an image at position pos onto the current canvas. The image
     * dimensions determine the covered area in pixel. The anchor point of the
     * image is in the lower left corner. By default, the first color layer is rendered.
     *
     * @param image   input image which is to be rendered onto the current render target
     * @param pos     position of lower left corner in screen space coordinates
     * @param canvasSize   dimensions of the current render target
     * @param layerType  defines which layer of the input image will be rendered, i.e. Color, Depth,
     *                   or Picking
     * @param transformation  additional transformation matrix to be applied before rendering. (For
     *                 example a rotation matrix to rotate the quad)
     * @param texTransform   optional transformation for texture coordinates, e.g.
     *                 for rendering a sub region of the input
     */
    void render(const Image& image, const ivec2& pos, const size2_t& canvasSize,
                LayerType layerType = LayerType::Color, const mat4& transformation = mat4(1),
                const mat4& texTransform = mat4(1));
    void render(const Image& image, const std::vector<ivec2>& pos,
                const std::vector<mat4>& texTransform, const size2_t& canvasSize,
                LayerType layerType = LayerType::Color, const mat4& transformation = mat4(1));
    void render(const std::shared_ptr<Image>& image, const ivec2& pos, const size2_t& canvasSize,
                LayerType layerType = LayerType::Color, const mat4& transformation = mat4(1),
                const mat4& texTransform = mat4(1));
    void render(const std::shared_ptr<Image>& image, const std::vector<ivec2>& pos,
                const std::vector<mat4>& texTransform, const size2_t& canvasSize,
                LayerType layerType = LayerType::Color, const mat4& transformation = mat4(1));

    /**
     * \brief renders a color layer of an image at position pos onto the current canvas. The layer
     * dimensions determine the covered area in pixel. The anchor point of the image is in the lower
     * left corner. By default, the first color layer is rendered.
     *
     * @param image   input image which is to be rendered onto the current render target
     * @param pos     position of lower left corner in screen space coordinates
     * @param canvasSize   dimensions of the current render target
     * @param colorLayerIndex defines which color layer of the input image will be rendered.
     * @param transformation  additional transformation matrix to be applied before rendering. (For
     *                 example a rotation matrix to rotate the quad)
     * @param texTransform   optional transformation for texture coordinates, e.g.
     *                 for rendering a sub region of the input
     */
    void render(const Image& image, const ivec2& pos, const size2_t& canvasSize,
                std::size_t colorLayerIndex, const mat4& transformation = mat4(1),
                const mat4& texTransform = mat4(1));
    void render(const Image& image, const std::vector<ivec2>& pos,
                const std::vector<mat4>& texTransform, const size2_t& canvasSize,
                std::size_t colorLayerIndex, const mat4& transformation = mat4(1));
    void render(const std::shared_ptr<Image>& image, const ivec2& pos, const size2_t& canvasSize,
                std::size_t colorLayerIndex, const mat4& transformation = mat4(1),
                const mat4& texTransform = mat4(1));
    void render(const std::shared_ptr<Image>& image, const std::vector<ivec2>& pos,
                const std::vector<mat4>& texTransform, const size2_t& canvasSize,
                std::size_t colorLayerIndex, const mat4& transformation = mat4(1));

    /**
     * \brief renders a layer at position pos onto the current canvas. The layer
     * dimensions determine the covered area in pixel. The anchor point of the
     * image is in the lower left corner.
     *
     * @param image   layer which is to be rendered onto the current render target
     * @param pos     position of lower left corner in screen space coordinates
     * @param canvasSize   dimensions of the current render target
     * @param transformation  additional transformation matrix to be applied before rendering. (For
     *                 example a rotation matrix to rotate the quad)
     * @param texTransform   optional transformation for texture coordinates, e.g.
     *                 for rendering a sub region of the input
     */
    void render(const Layer& image, const ivec2& pos, const size2_t& canvasSize,
                const mat4& transformation = mat4(1), const mat4& texTransform = mat4(1));
    void render(const Layer& image, const std::vector<ivec2>& pos,
                const std::vector<mat4>& texTransform, const size2_t& canvasSize,
                const mat4& transformation = mat4(1));
    void render(const std::shared_ptr<Layer>& image, const ivec2& pos, const size2_t& canvasSize,
                const mat4& transformation = mat4(1), const mat4& texTransform = mat4(1));
    void render(const std::shared_ptr<Layer>& image, const std::vector<ivec2>& pos,
                const std::vector<mat4>& texTransform, const size2_t& canvasSize,
                const mat4& transformation = mat4(1));

    /**
     * \brief renders a texture at position pos onto the current canvas. The texture
     * dimensions determine the covered area in pixel. The anchor point of the
     * texture is in the lower left corner.
     *
     * @param texture   texture which is to be rendered onto the current render target
     * @param pos     position of lower left corner in screen space coordinates
     * @param canvasSize   dimensions of the current render target
     * @param transformation  additional transformation matrix to be applied before rendering. (For
     *                 example a rotation matrix to rotate the quad)
     * @param texTransform   optional transformation for texture coordinates, e.g.
     *                 for rendering a sub region of the input
     */
    void render(const Texture2D& texture, const ivec2& pos, const size2_t& canvasSize,
                const mat4& transformation = mat4(1), const mat4& texTransform = mat4(1));
    void render(const Texture2D& texture, const std::vector<ivec2>& pos,
                const std::vector<mat4>& texTransform, const size2_t& canvasSize,
                const mat4& transformation = mat4(1));
    void render(const std::shared_ptr<Texture2D>& texture, const ivec2& pos,
                const size2_t& canvasSize, const mat4& transformation = mat4(1),
                const mat4& texTransform = mat4(1));
    void render(const std::shared_ptr<Texture2D>& texture, const std::vector<ivec2>& pos,
                const std::vector<mat4>& texTransform, const size2_t& canvasSize,
                const mat4& transformation = mat4(1));

    /**
     * \brief renders an image at position pos onto the current canvas with the given
     * extent. The covered area is defined by the extent (in pixel). The anchor point
     * of the image is in the lower left corner. By default, the first color layer is
     * rendered.
     *
     * @param image   input image which is to be rendered onto the current render target
     * @param pos     position of lower left corner in screen space coordinates
     * @param extent      extent covered by the rendered texture in screen space coordinates
     * @param canvasSize   dimensions of the current render target
     * @param layerType  defines which layer of the input image will be rendered, i.e. Color, Depth,
     *                   or Picking
     * @param transformation  additional transformation matrix to be applied before rendering. (For
     *                 example a rotation matrix to rotate the quad)
     * @param texTransform   optional transformation for texture coordinates, e.g.
     *                 for rendering a sub region of the input
     */
    void renderToRect(const Image& image, const ivec2& pos, const ivec2& extent,
                      const size2_t& canvasSize, LayerType layerType = LayerType::Color,
                      const mat4& transformation = mat4(1), const mat4& texTransform = mat4(1));
    void renderToRect(const Image& image, const std::vector<ivec2>& pos,
                      const std::vector<ivec2>& extent, const std::vector<mat4>& texTransform,
                      const size2_t& canvasSize, LayerType layerType = LayerType::Color,
                      const mat4& transformation = mat4(1));
    void renderToRect(const std::shared_ptr<Image>& image, const ivec2& pos, const ivec2& extent,
                      const size2_t& canvasSize, LayerType layerType = LayerType::Color,
                      const mat4& transformation = mat4(1), const mat4& texTransform = mat4(1));
    void renderToRect(const std::shared_ptr<Image>& image, const std::vector<ivec2>& pos,
                      const std::vector<ivec2>& extent, const std::vector<mat4>& texTransform,
                      const size2_t& canvasSize, LayerType layerType = LayerType::Color,
                      const mat4& transformation = mat4(1));

    /**
     * \brief renders a color layer of an image at position pos onto the current canvas
     * with the given extent. The covered area is defined by the extent (in pixel).
     * The anchor point of the image is in the lower left corner. By default,
     * the first color layer is rendered.
     *
     * @param image   input image which is to be rendered onto the current render target
     * @param pos     position of lower left corner in screen space coordinates
     * @param extent      extent covered by the rendered texture in screen space coordinates
     * @param canvasSize   dimensions of the current render target
     * @param colorLayerIndex defines which color layer of the input image will be rendered.
     * @param transformation  additional transformation matrix to be applied before rendering. (For
     *                 example a rotation matrix to rotate the quad)
     * @param texTransform   optional transformation for texture coordinates, e.g.
     *                 for rendering a sub region of the input
     */
    void renderToRect(const Image& image, const ivec2& pos, const ivec2& extent,
                      const size2_t& canvasSize, std::size_t colorLayerIndex,
                      const mat4& transformation = mat4(1), const mat4& texTransform = mat4(1));
    void renderToRect(const Image& image, const std::vector<ivec2>& pos,
                      const std::vector<ivec2>& extent, const std::vector<mat4>& texTransform,
                      const size2_t& canvasSize, std::size_t colorLayerIndex,
                      const mat4& transformation = mat4(1));
    void renderToRect(const std::shared_ptr<Image>& image, const ivec2& pos, const ivec2& extent,
                      const size2_t& canvasSize, std::size_t colorLayerIndex,
                      const mat4& transformation = mat4(1), const mat4& texTransform = mat4(1));
    void renderToRect(const std::shared_ptr<Image>& image, const std::vector<ivec2>& pos,
                      const std::vector<ivec2>& extent, const std::vector<mat4>& texTransform,
                      const size2_t& canvasSize, std::size_t colorLayerIndex,
                      const mat4& transformation = mat4(1));

    /**
     * \brief renders a layer at position pos onto the current canvas with the given
     * extent. The covered area is defined by the extent (in pixel). The anchor point
     * of the layer is in the lower left corner.
     *
     * @param image   layer which is to be rendered onto the current render target
     * @param pos     position of lower left corner in screen space coordinates
     * @param extent      extent covered by the rendered texture in screen space coordinates
     * @param canvasSize   dimensions of the current render target
     * @param transformation  additional transformation matrix to be applied before rendering. (For
     *                  example a rotation matrix to rotate the quad)
     * @param texTransform   optional transformation for texture coordinates, e.g.
     *                 for rendering a sub region of the input
     */
    void renderToRect(const Layer& image, const ivec2& pos, const ivec2& extent,
                      const size2_t& canvasSize, const mat4& transformation = mat4(1),
                      const mat4& texTransform = mat4(1));
    void renderToRect(const Layer& image, const std::vector<ivec2>& pos,
                      const std::vector<ivec2>& extent, const std::vector<mat4>& texTransform,
                      const size2_t& canvasSize, const mat4& transformation = mat4(1));
    void renderToRect(const std::shared_ptr<Layer>& image, const ivec2& pos, const ivec2& extent,
                      const size2_t& canvasSize, const mat4& transformation = mat4(1),
                      const mat4& texTransform = mat4(1));
    void renderToRect(const std::shared_ptr<Layer>& image, const std::vector<ivec2>& pos,
                      const std::vector<ivec2>& extent, const std::vector<mat4>& texTransform,
                      const size2_t& canvasSize, const mat4& transformation = mat4(1));

    /**
     * \brief renders a texture at position pos onto the current canvas with the given
     * extent. The covered area is defined by the extent (in pixel). The anchor point
     * of the texture is in the lower left corner.
     *
     * @param texture     texture which is to be rendered onto the current render target
     * @param pos         position of lower left corner in screen space coordinates
     * @param extent      extent covered by the rendered texture in screen space coordinates
     * @param canvasSize  dimensions of the current render target
     * @param transformation  additional transformation matrix to be applied before rendering. (For
     *                 example a rotation matrix to rotate the quad)
     * @param texTransform   optional transformation for texture coordinates, e.g.
     *                 for rendering a sub region of the input
     */
    void renderToRect(const Texture2D& texture, const ivec2& pos, const ivec2& extent,
                      const size2_t& canvasSize, const mat4& transformation = mat4(1),
                      const mat4& texTransform = mat4(1));
    void renderToRect(const Texture2D& texture, const std::vector<ivec2>& pos,
                      const std::vector<ivec2>& extent, const std::vector<mat4>& texTransform,
                      const size2_t& canvasSize, const mat4& transformation = mat4(1));
    void renderToRect(const std::shared_ptr<Texture2D>& texture, const ivec2& pos,
                      const ivec2& extent, const size2_t& canvasSize,
                      const mat4& transformation = mat4(1), const mat4& texTransform = mat4(1));
    void renderToRect(const std::shared_ptr<Texture2D>& texture, const std::vector<ivec2>& pos,
                      const std::vector<ivec2>& extent, const std::vector<mat4>& texTransform,
                      const size2_t& canvasSize, const mat4& transformation = mat4(1));

    void renderToRect(const Texture2D& texture, const std::vector<ivec2>& pos,
                      const std::vector<ivec2>& extent, const std::vector<mat4>& texTransform,
                      const size2_t& canvasSize, const std::vector<mat4>& transformation);

    /**
     * \brief renders a texture at world position pos onto the current canvas with the given
     * extent. The covered area is defined by the extent (in pixel). The anchor point
     * of the texture is defined by anchor (-1 to 1).
     *
     * @param camera      camera used for determining the screen position
     * @param texture     texture which is to be rendered onto the current render target
     * @param pos         position in world coordinates
     * @param extent      extent covered by the rendered texture in screen space coordinates
     * @param canvasSize  dimensions of the current render target
     * @param anchor      anchor position of texture (default lower left, i.e. (-1,-1))
     * @param transformation  additional transformation matrix to be applied before rendering. (For
     *                 example a rotation matrix to rotate the quad)
     * @param texTransform   optional transformation for texture coordinates, e.g.
     *                 for rendering a sub region of the input
     */
    void renderToRect3D(const Camera& camera, const Texture2D& texture, const vec3& pos,
                        const ivec2& extent, const size2_t& canvasSize,
                        const vec2& anchor = vec2(-1.0f), const mat4& transformation = mat4(1),
                        const mat4& texTransform = mat4(1));
    void renderToRect3D(const Camera& camera, const Texture2D& texture,
                        const std::vector<vec3>& pos, const std::vector<ivec2>& extent,
                        const std::vector<mat4>& texTransform, const size2_t& canvasSize,
                        const vec2& anchor = vec2(-1.0f), const mat4& transformation = mat4(1));
    void renderToRect3D(const Camera& camera, const std::shared_ptr<Texture2D>& texture,
                        const vec3& pos, const ivec2& extent, const size2_t& canvasSize,
                        const vec2& anchor = vec2(-1.0f), const mat4& transformation = mat4(1),
                        const mat4& texTransform = mat4(1));
    void renderToRect3D(const Camera& camera, const std::shared_ptr<Texture2D>& texture,
                        const std::vector<vec3>& pos, const std::vector<ivec2>& extent,
                        const std::vector<mat4>& texTransform, const size2_t& canvasSize,
                        const vec2& anchor = vec2(-1.0f), const mat4& transformation = mat4(1));

private:
    static std::shared_ptr<Shader> getDefaultShader();

    std::shared_ptr<Shader> shader_;
};

}  // namespace inviwo

#endif  // IVW_TEXTUREQUADRENDERER_H
