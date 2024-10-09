/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/opactopt/rendering/abufferopacityoptimisationrenderer.h>

#include <modules/opengl/openglutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opactopt/algorithm/gaussian.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/common/inviwomodule.h>  // for InviwoModule

namespace inviwo {

AbufferOpacityOptimisationRenderer::AbufferOpacityOptimisationRenderer(
    const Approximations::ApproximationProperties* p, CameraProperty* c, int isc, int odc,
    int gaussianRadius, float gaussianSigma)
    : OpacityOptimisationRenderer(c)
    , ap_(p)
    , nImportanceSumCoefficients_(isc)
    , nOpticalDepthCoefficients_(odc)
    , project_{"oit/simplequad.vert", "opactopt/approximate/project.frag", Shader::Build::No}
    , smoothH_{"oit/simplequad.vert", "opactopt/approximate/smooth.frag", Shader::Build::No}
    , smoothV_{"oit/simplequad.vert", "opactopt/approximate/smooth.frag", Shader::Build::No}
    , blend_{"oit/simplequad.vert", "opactopt/approximate/blend.frag", Shader::Build::No}
    , clearaoo_{"oit/simplequad.vert", "opactopt/approximate/clear.frag", Shader::Build::No}
    , importanceSumTexture_{{size3_t(screenSize_.x, screenSize_.y, isc), imageFormat_, GL_NEAREST},
                            {size3_t(screenSize_.x, screenSize_.y, isc), imageFormat_, GL_NEAREST}}
    , opticalDepthTexture_{size3_t(screenSize_.x, screenSize_.y, odc), imageFormat_, GL_NEAREST}
    , gaussianKernel_{64 * sizeof(float),                   // allocate largeish size
                      GLFormats::getGLFormat(GL_FLOAT, 1),  // dummy format
                      GL_STATIC_DRAW, GL_SHADER_STORAGE_BUFFER}
    , gaussianRadius_(gaussianRadius)
    , gaussianSigma_(gaussianSigma)
    , legendreCoefficients_{
          Approximations::approximations.at("legendre").maxCoefficients * sizeof(int),
          GLFormats::getGLFormat(GL_INT, 1),  // dummy format
          GL_STATIC_DRAW, GL_SHADER_STORAGE_BUFFER} {

    for (auto& ist : importanceSumTexture_) ist.initialize(nullptr);
    opticalDepthTexture_.initialize(nullptr);

    generateAndUploadGaussianKernel(gaussianRadius, gaussianSigma);
    generateAndUploadLegendreCoefficients();
    legendreCoefficients_.bindBase(9);

    project_.onReload([this]() { onReload_.invoke(); });
    smoothH_.onReload([this]() { onReload_.invoke(); });
    smoothV_.onReload([this]() { onReload_.invoke(); });
    blend_.onReload([this]() { onReload_.invoke(); });
    clearaoo_.onReload([this]() { onReload_.invoke(); });

    buildShaders();
}

void AbufferOpacityOptimisationRenderer::prePass(const size2_t& screenSize) {
    resizeBuffers(screenSize);

    // reset counter
    GLuint v[1] = {0};
    atomicCounter_.upload(v, sizeof(GLuint));
    atomicCounter_.unbind();

    // clear textures
    clearaoo_.activate();
    abuffUnit_ = &textureUnits_.emplace_back();

    importanceSumUnitMain_ = &textureUnits_.emplace_back();
    importanceSumUnitMain_->activate();
    importanceSumTexture_[0].bind();
    glBindImageTexture(importanceSumUnitMain_->getUnitNumber(), importanceSumTexture_[0].getID(), 0,
                       true, 0, GL_READ_WRITE, imageFormat_.internalFormat);
    clearaoo_.setUniform("importanceSumCoeffs[0]", importanceSumUnitMain_->getUnitNumber());

    if (smoothing) {
        importanceSumUnitSmooth_ = &textureUnits_.emplace_back();
        importanceSumUnitSmooth_->activate();
        importanceSumTexture_[1].bind();
        glBindImageTexture(importanceSumUnitSmooth_->getUnitNumber(),
                           importanceSumTexture_[1].getID(), 0, true, 0, GL_READ_WRITE, imageFormat_.internalFormat);
        clearaoo_.setUniform("importanceSumCoeffs[1]", importanceSumUnitSmooth_->getUnitNumber());
    }

    opticalDepthUnit_ = &textureUnits_.emplace_back();
    opticalDepthUnit_->activate();
    opticalDepthTexture_.bind();
    glBindImageTexture(opticalDepthUnit_->getUnitNumber(), opticalDepthTexture_.getID(), 0, true, 0,
                       GL_READ_WRITE, imageFormat_.internalFormat);

    setUniforms(clearaoo_, *abuffUnit_);
    utilgl::singleDrawImagePlaneRect();

    clearaoo_.deactivate();

    // memory barrier
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);

    LGL_ERROR;
}

bool AbufferOpacityOptimisationRenderer::postPass(bool useIllustration, const Image* background) {
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
    *nfrags = numFrags;
    execTimer->addCounter();

    // Build shader depending on inport state.
    if ((supportsFragmentLists() && static_cast<bool>(background) != builtWithBackground_) ||
        importanceVolumeDirty || debug) {
        buildShaders(background);
        importanceVolumeDirty = false;
    }

    process();
    render(background);

    textureUnits_.clear();

    return true;  // success, enough storage available
}

// Perform projection of optical depth coefficients and gaussian smoothing
void AbufferOpacityOptimisationRenderer::process() {
    // states for projection and smoothing steps
    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, false);
    utilgl::DepthMaskState depthMask(GL_FALSE);
    utilgl::CullFaceState culling(GL_NONE);

    // project importance sum
    project_.activate();
    setUniforms(project_, *abuffUnit_);
    if (importanceVolume && importanceVolume->hasData())
        utilgl::bindAndSetUniforms(project_, textureUnits_, *importanceVolume);
    project_.setUniform("reciprocalDimensions", vec2(1) / vec2(screenSize_));
    utilgl::singleDrawImagePlaneRect();
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
    execTimer->addCounter();

    if (smoothing) {
        // smoothing importance
        gaussianKernel_.bindBase(8);

        // horizontal pass
        smoothH_.activate();
        smoothH_.setUniform("radius", gaussianRadius_);
        setUniforms(smoothH_, *abuffUnit_);
        utilgl::singleDrawImagePlaneRect();
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // vertical pass
        smoothV_.activate();
        smoothV_.setUniform("radius", gaussianRadius_);
        setUniforms(smoothV_, *abuffUnit_);
        utilgl::singleDrawImagePlaneRect();
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
    execTimer->addCounter();
}

void AbufferOpacityOptimisationRenderer::render(const Image* background) {
    if (debug) db_.initialiseDebugBuffer();

    // final blending
    blend_.activate();
    setUniforms(blend_, *abuffUnit_);
    if (builtWithBackground_) {
        // Set depth buffer to read from.
        utilgl::bindAndSetUniforms(blend_, textureUnits_, *background, "bg", ImageType::ColorDepth);
        blend_.setUniform("reciprocalDimensions", vec2(1) / vec2(screenSize_));
    }
    utilgl::BlendModeState blendModeStateGL(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, GL_TRUE);
    utilgl::DepthMaskState depthMask(GL_TRUE);
    utilgl::DepthFuncState depthFunc(GL_ALWAYS);
    utilgl::CullFaceState culling(GL_NONE);
    utilgl::singleDrawImagePlaneRect();
    execTimer->addCounter();

    if (debug) db_.retrieveDebugInfo(nImportanceSumCoefficients_, nOpticalDepthCoefficients_);
}

void AbufferOpacityOptimisationRenderer::buildShaders(bool hasBackground) {
    builtWithBackground_ = hasBackground;

    auto* pfs = project_.getFragmentShaderObject();
    auto* shfs = smoothH_.getFragmentShaderObject();
    auto* svfs = smoothV_.getFragmentShaderObject();
    auto* bfs = blend_.getFragmentShaderObject();
    auto* cfs = clearaoo_.getFragmentShaderObject();

    for (auto& fs : {pfs, shfs, svfs, bfs, cfs}) {
        fs->clearShaderExtensions();
        fs->clearShaderDefines();
        fs->addShaderExtension("GL_NV_gpu_shader5", true);
        fs->addShaderExtension("GL_EXT_shader_image_load_store", true);
        fs->addShaderExtension("GL_NV_shader_buffer_load", true);
        fs->addShaderExtension("GL_EXT_bindable_uniform", true);
        fs->setShaderDefine("N_IMPORTANCE_SUM_COEFFICIENTS", true,
                            std::to_string(nImportanceSumCoefficients_).c_str());
        fs->setShaderDefine("N_OPTICAL_DEPTH_COEFFICIENTS", true,
                            std::to_string(nOpticalDepthCoefficients_).c_str());
        fs->setShaderDefine("USE_ABUFFER", true);
        if (debug) fs->setShaderDefine("DEBUG", true);
    }

    for (auto& fs : {pfs, bfs}) {
        fs->setShaderDefine(ap_->shaderDefineName.c_str(), true);
    }

    pfs->setShaderDefine("USE_IMPORTANCE_VOLUME", importanceVolume && importanceVolume->hasData());

    shfs->setShaderDefine("HORIZONTAL", true, "1");
    svfs->setShaderDefine("HORIZONTAL", true, "0");

    bfs->setShaderDefine("USE_EXACT_BLENDING", useExactBlending_);
    bfs->setShaderDefine("NORMALISE", normalisedBlending_);
    bfs->setShaderDefine("BACKGROUND_AVAILABLE", builtWithBackground_);

    project_.build();
    smoothH_.build();
    smoothV_.build();
    blend_.build();
    clearaoo_.build();
}

void AbufferOpacityOptimisationRenderer::setDescriptor(
    const Approximations::ApproximationProperties* p) {
    ap_ = p;
    buildShaders(builtWithBackground_);
}

void AbufferOpacityOptimisationRenderer::setImportanceSumCoeffs(int isc) {
    if (nImportanceSumCoefficients_ != isc) {
        importanceSumTexture_[0].uploadAndResize(nullptr,
                                                 size3_t(screenSize_.x, screenSize_.y, isc));
        importanceSumTexture_[1].uploadAndResize(nullptr,
                                                 size3_t(screenSize_.x, screenSize_.y, isc));
        nImportanceSumCoefficients_ = isc;
        buildShaders(builtWithBackground_);
    }
}

void AbufferOpacityOptimisationRenderer::setOpticalDepthCoeffs(int odc) {
    if (nOpticalDepthCoefficients_ != odc) {
        opticalDepthTexture_.uploadAndResize(nullptr, size3_t(screenSize_.x, screenSize_.y, odc));
        nOpticalDepthCoefficients_ = odc;
        buildShaders(builtWithBackground_);
    }
}

void AbufferOpacityOptimisationRenderer::setExactBlending(bool eb) {
    if (useExactBlending_ != eb) {
        useExactBlending_ = eb;
        buildShaders(builtWithBackground_);
    }
}

void AbufferOpacityOptimisationRenderer::setNormalisedBlending(bool nb) {
    if (normalisedBlending_ != nb) {
        normalisedBlending_ = nb;
        buildShaders(builtWithBackground_);
    }
}

void AbufferOpacityOptimisationRenderer::setUniforms(Shader& shader,
                                                     const TextureUnit& abuffUnit) const {
    OpacityOptimisationRenderer::setUniforms(shader, abuffUnit);

    shader.setUniform("screenSize", ivec2(screenSize_));
    if (debug) shader.setUniform("debugCoords", debugCoords);
    shader.setUniform("importanceSumCoeffs[0]", importanceSumUnitMain_->getUnitNumber());
    if (smoothing)
        shader.setUniform("importanceSumCoeffs[1]", importanceSumUnitSmooth_->getUnitNumber());
    shader.setUniform("opticalDepthCoeffs", opticalDepthUnit_->getUnitNumber());
}

void AbufferOpacityOptimisationRenderer::resizeBuffers(const size2_t& screenSize) {
    if (screenSize != screenSize_) {
        importanceSumTexture_[0].uploadAndResize(
            nullptr, size3_t(screenSize.x, screenSize.y, nImportanceSumCoefficients_));
        importanceSumTexture_[1].uploadAndResize(
            nullptr, size3_t(screenSize.x, screenSize.y, nImportanceSumCoefficients_));
        opticalDepthTexture_.uploadAndResize(
            nullptr, size3_t(screenSize.x, screenSize.y, nOpticalDepthCoefficients_));
    }
    FragmentListRenderer::resizeBuffers(screenSize);
}

void AbufferOpacityOptimisationRenderer::generateAndUploadGaussianKernel(int radius, float sigma) {
    gaussianRadius_ = radius;
    gaussianSigma_ = sigma;

    std::vector<float> k = util::generateGaussianKernel(radius, sigma);
    gaussianKernel_.upload(&k[0], k.size() * sizeof(float));
    gaussianKernel_.unbind();
}

void AbufferOpacityOptimisationRenderer::generateAndUploadLegendreCoefficients() {
    std::vector<float> coeffs = Approximations::generateLegendreCoefficients();
    legendreCoefficients_.upload(&coeffs[0], coeffs.size() * sizeof(int));
    legendreCoefficients_.unbind();
}

}  // namespace inviwo
