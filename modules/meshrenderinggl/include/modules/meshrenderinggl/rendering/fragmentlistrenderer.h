/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2022 Inviwo Foundation
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

#include <modules/meshrenderinggl/meshrenderingglmoduledefine.h>  // for IVW_MODULE_MESHRENDERIN...

#include <inviwo/core/util/dispatcher.h>                          // for Dispatcher, Dispatcher<...
#include <inviwo/core/util/glmvec.h>                              // for size2_t, vec3
#include <modules/opengl/buffer/bufferobject.h>                   // for BufferObject
#include <modules/opengl/inviwoopengl.h>                          // for GLuint
#include <modules/opengl/shader/shader.h>                         // for Shader
#include <modules/opengl/texture/texture2d.h>                     // for Texture2D
#include <modules/opengl/texture/textureunit.h>                   // for TextureUnit (ptr only)

#include <array>                                                  // for array
#include <cstddef>                                                // for size_t
#include <functional>                                             // for function
#include <iosfwd>                                                 // for ostream

#include <glm/vec3.hpp>                                           // for vec

namespace inviwo {
class Image;

/**
 * \brief helper class for rendering perfect alpha-blended shapes using fragment lists.
 * Inspiration taken from
 * http://blog.icare3d.org/2010/07/opengl-40-abuffer-v20-linked-lists-of.html.
 * It requires OpenGL 4.2.
 *
 * Any objects can be rendered with this framework in the following way:
 * 1. Render opaque objects normally
 * 2. Call FragmentListRenderer::prePass(...)
 * 3. For each transparent object:
 *    a) Include oit/abufferlinkedlist.glsl in the fragment shader
 *    b) Use the following snipped in the fragment shader:
 *       abufferRender(ivec2(gl_FragCoord.xy), depth, fragColor);
 *       discard;
 *    c) Assign additional shader uniforms with FragmentListRenderer::setShaderUniforms(shader)
 *    d) Render the object with depth test but without depth write
 * 4. Call FragmentListRenderer::postPass(...)
 *    If this returns <code>false</code>, not enough space for all pixels
 *    was available. Repeat from step 2.
 */
class IVW_MODULE_MESHRENDERINGGL_API FragmentListRenderer {
public:
    FragmentListRenderer();
    ~FragmentListRenderer();

    /**
     * \brief Starts the rendering of transparent objects using fragment lists.
     * It resets all counters and allocated the index textures of the given screen size.
     * This has to be called each frame before objects can be rendered with the fragment lists.
     * \param screenSize the current screen size
     */
    void prePass(const size2_t& screenSize);

    /**
     * \brief Sets the shader uniforms required by the fragment list renderer.
     * The uniforms are defined in <code>oit/abufferlinkedlist.glsl</code>
     * \param shader the shader of the object to be rendered
     */
    void setShaderUniforms(Shader& shader);

    /**
     * \brief Finishes the fragment list pass and renders the final result.
     * This sorts the fragment lists per pixel and outputs the blended color.
     * \param useIllustration Set to true if the illustration buffer
     * should be enabled
     * \param background The background to render on and use depth information from.
     * \return <code>true</code> if successfull, <code>false</code> if not enough
     * space for all fragments was available and the procedure should be repeated.
     */
    bool postPass(bool useIllustration, const Image* background);

    struct IllustrationSettings {
        vec3 edgeColor_;
        float edgeStrength_;
        float haloStrength_;
        int smoothingSteps_;
        float edgeSmoothing_;
        float haloSmoothing_;
    };
    void setIllustrationSettings(const IllustrationSettings& settings) {
        illustration_.settings = settings;
    }

    /**
     * \brief Tests if fragment lists are supported by the current opengl context.
     * Fragment lists require OpenGL 4.3
     * \return true iff they are supported
     */
    static bool supportsFragmentLists();
    /**
     * \brief Tests if the illustration buffer are supported and can therefore be enabled.
     * The Illustration Buffer requires OpenGL 4.6 or OpenGL 4.5 with the extension
     * "GL_ARB_shader_atomic_counter_ops". \return true iff they are supported
     */
    static bool supportsIllustration();

    typename Dispatcher<void()>::Handle onReload(std::function<void()> callback);

    void debugFragmentLists(std::ostream& oss);
    void debugIllustrationBuffer(std::ostream& oss);

private:
    void buildShaders(bool hasBackground = false);

    void setUniforms(Shader& shader, TextureUnit& abuffUnit) const;
    void resizeBuffers(const size2_t& screenSize);

    void fillIllustration(TextureUnit& abuffUnit, TextureUnit& idxUnit, TextureUnit& countUnit,
                          const Image* background);

    size2_t screenSize_;
    size_t fragmentSize_;

    // basic fragment lists
    Texture2D abufferIdxTex_;
    TextureUnitContainer textureUnits_;
    bool builtWithBackground_ = false;

    BufferObject atomicCounter_;
    BufferObject pixelBuffer_;

    GLuint totalFragmentQuery_;

    Shader clear_;
    Shader display_;

    struct Illustration {
        Illustration(size2_t screenSize, size_t fragmentSize);
        void resizeBuffers(size2_t screenSize, size_t fragmentSize);
        void setUniforms(Shader& shader, TextureUnit& idxUnit, TextureUnit& countUnit);

        void process(BufferObject& pixelBuffer, TextureUnit& idxUnit, TextureUnit& countUnit);
        void render(TextureUnit& idxUnit, TextureUnit& countUnit);

        Texture2D index;
        Texture2D count;

        BufferObject color;
        BufferObject surfaceInfo;

        std::array<BufferObject, 2> smoothing;
        int activeSmoothing;

        Shader fill;
        Shader neighbors;
        Shader draw;
        Shader smooth;

        IllustrationSettings settings;

        Dispatcher<void()> onReload;
    };

    Illustration illustration_;
    typename Dispatcher<void()>::Handle illustrationOnReload_;

    Dispatcher<void()> onReload_;
};

}  // namespace inviwo
