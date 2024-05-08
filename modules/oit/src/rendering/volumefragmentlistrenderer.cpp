/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2024 Inviwo Foundation
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

#include <modules/oit/rendering/volumefragmentlistrenderer.h>

#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/sharedopenglresources.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/openglcapabilities.h>
#include <modules/opengl/shader/shaderutils.h>

#include <cstdio>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <modules/opengl/volume/volumegl.h>
#include <inviwo/core/util/stringconversion.h>

namespace inviwo {

VolumeFragmentListRenderer::VolumeFragmentListRenderer()
    : screenSize_{0, 0}
    , fragmentSize_{1024}
    , abufferIdxTex_{screenSize_, GL_RED, GL_R32F, GL_FLOAT, GL_NEAREST}
    , textureUnits_{}
    , atomicCounter_{sizeof(GLuint), GLFormats::getGLFormat(GL_UNSIGNED_INT, 1), GL_DYNAMIC_DRAW,
                     GL_ATOMIC_COUNTER_BUFFER}
    , pixelBuffer_{fragmentSize_ * 4 * sizeof(GLfloat), GLFormats::getGLFormat(GL_FLOAT, 4),
                   GL_DYNAMIC_DRAW, GL_SHADER_STORAGE_BUFFER}
    , totalFragmentQuery_{0}
    , clear_("oit/simplequad.vert", "oit/clear.frag", Shader::Build::No)
    , display_("oit/simplequad.vert", "oit/volumeresolve.frag", Shader::Build::No) {

    LGL_ERROR_CLASS;

    buildShaders();

    clear_.onReload([this]() { onReload_.invoke(); });
    display_.onReload([this]() { onReload_.invoke(); });

    abufferIdxTex_.initialize(nullptr);

    // create fragment query
    glGenQueries(1, &totalFragmentQuery_);

    LGL_ERROR_CLASS;
}

VolumeFragmentListRenderer::~VolumeFragmentListRenderer() {
    if (totalFragmentQuery_) glDeleteQueries(1, &totalFragmentQuery_);
}

void VolumeFragmentListRenderer::prePass(const size2_t& screenSize) {
    resizeBuffers(screenSize);

    // reset counter
    GLuint v[1] = {0};
    atomicCounter_.upload(v, sizeof(GLuint));
    atomicCounter_.unbind();

    // clear textures
    clear_.activate();
    auto& texUnit = textureUnits_.emplace_back();
    setUniforms(clear_, texUnit);

    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, GL_TRUE);
    utilgl::DepthMaskState depthMask(GL_TRUE);
    utilgl::DepthFuncState depthFunc(GL_ALWAYS);
    utilgl::CullFaceState culling(GL_NONE);
    utilgl::singleDrawImagePlaneRect();

    clear_.deactivate();

    // memory barrier
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);

    LGL_ERROR;
}

void VolumeFragmentListRenderer::beginCount() {
    // start query
    // The query is used to determinate the size needed for the shader storage buffer
    // to store all the fragments.
    glBeginQuery(GL_SAMPLES_PASSED, totalFragmentQuery_);
    LGL_ERROR;
}
void VolumeFragmentListRenderer::endCount() { glEndQuery(GL_SAMPLES_PASSED); }

bool VolumeFragmentListRenderer::postPass(
    bool useIllustration, const Image* background,
    std::function<void(Shader&, TextureUnitContainer&)> setUniformsCallback) {
    // memory barrier
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

    // get query result
    GLuint numFrags = 0;
    glGetQueryObjectuiv(totalFragmentQuery_, GL_QUERY_RESULT, &numFrags);
    LGL_ERROR;

    // check if enough space was available
    if (numFrags > fragmentSize_) {
        // we have to resize the fragment storage buffer
        fragmentSize_ = static_cast<size_t>(1.1f * numFrags);

        // unbind texture
        textureUnits_.clear();
        return false;
    }

    // Build shader depending on inport state.
    if (supportsFragmentLists() && static_cast<bool>(background) != builtWithBackground_) {
        buildShaders(background);
    }

    if (!useIllustration) {
        // render fragment list
        display_.activate();
        setUniforms(display_, textureUnits_[0]);
        LGL_ERROR;

        setUniformsCallback(display_, textureUnits_);
        LGL_ERROR;

        if (builtWithBackground_) {
            // Set depth buffer to read from.
            utilgl::bindAndSetUniforms(display_, textureUnits_, *background, "bg",
                                       ImageType::ColorDepth);
            display_.setUniform("reciprocalDimensions", vec2(1) / vec2(screenSize_));
        }
        LGL_ERROR;
        utilgl::BlendModeState blendModeStateGL(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        utilgl::GlBoolState depthTest(GL_DEPTH_TEST, GL_TRUE);
        utilgl::DepthMaskState depthMask(GL_TRUE);
        utilgl::DepthFuncState depthFunc(GL_ALWAYS);
        utilgl::CullFaceState culling(GL_NONE);
        LGL_ERROR;

        utilgl::singleDrawImagePlaneRect();
        LGL_ERROR;
        display_.deactivate();
    }
    textureUnits_.clear();

    return true;  // success, enough storage available
}

void VolumeFragmentListRenderer::setShaderUniforms(Shader& shader) const {
    setUniforms(shader, textureUnits_[0]);
}

void VolumeFragmentListRenderer::setUniforms(Shader& shader, const TextureUnit& abuffUnit) const {
    LGL_ERROR;
    // screen size textures

    abuffUnit.activate();

    abufferIdxTex_.bind();
    glBindImageTexture(abuffUnit.getUnitNumber(), abufferIdxTex_.getID(), 0, false, 0,
                       GL_READ_WRITE, GL_R32UI);

    shader.setUniform("abufferIdxImg", abuffUnit.getUnitNumber());
    glActiveTexture(GL_TEXTURE0);

    // pixel storage
    atomicCounter_.bindBase(6);
    pixelBuffer_.bindBase(7);
    LGL_ERROR;

    // other uniforms
    shader.setUniform("AbufferParams.screenWidth", static_cast<GLint>(screenSize_.x));
    shader.setUniform("AbufferParams.screenHeight", static_cast<GLint>(screenSize_.y));
    shader.setUniform("AbufferParams.storageSize", static_cast<GLuint>(fragmentSize_));
    LGL_ERROR;
}

bool VolumeFragmentListRenderer::supportsFragmentLists() {
    static const bool support =
        OpenGLCapabilities::getOpenGLVersion() >= 430 &&
        OpenGLCapabilities::isExtensionSupported("GL_NV_gpu_shader5") &&
        OpenGLCapabilities::isExtensionSupported("GL_EXT_shader_image_load_store") &&
        OpenGLCapabilities::isExtensionSupported("GL_NV_shader_buffer_load") &&
        OpenGLCapabilities::isExtensionSupported("GL_EXT_bindable_uniform");

    return support;
}

DispatcherHandle<void()> VolumeFragmentListRenderer::onReload(std::function<void()> callback) {
    return onReload_.add(callback);
}

void VolumeFragmentListRenderer::buildShaders(bool hasBackground) {
    builtWithBackground_ = hasBackground;
    auto* dfs = display_.getFragmentShaderObject();
    dfs->clearShaderExtensions();

    auto* cfs = clear_.getFragmentShaderObject();
    cfs->clearShaderExtensions();

    if (supportsFragmentLists()) {
        dfs->addShaderExtension("GL_NV_gpu_shader5", true);
        dfs->addShaderExtension("GL_EXT_shader_image_load_store", true);
        dfs->addShaderExtension("GL_NV_shader_buffer_load", true);
        dfs->addShaderExtension("GL_EXT_bindable_uniform", true);

        cfs->addShaderExtension("GL_NV_gpu_shader5", true);
        cfs->addShaderExtension("GL_EXT_shader_image_load_store", true);
        cfs->addShaderExtension("GL_NV_shader_buffer_load", true);
        cfs->addShaderExtension("GL_EXT_bindable_uniform", true);

        dfs->setShaderDefine("BACKGROUND_AVAILABLE", builtWithBackground_);
        display_.build();
        clear_.build();
    }
}

void VolumeFragmentListRenderer::resizeBuffers(const size2_t& screenSize) {
    if (screenSize != screenSize_) {
        screenSize_ = screenSize;
        // reallocate screen size texture that holds the pointer to the end of the fragment list at
        // that pixel
        abufferIdxTex_.resize(screenSize_);
    }

    const auto bufferSize = static_cast<GLsizeiptr>(fragmentSize_ * 4 * sizeof(GLfloat));
    if (pixelBuffer_.getSizeInBytes() != bufferSize) {
        // create new SSBO for the pixel storage
        pixelBuffer_.setSizeInBytes(bufferSize);
        pixelBuffer_.unbind();
    }
}

}  // namespace inviwo
