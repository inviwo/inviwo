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

#include <modules/opactopt/rendering/approximateopacityoptimisationrenderer.h>

namespace inviwo {

ApproximateOpacityOptimisationRenderer::ApproximateOpacityOptimisationRenderer(
    const Approximations::ApproximationProperties& p, int N, int gaussianRadius,
    float gaussianSigma)
    : ap_(p)
    , nCoefficients_(N)
    , importanceSumCoeffs_{{size3_t(screenSize_.x, screenSize_.y, nCoefficients_), GL_RED, GL_R32F,
                            GL_FLOAT, GL_NEAREST},
                           {size3_t(screenSize_.x, screenSize_.y, nCoefficients_), GL_RED, GL_R32F,
                            GL_FLOAT, GL_NEAREST}}
    , opticalDepthCoeffs_{size3_t(screenSize_.x, screenSize_.y, nCoefficients_), GL_RED, GL_R32F,
                          GL_FLOAT, GL_NEAREST}
    , gaussianKernel_{21 * sizeof(float),                   // allocate max possible size
                      GLFormats::getGLFormat(GL_FLOAT, 1),  // dummy format, will not be used
                      GL_STATIC_DRAW, GL_SHADER_STORAGE_BUFFER}
    , gaussianRadius_(gaussianRadius)
    , gaussianSigma_(gaussianSigma) {

    display_ =
        Shader("oit/simplequad.vert", "opactopt/approximate/display.frag", Shader::Build::No);
    clear_ = Shader("oit/simplequad.vert", "opactopt/approximate/clear.frag", Shader::Build::No);
    generateAndUploadGaussianKernel(gaussianRadius, gaussianSigma, true);
    buildShaders();
}

ApproximateOpacityOptimisationRenderer::~ApproximateOpacityOptimisationRenderer() {}

void ApproximateOpacityOptimisationRenderer::buildShaders(bool hasBackground,
                                                          bool useIllustration) {
    builtWithBackground_ = hasBackground;
    auto* dfs = display_.getFragmentShaderObject();
    dfs->clearShaderExtensions();
    dfs->clearShaderDefines();

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

        cfs->setShaderDefine("N_APPROXIMATION_COEFFICIENTS", true,
                             std::to_string(nCoefficients_).c_str());

        dfs->setShaderDefine("BACKGROUND_AVAILABLE", builtWithBackground_);
        dfs->setShaderDefine(ap_.shaderDefineName.c_str(), true);
        dfs->setShaderDefine("N_APPROXIMATION_COEFFICIENTS", true,
                             std::to_string(nCoefficients_).c_str());

        display_.build();
        clear_.build();
    }
}

void ApproximateOpacityOptimisationRenderer::updateDescriptor(
    const Approximations::ApproximationProperties& p, int N) {
    ap_ = Approximations::ApproximationProperties(p);
    if (N != nCoefficients_) {
        importanceSumCoeffs_[0].uploadAndResize(nullptr, size3_t(screenSize_.x, screenSize_.y, N));
        importanceSumCoeffs_[1].uploadAndResize(nullptr, size3_t(screenSize_.x, screenSize_.y, N));
        opticalDepthCoeffs_.uploadAndResize(nullptr, size3_t(screenSize_.x, screenSize_.y, N));
        nCoefficients_ = N;
    }
    buildShaders(builtWithBackground_, useIllustration_);
}

void ApproximateOpacityOptimisationRenderer::setUniforms(Shader& shader,
                                                         const TextureUnit& abuffUnit) const {
    OpacityOptimisationRenderer::setUniforms(shader, abuffUnit);

    TextureUnit importanceSumUnit[2];
    importanceSumUnit[0].activate();
    importanceSumCoeffs_[0].bind();
    glBindImageTexture(importanceSumUnit[0].getUnitNumber(), importanceSumCoeffs_[0].getID(), 0,
                       false, 0, GL_READ_WRITE, GL_R32F);
    importanceSumUnit[1].activate();
    importanceSumCoeffs_[1].bind();
    glBindImageTexture(importanceSumUnit[1].getUnitNumber(), importanceSumCoeffs_[1].getID(), 0,
                       false, 0, GL_READ_WRITE, GL_R32F);

    shader.setUniform("importanceSumCoeffs[0]", importanceSumUnit[0]);
    shader.setUniform("importanceSumCoeffs[1]", importanceSumUnit[1]);

    TextureUnit opticalDepthUnit;
    opticalDepthUnit.activate();
    opticalDepthCoeffs_.bind();
    glBindImageTexture(opticalDepthUnit.getUnitNumber(), opticalDepthCoeffs_.getID(), 0, false, 0,
                       GL_READ_WRITE, GL_R32F);
    shader.setUniform("opticalDepthCoeffs", opticalDepthUnit.getUnitNumber());

    glActiveTexture(GL_TEXTURE0);

    shader.setUniform("smoothing", smoothing);
    gaussianKernel_.bindBase(8);
    shader.setUniform("radius", gaussianRadius_);

    shader.setUniform("znear", znear);
    shader.setUniform("zfar", zfar);
}

void ApproximateOpacityOptimisationRenderer::resizeBuffers(const size2_t& screenSize) {
    if (screenSize != screenSize_) {
        importanceSumCoeffs_[0].uploadAndResize(
            nullptr, size3_t(screenSize.x, screenSize.y, nCoefficients_));
        importanceSumCoeffs_[1].uploadAndResize(
            nullptr, size3_t(screenSize.x, screenSize.y, nCoefficients_));
        opticalDepthCoeffs_.uploadAndResize(nullptr,
                                            size3_t(screenSize.x, screenSize.y, nCoefficients_));
    }
    FragmentListRenderer::resizeBuffers(screenSize);
}

std::vector<float> generateGaussianKernel(int radius, float sigma) {
    std::vector<float> res(2 * radius + 1, 0.0f);
    float kernel_sum = 0.0f;

    // Calculate kernel
    for (int i = 0; i <= radius; i++) {
        float val = std::exp(-((float)(i * i)) / (2 * (sigma * sigma)));
        if (i == 0) {
            res[radius] = val;
            kernel_sum += val;
        } else {
            res[radius + i] = val;
            res[radius - i] = val;
            kernel_sum += 2 * val;
        }
    }

    // Normalise
    for (int i = -radius; i <= radius; i++) {
        res[radius + i] /= kernel_sum;
    }

    return res;
}

void ApproximateOpacityOptimisationRenderer::generateAndUploadGaussianKernel(int radius,
                                                                             float sigma,
                                                                             bool force) {
    if (force || radius != gaussianRadius_ || sigma != gaussianSigma_) {
        gaussianRadius_ = radius;
        gaussianSigma_ = sigma;

        std::vector<float> k = generateGaussianKernel(radius, sigma);
        gaussianKernel_.upload(&k[0], (2 * radius + 1) * sizeof(float));
        gaussianKernel_.unbind();
    }
}

}  // namespace inviwo
