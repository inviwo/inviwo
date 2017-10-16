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

#ifndef IVW_FRAGMENTLISTRENDERER_H
#define IVW_FRAGMENTLISTRENDERER_H

#include <fancymeshrenderer/fancymeshrenderermoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/opengl/inviwoopengl.h>
#include <inviwo/core/rendering/meshdrawer.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/texture/texture2d.h>

namespace inviwo {

/**
 * \brief helper class for rendering perfect alpha-blended shapes using fragment lists.
 * A large part of the code was taken from http://blog.icare3d.org/2010/07/opengl-40-abuffer-v20-linked-lists-of.html.
 */
class IVW_MODULE_FANCYMESHRENDERER_API FragmentListRenderer
{
public:
    FragmentListRenderer();
    ~FragmentListRenderer();

    void prePass(const size2_t& screenSize);

    void setShaderUniforms(Shader& shader);

    void postPass();

private:
    void initShaders();
    void initBuffers(const size2_t& screenSize, size_t fragmentSize);
    void assignUniforms(Shader& shader) const;
    void drawQuad() const;

    size2_t screenSize_;
    size_t fragmentSize_;

    Texture2D* abufferPageIdxImg_;
    Texture2D* abufferFragCountImg_;
    Texture2D* semaphoreImg_;
    GLuint atomicCounter_;
    GLuint pixelBuffer_;

    Shader clearShader_;
    Shader displayShader_;
};

}

#endif