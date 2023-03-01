/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2023 Inviwo Foundation
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

#include <modules/meshrenderinggl/rendering/fragmentlistrenderer.h>

#include <inviwo/core/datastructures/image/imagetypes.h>  // for ImageType, ImageType::ColorDepth
#include <inviwo/core/util/dispatcher.h>                  // for Dispatcher, Dispatcher<>::Handle
#include <inviwo/core/util/glmvec.h>                      // for size2_t, vec2, vec4, ivec2
#include <modules/opengl/buffer/bufferobject.h>           // for BufferObject
#include <modules/opengl/glformats.h>                     // for GLFormats
#include <modules/opengl/inviwoopengl.h>                  // for GLfloat, GLuint, GLint, GL_ARRA...
#include <modules/opengl/openglcapabilities.h>            // for OpenGLCapabilities
#include <modules/opengl/openglutils.h>                   // for CullFaceState, DepthMaskState
#include <modules/opengl/shader/shader.h>                 // for Shader, Shader::Build
#include <modules/opengl/shader/shaderobject.h>           // for ShaderObject
#include <modules/opengl/texture/texture2d.h>             // for Texture2D
#include <modules/opengl/texture/textureunit.h>           // for TextureUnit, TextureUnitContainer
#include <modules/opengl/texture/textureutils.h>          // for singleDrawImagePlaneRect, bindA...

#include <algorithm>    // for min
#include <ostream>      // for operator<<, char_traits, basic_...
#include <string_view>  // for string_view
#include <vector>       // for vector

#include <fmt/core.h>                // for basic_string_view, format
#include <fmt/ostream.h>             // for print
#include <glm/detail/qualifier.hpp>  // for tvec2, tvec4
#include <glm/vec2.hpp>              // for vec<>::(anonymous), operator!=

namespace inviwo {
class Image;

FragmentListRenderer::Illustration::Illustration(size2_t screenSize, size_t fragmentSize)
    : index{screenSize, GL_RED, GL_R32F, GL_FLOAT, GL_NEAREST}
    , count{screenSize, GL_RED, GL_R32F, GL_FLOAT, GL_NEAREST}
    , color{fragmentSize * 2 * sizeof(GLfloat), GLFormats::getGLFormat(GL_FLOAT, 2),
            GL_DYNAMIC_DRAW, GL_SHADER_STORAGE_BUFFER}
    , surfaceInfo{fragmentSize * 2 * sizeof(GLfloat), GLFormats::getGLFormat(GL_FLOAT, 2),
                  GL_DYNAMIC_DRAW, GL_SHADER_STORAGE_BUFFER}
    , smoothing{{{fragmentSize * 2 * sizeof(GLfloat), GLFormats::getGLFormat(GL_FLOAT, 2),
                  GL_DYNAMIC_DRAW, GL_SHADER_STORAGE_BUFFER},
                 {fragmentSize * 2 * sizeof(GLfloat), GLFormats::getGLFormat(GL_FLOAT, 2),
                  GL_DYNAMIC_DRAW, GL_SHADER_STORAGE_BUFFER}}}
    , activeSmoothing{0}
    , fill{"oit/simplequad.vert", "illustration/sortandfill.frag", Shader::Build::No}
    , neighbors{"oit/simplequad.vert", "illustration/neighbors.frag", Shader::Build::No}
    , draw{"oit/simplequad.vert", "illustration/display.frag", Shader::Build::No}
    , smooth{"oit/simplequad.vert", "illustration/smooth.frag", Shader::Build::No}
    , settings{} {

    index.initialize(nullptr);
    count.initialize(nullptr);

    fill.onReload([this]() { onReload.invoke(); });
    neighbors.onReload([this]() { onReload.invoke(); });
    draw.onReload([this]() { onReload.invoke(); });
    smooth.onReload([this]() { onReload.invoke(); });
}

FragmentListRenderer::FragmentListRenderer()
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
    , display_("oit/simplequad.vert", "oit/display.frag", Shader::Build::No)
    , illustration_{screenSize_, fragmentSize_} {

    LGL_ERROR_CLASS;

    buildShaders();

    illustrationOnReload_ = illustration_.onReload.add([this]() { onReload_.invoke(); });
    clear_.onReload([this]() { onReload_.invoke(); });
    display_.onReload([this]() { onReload_.invoke(); });

    abufferIdxTex_.initialize(nullptr);

    // create fragment query
    glGenQueries(1, &totalFragmentQuery_);

    LGL_ERROR_CLASS;
}

FragmentListRenderer::~FragmentListRenderer() {
    if (totalFragmentQuery_) glDeleteQueries(1, &totalFragmentQuery_);
}

void FragmentListRenderer::prePass(const size2_t& screenSize) {
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

    // start query
    // The query is used to determinate the size needed for the shader storage buffer
    // to store all the fragments.
    glBeginQuery(GL_SAMPLES_PASSED, totalFragmentQuery_);
    LGL_ERROR;
}

bool FragmentListRenderer::postPass(bool useIllustration, const Image* background) {
    // memory barrier
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

    // get query result
    GLuint numFrags = 0;
    glEndQuery(GL_SAMPLES_PASSED);
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
    if (supportsFragmentLists() && static_cast<bool>(background) != builtWithBackground_)
        buildShaders(background);

    if (!useIllustration) {
        // render fragment list
        display_.activate();
        setUniforms(display_, textureUnits_[0]);
        if (builtWithBackground_) {
            // Set depth buffer to read from.
            utilgl::bindAndSetUniforms(display_, textureUnits_, *background, "bg",
                                       ImageType::ColorDepth);
            display_.setUniform("reciprocalDimensions", vec2(1) / vec2(screenSize_));
        }
        utilgl::BlendModeState blendModeStateGL(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        utilgl::GlBoolState depthTest(GL_DEPTH_TEST, GL_TRUE);
        utilgl::DepthMaskState depthMask(GL_TRUE);
        utilgl::DepthFuncState depthFunc(GL_ALWAYS);
        utilgl::CullFaceState culling(GL_NONE);
        utilgl::singleDrawImagePlaneRect();
        display_.deactivate();
    }

    // Note: illustration buffers are only called when enough space was available.
    // This allows us to drop the tests for overflow
    TextureUnit idxUnit;
    TextureUnit countUnit;
    if (useIllustration) {  // 1. copy to illustration buffer
        illustration_.resizeBuffers(screenSize_, fragmentSize_);
        fillIllustration(textureUnits_[0], idxUnit, countUnit, background);
    }

    // unbind texture with abuffer indices
    textureUnits_.clear();

    if (useIllustration) {  // 2. perform all the crazy post-processing steps
        illustration_.process(pixelBuffer_, idxUnit, countUnit);
        illustration_.render(idxUnit, countUnit);
    }

    return true;  // success, enough storage available
}

void FragmentListRenderer::setShaderUniforms(Shader& shader) {
    setUniforms(shader, textureUnits_[0]);
}

void FragmentListRenderer::setUniforms(Shader& shader, TextureUnit& abuffUnit) const {
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
}

bool FragmentListRenderer::supportsFragmentLists() {
    static const bool support =
        OpenGLCapabilities::getOpenGLVersion() >= 430 &&
        OpenGLCapabilities::isExtensionSupported("GL_NV_gpu_shader5") &&
        OpenGLCapabilities::isExtensionSupported("GL_EXT_shader_image_load_store") &&
        OpenGLCapabilities::isExtensionSupported("GL_NV_shader_buffer_load") &&
        OpenGLCapabilities::isExtensionSupported("GL_EXT_bindable_uniform");

    return support;
}

bool FragmentListRenderer::supportsIllustration() {
    static const bool support = []() {
        if (OpenGLCapabilities::getOpenGLVersion() >= 460)
            return true;
        else if (OpenGLCapabilities::getOpenGLVersion() >= 450)
            return OpenGLCapabilities::isExtensionSupported("GL_ARB_shader_atomic_counter_ops");
        else
            return false;
    }();

    return support;
}

typename Dispatcher<void()>::Handle FragmentListRenderer::onReload(std::function<void()> callback) {
    return onReload_.add(callback);
}

void FragmentListRenderer::buildShaders(bool hasBackground) {
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
    }

    auto* ffs = illustration_.fill.getFragmentShaderObject();
    ffs->setShaderExtension("GL_ARB_shader_atomic_counter_ops",
                            ShaderObject::ExtensionBehavior::Enable, supportsIllustration());

    if (supportsFragmentLists()) {
        dfs->setShaderDefine("BACKGROUND_AVAILABLE", builtWithBackground_);
        display_.build();
        clear_.build();
    }

    if (supportsIllustration()) {
        illustration_.fill.getFragmentShaderObject()->setShaderDefine("BACKGROUND_AVAILABLE",
                                                                      builtWithBackground_);
        illustration_.fill.build();
        illustration_.draw.build();
        illustration_.neighbors.build();
        illustration_.smooth.build();
    }
}

void FragmentListRenderer::resizeBuffers(const size2_t& screenSize) {
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

void FragmentListRenderer::Illustration::resizeBuffers(size2_t screenSize, size_t fragmentSize) {
    // reallocate textures with head and count
    if (index.getDimensions() != screenSize) {
        // reallocate screen size texture that holds the pointer to the begin of the block of
        // fragments
        index.resize(screenSize);
    }
    if (count.getDimensions() != screenSize) {
        // reallocate screen size texture that holds the count of fragments at that pixel
        count.resize(screenSize);
        count.bind();
    }

    const auto bufferSize = static_cast<GLsizeiptr>(fragmentSize * 2 * sizeof(GLfloat));
    if (color.getSizeInBytes() != bufferSize) {
        // reallocate SSBO for the illustration buffer storage
        // color: alpha+rgb
        color.setSizeInBytes(bufferSize);
        color.unbind();

        // surface info: depth, gradient, compressed normal (not yet)
        surfaceInfo.setSizeInBytes(bufferSize);
        surfaceInfo.unbind();

        // smoothing: beta + gamma
        for (int i = 0; i < 2; ++i) {
            smoothing[i].setSizeInBytes(bufferSize);
            smoothing[i].unbind();
        }
    }
}

void FragmentListRenderer::fillIllustration(TextureUnit& abuffUnit, TextureUnit& idxUnit,
                                            TextureUnit& countUnit, const Image* background) {
    // reset counter
    LGL_ERROR;
    GLuint v[1] = {0};
    atomicCounter_.upload(v, sizeof(GLuint));
    atomicCounter_.unbind();
    LGL_ERROR;

    // execute sort+fill shader
    illustration_.fill.activate();
    setUniforms(illustration_.fill, abuffUnit);
    illustration_.setUniforms(illustration_.fill, idxUnit, countUnit);
    if (builtWithBackground_) {
        utilgl::bindAndSetUniforms(illustration_.fill, textureUnits_, *background, "bg",
                                   ImageType::ColorDepth);
        illustration_.fill.setUniform("reciprocalDimensions", vec2(1) / vec2(screenSize_));
    }

    illustration_.color.bindBase(0);        // out: alpha + color
    illustration_.surfaceInfo.bindBase(1);  // out: depth + gradient
    atomicCounter_.bindBase(6);

    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, false);
    utilgl::DepthMaskState depthMask(GL_FALSE);
    utilgl::CullFaceState culling(GL_NONE);
    utilgl::singleDrawImagePlaneRect();

    illustration_.fill.deactivate();
}

void FragmentListRenderer::Illustration::process(BufferObject& pixelBuffer, TextureUnit& idxUnit,
                                                 TextureUnit& countUnit) {
    // resolve neighbors
    // and set initial conditions for silhouettes+halos
    neighbors.activate();
    setUniforms(neighbors, idxUnit, countUnit);
    surfaceInfo.bindBase(0);                     // in:  depth + gradient
    pixelBuffer.bindBase(1);                     // out: neighbors
    smoothing[1 - activeSmoothing].bindBase(2);  // out: beta + gamma
    activeSmoothing = 1 - activeSmoothing;

    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, false);
    utilgl::DepthMaskState depthMask(GL_FALSE);
    utilgl::CullFaceState culling(GL_NONE);
    utilgl::singleDrawImagePlaneRect();
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    neighbors.deactivate();

    // perform the bluring
    if (settings.smoothingSteps_ > 0) {
        smooth.activate();
        smooth.setUniform("lambdaBeta", 1.0f - settings.edgeSmoothing_);
        smooth.setUniform("lambdaGamma", 1.0f - settings.haloSmoothing_);
        for (int i = 0; i < settings.smoothingSteps_; ++i) {
            setUniforms(smooth, idxUnit, countUnit);
            pixelBuffer.bindBase(0);                     // in: neighbors
            smoothing[activeSmoothing].bindBase(1);      // in: beta + gamma
            smoothing[1 - activeSmoothing].bindBase(2);  // out: beta + gamma
            activeSmoothing = 1 - activeSmoothing;

            utilgl::singleDrawImagePlaneRect();

            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        }
        smooth.deactivate();
    }
}

void FragmentListRenderer::Illustration::render(TextureUnit& idxUnit, TextureUnit& countUnit) {
    // final blending
    draw.activate();
    setUniforms(draw, idxUnit, countUnit);
    surfaceInfo.bindBase(0);                 // in: depth + gradient
    color.bindBase(1);                       // in: alpha + color
    smoothing[activeSmoothing].bindBase(2);  // in: beta + gamma
    vec4 edgeColor = vec4(settings.edgeColor_, settings.edgeStrength_);
    draw.setUniform("edgeColor", edgeColor);
    draw.setUniform("haloStrength", settings.haloStrength_);

    utilgl::BlendModeState blendModeStateGL(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    utilgl::DepthMaskState depthMask(GL_TRUE);
    utilgl::DepthFuncState depthFunc(GL_ALWAYS);
    utilgl::CullFaceState culling(GL_NONE);
    utilgl::singleDrawImagePlaneRect();

    draw.deactivate();
}

void FragmentListRenderer::Illustration::setUniforms(Shader& shader, TextureUnit& idxUnit,
                                                     TextureUnit& countUnit) {
    idxUnit.activate();
    index.bind();
    glBindImageTexture(idxUnit.getUnitNumber(), index.getID(), 0, false, 0, GL_READ_WRITE,
                       GL_R32UI);
    shader.setUniform("illustrationBufferIdxImg", idxUnit.getUnitNumber());
    glActiveTexture(GL_TEXTURE0);

    countUnit.activate();
    count.bind();
    glBindImageTexture(countUnit.getUnitNumber(), count.getID(), 0, false, 0, GL_READ_WRITE,
                       GL_R32UI);
    shader.setUniform("illustrationBufferCountImg", countUnit.getUnitNumber());
    glActiveTexture(GL_TEXTURE0);

    shader.setUniform("screenSize", static_cast<ivec2>(index.getDimensions()));
}

void FragmentListRenderer::debugFragmentLists(std::ostream& oss) {
    oss << "========= Fragment List Renderer =========\n";

    // read global counter
    GLuint counter = 0xffffffff;
    atomicCounter_.bind();
    glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &counter);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    oss << "global counter: " << counter << "\n";

    GLuint numFrags = 0;
    glGetQueryObjectuiv(totalFragmentQuery_, GL_QUERY_RESULT, &numFrags);
    oss << "fragment query: " << numFrags << "\n";

    // read index image
    oss << "Index image:"
        << "\n";
    std::vector<GLuint> idxImg(screenSize_.x * screenSize_.y);
    abufferIdxTex_.download(idxImg.data());

    for (size_t y = 0; y < screenSize_.y; ++y) {
        oss << "y = " << y;
        for (size_t x = 0; x < screenSize_.x; ++x) {
            oss << " " << idxImg[x + screenSize_.x * y];
        }
        oss << "\n";
    }

    // read pixel storage buffer
    oss << "\n"
        << "Pixel storage: "
        << "\n";
    glBindBuffer(GL_ARRAY_BUFFER, pixelBuffer_.getId());

    const size_t size = std::min(static_cast<size_t>(counter), fragmentSize_);
    std::vector<GLfloat> pixelBuffer(4 * counter);
    glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 4 * size, pixelBuffer.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    for (size_t i = 0; i < size; ++i) {
        GLuint previous = *reinterpret_cast<GLuint*>(&pixelBuffer[4 * i]);
        GLfloat depth = pixelBuffer[4 * i + 1];
        GLfloat alpha = pixelBuffer[4 * i + 2];
        GLuint c = *reinterpret_cast<GLuint*>(&pixelBuffer[4 * i + 3]);
        float r = float((c >> 20) & 0x3ff) / 1023.0f;
        float g = float((c >> 10) & 0x3ff) / 1023.0f;
        float b = float(c & 0x3ff) / 1023.0f;
        oss << fmt::format(
            "{:5}: previous={:5}, depth={:6.3f}, alpha={:5.3f}, r={:5.3f}, g={:5.3f}, b={:5.3f}\n",
            i, (int)previous, (float)depth, (float)alpha, r, g, b);
    }

    oss << std::endl << "\n==================================================\n";
}

struct ColorPack {
    float a;
    unsigned int b : 10;
    unsigned int g : 10;
    unsigned int r : 10;
    unsigned int unused : 2;
};
static_assert(sizeof(ColorPack) == 8);

void FragmentListRenderer::debugIllustrationBuffer(std::ostream& oss) {
    oss << "========= Fragment List Renderer - Illustration Buffers =========\n";

    // read images
    std::vector<GLuint> idxImg(screenSize_.x * screenSize_.y);
    illustration_.index.download(&idxImg[0]);

    std::vector<GLuint> countImg(screenSize_.x * screenSize_.y);
    illustration_.count.download(&countImg[0]);

    // read pixel storage buffer
    GLuint numFrags = 0;
    glGetQueryObjectuiv(totalFragmentQuery_, GL_QUERY_RESULT, &numFrags);
    size_t size = std::min(static_cast<size_t>(numFrags), fragmentSize_);

    glBindBuffer(GL_ARRAY_BUFFER, illustration_.color.getId());
    std::vector<ColorPack> colorBuffer(size);
    glGetBufferSubData(GL_ARRAY_BUFFER, 0, 2 * sizeof(GLfloat) * size, &colorBuffer[0]);

    glBindBuffer(GL_ARRAY_BUFFER, illustration_.surfaceInfo.getId());
    std::vector<glm::tvec2<GLfloat>> surfaceInfoBuffer(size);
    glGetBufferSubData(GL_ARRAY_BUFFER, 0, 2 * sizeof(GLfloat) * size, &surfaceInfoBuffer[0]);

    glBindBuffer(GL_ARRAY_BUFFER, pixelBuffer_.getId());
    std::vector<glm::tvec4<GLint>> neighborBuffer(size);
    glGetBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(GLint) * size, &neighborBuffer[0]);

    glBindBuffer(GL_ARRAY_BUFFER,
                 illustration_.smoothing[1 - illustration_.activeSmoothing].getId());
    std::vector<glm::tvec2<GLfloat>> smoothingBuffer(size);
    glGetBufferSubData(GL_ARRAY_BUFFER, 0, 2 * sizeof(GLfloat) * size, &smoothingBuffer[0]);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // print
    for (size_t y = 0; y < screenSize_.y; ++y) {
        for (size_t x = 0; x < screenSize_.x; ++x) {
            const size_t start = idxImg[x + screenSize_.x * y];
            const size_t count = countImg[x + screenSize_.x * y];
            fmt::print(oss, "{: <4}:{: <4} start={: <5}, count={: <5}\n", x, y, start, count);

            for (size_t i = 0; i < count; ++i) {
                auto color = colorBuffer[start + i];
                float depth = surfaceInfoBuffer[start + i].x;
                glm::tvec4<GLint> neighbors = neighborBuffer[start + i];
                float beta = smoothingBuffer[start + i].x;
                float gamma = smoothingBuffer[start + i].y;
                float r = color.r / 1023.0f;
                float g = color.g / 1023.0f;
                float b = color.b / 1023.0f;

                fmt::print(oss,
                           "\tdepth={:5.3f}, alpha={:5.3f}, r={:5.3f}, g={:5.3f}, b={:5.3f}, "
                           "beta={:5.3f}, gamma={:5.3f}, neighbors:",
                           depth, color.a, r, g, b, beta, gamma);

                for (size_t n = 0; n < 4; ++n) {
                    if (neighbors[n] >= 0) {
                        if (neighbors[n] < static_cast<int>(size)) {
                            fmt::print(oss, "{}: {:5.3f}", neighbors[n],
                                       surfaceInfoBuffer[neighbors[n]].x);
                        } else {
                            fmt::print(oss, "(>size)");
                        }
                    } else {
                        fmt::print(oss, "(-1)");
                    }
                }
                fmt::print(oss, "\n");
            }
        }
    }

    oss << "\n==================================================\n";
}

}  // namespace inviwo
