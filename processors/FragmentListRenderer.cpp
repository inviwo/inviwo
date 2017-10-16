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

#include <fancymeshrenderer/processors/FragmentListRenderer.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/sharedopenglresources.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/image/imagegl.h>

namespace inviwo
{
    FragmentListRenderer::FragmentListRenderer()
        : screenSize_(0,0)
        , fragmentSize_(0)
        , abufferPageIdxImg_(nullptr)
        , abufferFragCountImg_(nullptr)
        , semaphoreImg_(nullptr)
        , atomicCounter_(0)
        , pixelBuffer_(0)
        , clearShader_("simpleQuad.vert", "clearABufferLinkedList.frag", true)
        , displayShader_("simpleQuad.vert", "dispABufferLinkedList.frag", true)
    {
        initShaders();

        //init atomic counter
        glGenBuffers(1, &atomicCounter_);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounter_);
        glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    }

    FragmentListRenderer::~FragmentListRenderer()
    {
        if (abufferPageIdxImg_) delete abufferPageIdxImg_;
        if (abufferFragCountImg_) delete abufferFragCountImg_;
        if (semaphoreImg_) delete semaphoreImg_;
        if (atomicCounter_) glDeleteBuffers(1, &atomicCounter_);
        if (pixelBuffer_) glDeleteBuffers(1, &pixelBuffer_);
        LGL_ERROR;
    }

    void FragmentListRenderer::prePass(const size2_t& screenSize)
    {
        fragmentSize_ = std::max(fragmentSize_, screenSize.x*screenSize.y);
        initBuffers(screenSize, fragmentSize_);

        //reset counter
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounter_);
        GLuint v[1] = { 0 };
        glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), v);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
        LGL_ERROR;

        //clear textures
        clearShader_.activate();
        assignUniforms(clearShader_);
        drawQuad();
        clearShader_.deactivate();
    }

    void FragmentListRenderer::setShaderUniforms(Shader& shader)
    {
        assignUniforms(shader);
    }

    void FragmentListRenderer::postPass()
    {
    }

    void FragmentListRenderer::initShaders()
    {
        //nothing to do?
    }

    void FragmentListRenderer::initBuffers(const size2_t& screenSize, size_t fragmentSize)
    {
        if (screenSize != screenSize_)
        {
            screenSize_ = screenSize;
            //delete screen size textures
            if (abufferPageIdxImg_) delete abufferPageIdxImg_;
            if (abufferFragCountImg_) delete abufferFragCountImg_;
            if (semaphoreImg_) delete semaphoreImg_;

            //reallocate them
            abufferPageIdxImg_ = new Texture2D(screenSize, GL_RED, GL_R32UI, GL_UNSIGNED_INT, GL_NEAREST, 0);
            abufferFragCountImg_ = new Texture2D(screenSize, GL_RED, GL_R32UI, GL_UNSIGNED_INT, GL_NEAREST, 0);
            semaphoreImg_ = new Texture2D(screenSize, GL_RED, GL_R32UI, GL_UNSIGNED_INT, GL_NEAREST, 0);

            LogInfo("fragment-list: screen size buffers allocated of size " << screenSize);
        }

        if (fragmentSize != fragmentSize_)
        {
            fragmentSize_ = fragmentSize;
            //create new SSBO for the pixel storage
            if (pixelBuffer_) glDeleteBuffers(1, &pixelBuffer_);
            glGenBuffers(1, &pixelBuffer_);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, pixelBuffer_);
            glBufferData(GL_SHADER_STORAGE_BUFFER, fragmentSize*2*sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            LGL_ERROR;
        }
    }

    void FragmentListRenderer::assignUniforms(Shader& shader) const
    {
        //screen size textures
        TextureUnit abufferPageIdxUnit, abufferFragCountUnit, semaphoreUnit;
        shader.setUniform("abufferPageIdxImg", abufferPageIdxUnit.getUnitNumber());
        shader.setUniform("abufferFragCountImg", abufferFragCountUnit.getUnitNumber());
        shader.setUniform("abufferSemaphoreImg", semaphoreUnit.getUnitNumber());
        utilgl::bindTexture(*abufferPageIdxImg_, abufferPageIdxUnit);
        utilgl::bindTexture(*abufferFragCountImg_, abufferFragCountUnit);
        utilgl::bindTexture(*semaphoreImg_, semaphoreUnit);

        //pixel storage
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 6, atomicCounter_);
        LGL_ERROR;
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, pixelBuffer_);
        LGL_ERROR;

        //other uniforms
        shader.setUniform("AbufferParams.screenWidth", static_cast<int>(screenSize_.x));
        shader.setUniform("AbufferParams.screenHeight", static_cast<int>(screenSize_.y));
        shader.setUniform("AbufferParams.sharedPoolSize", static_cast<int>(fragmentSize_));
    }

    void FragmentListRenderer::drawQuad() const
    {
        auto rect = SharedOpenGLResources::getPtr()->imagePlaneRect();
        utilgl::Enable<MeshGL> enable(rect);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}
