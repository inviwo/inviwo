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

/*-----------------------------------------------------------------------
Copyright (c) 2014-2015, NVIDIA. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Neither the name of its contributors may be used to endorse
or promote products derived from this software without specific
prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------*/

#include <modules/postprocessing/processors/ssao.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/sharedopenglresources.h>
#include <modules/opengl/geometry/meshgl.h>
#include <random>

namespace inviwo {

#define USE_AO_SPECIAL_BLUR 1
static vec4 hbaoRandom[SSAO::HBAO_RANDOM_ELEMENTS * SSAO::MAX_SAMPLES];

static void newTexture(GLuint& id) {
    if (id) glDeleteTextures(1, &id);
    glGenTextures(1, &id);
}

static void newFramebuffer(GLuint& id) {
    if (id) glDeleteFramebuffers(1, &id);
    glGenFramebuffers(1, &id);
}

static void delTexture(GLuint id) {
    if (id) glDeleteTextures(1, &id);
}

static void delFramebuffer(GLuint id) {
    if (id) glDeleteFramebuffers(1, &id);
}

static void delBuffer(GLuint id) {
    if (id) glDeleteBuffers(1, &id);
}

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo SSAO::processorInfo_{
    "org.inviwo.SSAO",                                         // Class identifier
    "SSAO",                                                    // Display name
    "Postprocessing",                                          // Category
    CodeState::Stable,                                         // Code state
    "GL, Postprocessing, Image Operation, Ambient Occlusion",  // Tags
};

const ProcessorInfo SSAO::getProcessorInfo() const { return processorInfo_; }

SSAO::SSAO()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , enable_("enable", "Enable SSAO", true)
    , technique_("option", "SSAO Technique")
    , radius_("radius", "Radius", 2.f, 0.f, 128.f, 0.05f)
    , intensity_("intensity", "Intensity", 1.5f, 0.f, 5.f)
    , bias_("bias", "Angle Bias", 0.1f, 0.f, 0.5f, 0.01f)
    , directions_("directions", "Directions", 8, 4, 32)
    , steps_("steps", "Steps / Dir", 4, 2, 32)
    , useNormal_("normal", "Use Normal", true)
    , enableBlur_("enable-blur", "Enable Blur", true)
    , blurSharpness_("blur-sharpness", "Blur Sharpness", 40.f, 0.f, 200.f)
    , camera_("camera", "Camera")
    , depthLinearize_("fullscreenquad.vert", "depthlinearize.frag", false)
    , hbaoCalc_("fullscreenquad.vert", "hbao.frag", false)
    , hbaoCalcBlur_("fullscreenquad.vert", "hbao.frag", false)
    , hbaoBlurHoriz_("fullscreenquad.vert", "hbao_blur.frag", false)
    , hbaoBlurVert_("fullscreenquad.vert", "hbao_blur.frag", false)
    , hbaoUbo_(0) {

    technique_.addOption("hbao-classic", "HBAO Classic", 1);
    technique_.set(1);
    technique_.setCurrentStateAsDefault();

    addPort(inport_);
    addPort(outport_);
    addProperty(enable_);
    addProperty(technique_);
    addProperty(radius_);
    addProperty(intensity_);
    addProperty(bias_);
    addProperty(directions_);
    addProperty(steps_);
    addProperty(useNormal_);
    addProperty(blurSharpness_);
    addProperty(enableBlur_);
    addProperty(camera_);

    initHbao();

    initializeResources();
    directions_.onChange([this]() { invalidate(InvalidationLevel::InvalidResources); });
    steps_.onChange([this]() { invalidate(InvalidationLevel::InvalidResources); });
    useNormal_.onChange([this]() { invalidate(InvalidationLevel::InvalidResources); });
    depthLinearize_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    hbaoCalc_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    hbaoCalcBlur_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    hbaoBlurHoriz_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    hbaoBlurVert_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    inport_.onChange([this]() {
        const DataFormatBase* format = inport_.getData()->getDataFormat();
        const auto swizzleMask = inport_.getData()->getColorLayer()->getSwizzleMask();

        if (!outport_.hasEditableData() || format != outport_.getData()->getDataFormat() ||
            swizzleMask != outport_.getData()->getColorLayer()->getSwizzleMask()) {
            auto dim = outport_.getData()->getDimensions();
            Image* img = new Image(dim, format);
            img->copyMetaDataFrom(*inport_.getData());
            // forward swizzle mask of the input
            img->getColorLayer()->setSwizzleMask(swizzleMask);

            outport_.setData(img);
        }
    });
}

SSAO::~SSAO() {
    delFramebuffer(framebuffers_.depthLinear);
    delFramebuffer(framebuffers_.hbaoCalc);

    delTexture(textures_.depthLinear);
    delTexture(textures_.hbaoResult);
    delTexture(textures_.hbaoBlur);
    delTexture(textures_.hbaoRandom);

    delBuffer(hbaoUbo_);
}

void SSAO::initializeResources() {
    depthLinearize_[ShaderType::Fragment]->addShaderDefine("DEPTHLINEARIZE_MSAA", "0");
    depthLinearize_.build();
    hbaoCalc_[ShaderType::Fragment]->addShaderDefine("AO_DEINTERLEAVED", "0");
    hbaoCalc_[ShaderType::Fragment]->addShaderDefine("AO_BLUR", "0");
    hbaoCalc_[ShaderType::Fragment]->addShaderDefine("AO_LAYERED", "0");
    hbaoCalc_[ShaderType::Fragment]->addShaderDefine("AO_DIRS", std::to_string(directions_.get()));
    hbaoCalc_[ShaderType::Fragment]->addShaderDefine("AO_STEPS", std::to_string(steps_.get()));
    hbaoCalc_[ShaderType::Fragment]->addShaderDefine("AO_USE_NORMAL",
                                                     std::to_string(useNormal_.get() ? 1 : 0));
    hbaoCalc_.build();
    hbaoCalcBlur_[ShaderType::Fragment]->addShaderDefine("AO_DEINTERLEAVED", "0");
    hbaoCalcBlur_[ShaderType::Fragment]->addShaderDefine("AO_BLUR", "1");
    hbaoCalcBlur_[ShaderType::Fragment]->addShaderDefine("AO_LAYERED", "0");
    hbaoCalcBlur_[ShaderType::Fragment]->addShaderDefine("AO_DIRS",
                                                         std::to_string(directions_.get()));
    hbaoCalcBlur_[ShaderType::Fragment]->addShaderDefine("AO_STEPS", std::to_string(steps_.get()));
    hbaoCalcBlur_[ShaderType::Fragment]->addShaderDefine("AO_USE_NORMAL",
                                                         std::to_string(useNormal_.get() ? 1 : 0));
    hbaoCalcBlur_.build();
    hbaoBlurHoriz_[ShaderType::Fragment]->addShaderDefine("AO_BLUR_PRESENT", "0");
    hbaoBlurHoriz_.build();
    hbaoBlurVert_[ShaderType::Fragment]->addShaderDefine("AO_BLUR_PRESENT", "1");
    hbaoBlurVert_.build();

    locations_.depthLinearClipInfo = glGetUniformLocation(depthLinearize_.getID(), "clipInfo");
    locations_.depthLinearInputTexture =
        glGetUniformLocation(depthLinearize_.getID(), "inputTexture");
    locations_.hbaoControlBuffer = glGetUniformBlockIndex(hbaoCalc_.getID(), "controlBuffer");
    locations_.hbaoTexLinearDepth = glGetUniformLocation(hbaoCalc_.getID(), "texLinearDepth");
    locations_.hbaoTexRandom = glGetUniformLocation(hbaoCalc_.getID(), "texRandom");
    locations_.hbaoBlurSharpness = glGetUniformLocation(hbaoBlurHoriz_.getID(), "g_Sharpness");
    locations_.hbaoBlurInvResolutionDirection =
        glGetUniformLocation(hbaoBlurHoriz_.getID(), "g_InvResolutionDirection");
    locations_.hbaoBlurTexSource = glGetUniformLocation(hbaoBlurHoriz_.getID(), "texSource");
}

void SSAO::process() {
    if (!enable_.get()) {
        outport_.setData(inport_.getData());
        return;
    }

    int width = static_cast<int>(outport_.getDimensions().x);
    int height = static_cast<int>(outport_.getDimensions().y);

    if (framebuffers_.width != width || framebuffers_.height != height) {
        initFramebuffers(width, height);
    }

    projParam_.nearplane = camera_.getNearPlaneDist();
    projParam_.farplane = camera_.getFarPlaneDist();
    projParam_.matrix = camera_.projectionMatrix();

    auto persp_camera = dynamic_cast<PerspectiveCamera*>(&camera_.get());
    if (persp_camera) {
        projParam_.ortho = 0;
        projParam_.fov = glm::radians(persp_camera->getFovy());
    } else {
        projParam_.ortho = 1;
        projParam_.orthoheight = static_cast<float>(height);
    }

    utilgl::activateTargetAndCopySource(outport_, inport_);

    auto imageGL = inport_.getData()->getRepresentation<ImageGL>();
    auto depthLayer = imageGL->getDepthLayerGL();
    auto depthTex = depthLayer->getTexture()->getID();

    auto outImageGL = outport_.getEditableData()->getRepresentation<ImageGL>();
    auto outFbo = outImageGL->getFBO()->getID();

    if (technique_.get() == 1) {
        // This geometry is actually never used, but a valid VAO and VBO is required to kick off the
        // drawcalls
        auto rect = SharedOpenGLResources::getPtr()->imagePlaneRect();
        utilgl::Enable<MeshGL> enable(rect);
        drawHbaoClassic(outFbo, depthTex, projParam_, width, height);
    }
}

void SSAO::initHbao() {
    constexpr float numDir = 8;  // keep in sync to glsl

    std::mt19937 gen;
    std::uniform_real_distribution<float> uni_float(0.f, 1.f);

    auto random = [&gen, &uni_float]() -> float { return uni_float(gen); };

    signed short hbaoRandomShort[HBAO_RANDOM_ELEMENTS * MAX_SAMPLES * 4];

    for (int i = 0; i < HBAO_RANDOM_ELEMENTS * MAX_SAMPLES; i++) {
        float rand1 = random();
        float rand2 = random();

        // Use random rotation angles in [0,2PI/NUM_DIRECTIONS)
        float angle = 2.f * glm::pi<float>() * rand1 / numDir;
        hbaoRandom[i].x = glm::cos(angle);
        hbaoRandom[i].y = glm::sin(angle);
        hbaoRandom[i].z = rand2;
        hbaoRandom[i].w = 0;
#define SCALE ((1 << 15))
        hbaoRandomShort[i * 4 + 0] = (signed short)(SCALE * hbaoRandom[i].x);
        hbaoRandomShort[i * 4 + 1] = (signed short)(SCALE * hbaoRandom[i].y);
        hbaoRandomShort[i * 4 + 2] = (signed short)(SCALE * hbaoRandom[i].z);
        hbaoRandomShort[i * 4 + 3] = (signed short)(SCALE * hbaoRandom[i].w);
#undef SCALE
    }

    newTexture(textures_.hbaoRandom);
    glBindTexture(GL_TEXTURE_2D, textures_.hbaoRandom);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16_SNORM, HBAO_RANDOM_SIZE, HBAO_RANDOM_SIZE);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, HBAO_RANDOM_SIZE, HBAO_RANDOM_SIZE, GL_RGBA, GL_SHORT,
                    hbaoRandomShort);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenBuffers(1, &hbaoUbo_);
    glBindBuffer(GL_UNIFORM_BUFFER, hbaoUbo_);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(HBAOData), nullptr, GL_DYNAMIC_DRAW);
}

void SSAO::initFramebuffers(int width, int height) {
    framebuffers_.width = width;
    framebuffers_.height = height;

    newTexture(textures_.depthLinear);
    glBindTexture(GL_TEXTURE_2D, textures_.depthLinear);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    newFramebuffer(framebuffers_.depthLinear);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers_.depthLinear);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures_.depthLinear, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

#if USE_AO_SPECIAL_BLUR
    GLenum formatAO = GL_RG16F;
    GLint swizzle[4] = {GL_RED, GL_GREEN, GL_ZERO, GL_ZERO};
#else
    GLenum formatAO = GL_R8;
    GLint swizzle[4] = {GL_RED, GL_RED, GL_RED, GL_RED};
#endif

    newTexture(textures_.hbaoResult);
    glBindTexture(GL_TEXTURE_2D, textures_.hbaoResult);
    glTexStorage2D(GL_TEXTURE_2D, 1, formatAO, width, height);
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    newTexture(textures_.hbaoBlur);
    glBindTexture(GL_TEXTURE_2D, textures_.hbaoBlur);
    glTexStorage2D(GL_TEXTURE_2D, 1, formatAO, width, height);
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    newFramebuffer(framebuffers_.hbaoCalc);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers_.hbaoCalc);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures_.hbaoResult, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, textures_.hbaoBlur, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSAO::prepareHbaoData(const ProjectionParam& proj, int width, int height) {
    // projection
    const float* P = &proj.matrix[0][0];

    vec4 projInfoPerspective{
        2.0f / (P[4 * 0 + 0]),                  // (x) * (R - L)/N
        2.0f / (P[4 * 1 + 1]),                  // (y) * (T - B)/N
        -(1.0f - P[4 * 2 + 0]) / P[4 * 0 + 0],  // L/N
        -(1.0f + P[4 * 2 + 1]) / P[4 * 1 + 1],  // B/N
    };

    vec4 projInfoOrtho{
        2.0f / (P[4 * 0 + 0]),                  // ((x) * R - L)
        2.0f / (P[4 * 1 + 1]),                  // ((y) * T - B)
        -(1.0f + P[4 * 3 + 0]) / P[4 * 0 + 0],  // L
        -(1.0f - P[4 * 3 + 1]) / P[4 * 1 + 1],  // B
    };

    int useOrtho = proj.ortho ? 1 : 0;
    hbaoUboData_.projOrtho = useOrtho;
    hbaoUboData_.projInfo = useOrtho ? projInfoOrtho : projInfoPerspective;

    float projScale;
    if (useOrtho) {
        projScale = float(height) / (projInfoOrtho[1]);
    } else {
        projScale = float(height) / (tanf(proj.fov * 0.5f) * 2.0f);
    }

    // radius
    float meters2viewspace = 1.0f;
    float R = radius_.get() * meters2viewspace;
    hbaoUboData_.R2 = R * R;
    hbaoUboData_.NegInvR2 = -1.0f / hbaoUboData_.R2;
    hbaoUboData_.RadiusToScreen = R * 0.5f * projScale;

    // ao
    hbaoUboData_.PowExponent = std::max(intensity_.get(), 0.0f);
    hbaoUboData_.NDotVBias = glm::clamp(bias_.get(), 0.0f, 1.0f);
    hbaoUboData_.AOMultiplier = 1.0f / (1.0f - hbaoUboData_.NDotVBias);

    // resolution
    int quarterWidth = ((width + 3) / 4);
    int quarterHeight = ((height + 3) / 4);

    hbaoUboData_.InvQuarterResolution =
        vec2(1.0f / float(quarterWidth), 1.0f / float(quarterHeight));
    hbaoUboData_.InvFullResolution = vec2(1.0f / float(width), 1.0f / float(height));

    for (int i = 0; i < HBAO_RANDOM_ELEMENTS; i++) {
        hbaoUboData_.float2Offsets[i] = vec4(float(i % 4) + 0.5f, float(i / 4) + 0.5f, 0, 0);
        hbaoUboData_.jitters[i] = hbaoRandom[i];
    }
}

void SSAO::drawLinearDepth(GLuint depthTex, const ProjectionParam& proj) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers_.depthLinear);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    depthLinearize_.activate();
    glUniform4f(locations_.depthLinearClipInfo, proj.nearplane * proj.farplane,
                proj.nearplane - proj.farplane, proj.farplane, proj.ortho ? 0.0f : 1.0f);
    glUniform1i(locations_.depthLinearInputTexture, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void SSAO::drawHbaoClassic(GLuint fboOut, GLuint depthTex, const ProjectionParam& proj, int width,
                           int height) {
    prepareHbaoData(proj, width, height);
    glViewport(0, 0, width, height);
    drawLinearDepth(depthTex, proj);

    glBindFramebuffer(GL_FRAMEBUFFER, fboOut);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glColorMask(1, 1, 1, 0);

    auto blur = enableBlur_.get();

    if (blur) {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers_.hbaoCalc);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, fboOut);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ZERO, GL_SRC_COLOR);
    }

    GLuint program = 0;

    if (blur) {
        hbaoCalcBlur_.activate();
        program = hbaoCalcBlur_.getID();
    } else {
        hbaoCalc_.activate();
        program = hbaoCalc_.getID();
    }

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, hbaoUbo_);
    glBindBuffer(GL_UNIFORM_BUFFER, hbaoUbo_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(HBAOData), &hbaoUboData_);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Uniform Bindings
    glUniformBlockBinding(program, locations_.hbaoControlBuffer, 0);
    glUniform1i(locations_.hbaoTexLinearDepth, 0);
    glUniform1i(locations_.hbaoTexRandom, 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures_.depthLinear);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures_.hbaoRandom);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    if (blur) drawHbaoBlur(fboOut, proj, width, height);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glColorMask(1, 1, 1, 1);

    glUseProgram(0);
}

void SSAO::drawHbaoBlur(GLuint fboOut, const ProjectionParam& /*proj*/, int width, int height) {
    auto meters2viewspace = 1.0f;
    auto sharpness = blurSharpness_.get();

    glDrawBuffer(GL_COLOR_ATTACHMENT1);

    // --- HORIZONTAL PASS ---
    hbaoBlurHoriz_.activate();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures_.hbaoResult);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures_.depthLinear);

    // Uniforms
    glUniform1f(locations_.hbaoBlurSharpness, sharpness / meters2viewspace);
    glUniform2f(locations_.hbaoBlurInvResolutionDirection, 1.0f / float(width), 0);
    glUniform1i(locations_.hbaoBlurTexSource, 0);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    // final output to main fbo
    glBindFramebuffer(GL_FRAMEBUFFER, fboOut);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ZERO, GL_SRC_COLOR);

    // --- VERTICAL PASS ---
    hbaoBlurVert_.activate();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures_.hbaoBlur);

    // Uniforms
    glUniform1f(locations_.hbaoBlurSharpness, sharpness / meters2viewspace);
    glUniform2f(locations_.hbaoBlurInvResolutionDirection, 0, 1.0f / float(height));
    glUniform1i(locations_.hbaoBlurTexSource, 0);

    glDrawArrays(GL_TRIANGLES, 0, 3);
}

}  // namespace inviwo
