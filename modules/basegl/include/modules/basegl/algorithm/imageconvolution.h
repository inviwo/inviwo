/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2022 Inviwo Foundation
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

#pragma once

#include <modules/basegl/baseglmoduledefine.h>  // for IVW_MODULE_BASEGL_API

#include <inviwo/core/util/glmvec.h>       // for ivec2, vec2
#include <modules/opengl/shader/shader.h>  // for Shader, Shader::Build

#include <functional>   // for function
#include <memory>       // for shared_ptr
#include <string_view>  // for string_view
#include <vector>       // for vector

namespace inviwo {
class Image;
class Layer;

class IVW_MODULE_BASEGL_API ImageConvolution {
public:
    template <typename Callback>
    ImageConvolution(Callback C) : ImageConvolution() {
        shader_.onReload(C);
    }
    ImageConvolution() : shader_("img_convolution.frag", Shader::Build::No) {}
    virtual ~ImageConvolution() {}

    std::shared_ptr<Image> convolution(const Layer& layer, std::function<float(vec2)> kernelWeight,
                                       const float& kernelScale, ivec2 kernelSize);
    std::shared_ptr<Image> convolution(const Layer& layer, int kw, int kh,
                                       const std::vector<float>& kernel, const float& kernelScale);

    std::shared_ptr<Image> convolution_separable(const Layer& layer, std::function<float(float)>,
                                                 int kernelSize, const float& kernelScale);

    std::shared_ptr<Image> gaussianLowpass(const Layer& layer, int kernelSize);
    std::shared_ptr<Image> gaussianLowpass(const Layer& layer, float sigma);
    std::shared_ptr<Image> gaussianLowpass(const Layer& layer, int kernelSize, float sigma);

    std::shared_ptr<Image> lowpass(const Layer& layer, int kernelSize);

protected:
    Shader shader_;
};

}  // namespace inviwo
