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

#include <modules/basegl/algorithm/imageconvolution.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <modules/opengl/image/layergl.h>

#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/texture/texture2d.h>

namespace inviwo {

std::shared_ptr<Image> ImageConvolution::gaussianLowpass(const Layer &layer, int kernelSize) {
    float sigma =
        kernelSize / (2.f * 2.576f);  // 99% of samples are within +- 2.576 standard deviations
                                      // https://de.wikipedia.org/wiki/Normalverteilung
    return gaussianLowpass(layer, kernelSize, sigma);
}

std::shared_ptr<Image> ImageConvolution::gaussianLowpass(const Layer &layer, float sigma) {
    int kernelSize = static_cast<int>(
        sigma * 2 * 2.576);  // 99% of samples are within +- 2.576 standard deviations
                             // https://de.wikipedia.org/wiki/Normalverteilung
    return gaussianLowpass(layer, kernelSize, sigma);
}

std::shared_ptr<Image> ImageConvolution::gaussianLowpass(const Layer &layer, int kernelSize,
                                                         float sigma) {
    float sigmaSq2 = 2.0f * sigma * sigma;
    float a = 1.0f / (sigmaSq2 * glm::pi<float>());

    float totWeight = 0;
    auto kernelFunc = [&](float p) {
        float w = a * std::exp(-(p * p) / sigmaSq2);
        totWeight += w;
        return w;
    };

    return convolution_separable(layer, kernelFunc, kernelSize, totWeight);
}

std::shared_ptr<Image> ImageConvolution::lowpass(const Layer &layer, int kernelSize) {
    return convolution_separable(layer, [](float /*p*/) { return 1.f; }, kernelSize,
                                 static_cast<float>(kernelSize));
}

std::shared_ptr<Image> ImageConvolution::convolution(const Layer &layer,
                                                     std::function<float(vec2)> kernelWeight,
                                                     const float &kernelScale, ivec2 kernelSize) {

    auto kernel1DSize = kernelSize.x * kernelSize.y;
    if (kernel1DSize == 1) {
        auto newlayer = std::make_shared<Layer>(layer);
        return std::make_shared<Image>(newlayer);
    }

    std::vector<float> kernel(kernel1DSize);

    vec2 kernelCenter(kernelSize);
    kernelCenter /= 2.0f;

    for (int j = 0; j < kernelSize.y; j++) {
        for (int i = 0; i < kernelSize.x; i++) {
            vec2 p = vec2(i, j) - kernelCenter;
            kernel[i + j * kernelSize.y] = kernelWeight(p);
        }
    }

    return convolution(layer, kernelSize.x, kernelSize.y, kernel, kernelScale);
}

std::shared_ptr<Image> ImageConvolution::convolution_separable(
    const Layer &layer, std::function<float(float)> kernelWeight, int kernelSize,
    const float &kernelScale) {

    std::vector<float> kernel(kernelSize);
    float kernelCenter = (kernelSize - 1) / 2.0f;
    for (int i = 0; i < kernelSize; i++) {
        float p = i - kernelCenter;
        kernel[i] = kernelWeight(p);
    }

    auto hori = convolution(layer, kernelSize, 1, kernel, kernelScale);
    return convolution(*hori->getColorLayer(), 1, kernelSize, kernel, kernelScale);
}

std::shared_ptr<Image> ImageConvolution::convolution(const Layer &layer, int kw, int kh,
                                                     const std::vector<float> &kernel,
                                                     const float &kernelScale) {
    shader_.getFragmentShaderObject()->addShaderDefine("KERNELWIDTH", std::to_string(kw));
    shader_.getFragmentShaderObject()->addShaderDefine("KERNELHEIGHT", std::to_string(kh));
    shader_.getFragmentShaderObject()->addShaderDefine("KERNELSIZE", std::to_string(kw * kh));
    shader_.build();

    auto outImage = std::make_shared<Image>(std::make_shared<Layer>(
        layer.getDimensions(), layer.getDataFormat(),
        LayerType::Color,  // always treat as color, even if picking or depth
        layer.getSwizzleMask()));

    TextureUnitContainer cont;

    utilgl::activateTarget(*outImage);
    shader_.activate();

    utilgl::bindAndSetUniforms(shader_, cont, *layer.getRepresentation<LayerGL>()->getTexture(),
                               "tex");

    shader_.setUniform("kernel", kw * kh, kernel.data());
    shader_.setUniform("kernelScale", kernelScale);
    shader_.setUniform("reciprocalDimensions", vec2(1) / vec2(layer.getDimensions()));

    utilgl::singleDrawImagePlaneRect();
    shader_.deactivate();
    utilgl::deactivateCurrentTarget();

    return outImage;
}

}  // namespace inviwo
