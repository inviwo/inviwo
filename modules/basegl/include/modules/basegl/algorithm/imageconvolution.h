/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2018 Inviwo Foundation
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

#ifndef IVW_IMAGECONVOLUTION_H
#define IVW_IMAGECONVOLUTION_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/opengl/shader/shader.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/image.h>

namespace inviwo {

class IVW_MODULE_BASEGL_API ImageConvolution {
public:
    template <typename Callback>
    ImageConvolution(Callback C) : ImageConvolution() {
        shader_.onReload(C);
    }
    ImageConvolution() : shader_("img_convolution.frag", false) {}
    virtual ~ImageConvolution() {}

    std::shared_ptr<Image> convolution(const Layer &layer, std::function<float(vec2)> kernelWeight,
                                       const float &kernelScale, ivec2 kernelSize);

    std::shared_ptr<Image> convolution_separable(const Layer &layer, std::function<float(float)>,
                                                 int kernelSize, const float &kernelScale);

    std::shared_ptr<Image> gaussianLowpass(const Layer &layer, int kernelSize);
    std::shared_ptr<Image> gaussianLowpass(const Layer &layer, float sigma);
    std::shared_ptr<Image> gaussianLowpass(const Layer &layer, int kernelSize, float sigma);

    std::shared_ptr<Image> lowpass(const Layer &layer, int kernelSize);

protected:
    Shader shader_;

    std::shared_ptr<Image> convolution_internal(const Layer &layer, int kw, int kh,
                                                const std::vector<float> &kernel,
                                                const float &kernelScale);
};

}  // namespace inviwo

#endif  // IVW_IMAGECONVOLUTION_H
