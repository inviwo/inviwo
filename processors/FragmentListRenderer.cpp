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
        , fragmentSize_(1024)
        , oldFragmentSize_(0)
        , abufferIdxImg_(nullptr)
        //, abufferFragCountImg_(nullptr)
        //, semaphoreImg_(nullptr)
        , atomicCounter_(0)
        , pixelBuffer_(0)
        , totalFragmentQuery_(0)
        , clearShader_("simpleQuad.vert", "clearABufferLinkedList.frag", false)
        , displayShader_("simpleQuad.vert", "dispABufferLinkedList.frag", false)
    {
        initShaders();

        //init atomic counter
        glGenBuffers(1, &atomicCounter_);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounter_);
        glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
        LGL_ERROR;

        //create fragment query
        glGenQueries(1, &totalFragmentQuery_);
        LGL_ERROR;
    }

    FragmentListRenderer::~FragmentListRenderer()
    {
        if (abufferIdxImg_) delete abufferIdxImg_;
        //if (abufferFragCountImg_) delete abufferFragCountImg_;
        //if (semaphoreImg_) delete semaphoreImg_;
        if (atomicCounter_) glDeleteBuffers(1, &atomicCounter_);
        if (pixelBuffer_) glDeleteBuffers(1, &pixelBuffer_);
        if (totalFragmentQuery_) glDeleteQueries(1, &totalFragmentQuery_);
        LGL_ERROR;
    }

    void FragmentListRenderer::prePass(const size2_t& screenSize)
    {
        initBuffers(screenSize);

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

        //start query
        glBeginQuery(GL_SAMPLES_PASSED, totalFragmentQuery_);
        LGL_ERROR;
    }

    void FragmentListRenderer::setShaderUniforms(Shader& shader) const
    {
        assignUniforms(shader);
    }

    bool FragmentListRenderer::postPass()
    {
        LogInfo("FLR: post pass entry");
        //get query result
        GLuint numFrags = 0;
        glEndQuery(GL_SAMPLES_PASSED);
        glGetQueryObjectuiv(totalFragmentQuery_, GL_QUERY_RESULT, &numFrags);
        LGL_ERROR;
        LogInfo("FLR: fragment query: " << numFrags);

        //Test: read global counter
        GLuint counter = 0xffffffff;
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounter_);
        glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &counter);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
        LogInfo("FLR: value of the global counter: " << counter);

        //render fragment list (even if incomplete)
        displayShader_.activate();
        assignUniforms(displayShader_);
        utilgl::GlBoolState depthTest(GL_DEPTH_TEST, false);
        utilgl::DepthMaskState depthMask(GL_FALSE);
        utilgl::CullFaceState culling(GL_NONE);
        utilgl::BlendModeState blendModeStateGL(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        drawQuad();
        displayShader_.deactivate();

        LogInfo("fragment lists resolved, pixels drawn: " << numFrags << ", available: " << fragmentSize_);
        if (numFrags > fragmentSize_)
        {
            //we have to resize the fragment storage buffer
            fragmentSize_ = 1.1f * numFrags;
            return false;
        }

        return true; //success, enough storage available
    }

    void FragmentListRenderer::initShaders()
    {
        displayShader_.getFragmentShaderObject()->addShaderDefine("COLOR_LAYER");
        displayShader_.build();
        clearShader_.build();
    }

    void FragmentListRenderer::initBuffers(const size2_t& screenSize)
    {
        if (screenSize != screenSize_)
        {
            screenSize_ = screenSize;
            //delete screen size textures
            if (abufferIdxImg_) delete abufferIdxImg_;
            //if (abufferFragCountImg_) delete abufferFragCountImg_;
            //if (semaphoreImg_) delete semaphoreImg_;

            //reallocate them
            abufferIdxImg_ = new Texture2D(screenSize, GL_RED, GL_R32UI, GL_UNSIGNED_INT, GL_NEAREST, 0);
            //abufferFragCountImg_ = new Texture2D(screenSize, GL_RED, GL_R32UI, GL_UNSIGNED_INT, GL_NEAREST, 0);
            //semaphoreImg_ = new Texture2D(screenSize, GL_RED, GL_R32UI, GL_UNSIGNED_INT, GL_NEAREST, 0);

            LogInfo("fragment-list: screen size buffers allocated of size " << screenSize);
        }

        if (oldFragmentSize_ != fragmentSize_)
        {
            oldFragmentSize_ = fragmentSize_;
            //create new SSBO for the pixel storage
            if (pixelBuffer_) glDeleteBuffers(1, &pixelBuffer_);
            glGenBuffers(1, &pixelBuffer_);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, pixelBuffer_);
            glBufferData(GL_SHADER_STORAGE_BUFFER, fragmentSize_ *4*sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            LGL_ERROR;

            LogInfo("fragment-list: pixel storage for " << fragmentSize_ << " pixels allocated, memory usage: "
                << (fragmentSize_ * 4 * sizeof(GLfloat) /1024 / 1024.0f) << " MB" );
        }
    }

    void FragmentListRenderer::assignUniforms(Shader& shader) const
    {
        //screen size textures
        TextureUnit abufferIdxUnit, abufferFragCountUnit, semaphoreUnit;
        shader.setUniform("abufferIdxImg", abufferIdxUnit.getUnitNumber());
        //shader.setUniform("abufferFragCountImg", abufferFragCountUnit.getUnitNumber());
        //shader.setUniform("abufferSemaphoreImg", semaphoreUnit.getUnitNumber());
        utilgl::bindTexture(*abufferIdxImg_, abufferIdxUnit);
        //utilgl::bindTexture(*abufferFragCountImg_, abufferFragCountUnit);
        //utilgl::bindTexture(*semaphoreImg_, semaphoreUnit);

        //pixel storage
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 6, atomicCounter_);
        LGL_ERROR;
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, pixelBuffer_);
        LGL_ERROR;

        //other uniforms
        shader.setUniform("AbufferParams.screenWidth", static_cast<GLint>(screenSize_.x));
        shader.setUniform("AbufferParams.screenHeight", static_cast<GLint>(screenSize_.y));
        shader.setUniform("AbufferParams.sharedPoolSize", static_cast<GLuint>(fragmentSize_));
    }

    void FragmentListRenderer::drawQuad() const
    {
        auto rect = SharedOpenGLResources::getPtr()->imagePlaneRect();
        utilgl::Enable<MeshGL> enable(rect);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}
